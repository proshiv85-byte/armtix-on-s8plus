/*
 * armtix-camera: simple camera app for the Galaxy S8+ rear camera on ARMtix.
 *
 * The sensor (S5K2L2 behind the 73C3 companion) streams raw Bayer frames
 * through Samsung's FIMC-IS V4L2 driver without the ISP (needs the ARMtix
 * kernel's raw-capture scenario 7). Preview: 2016x1512 @ 30 fps, reduced to
 * 1008x756 RGB on the CPU, auto exposure + gray-world white balance. Photo:
 * the sensor switches to 4032x3024 for one frame, saved as JPEG in
 * ~/Pictures.
 *
 * Tap the shutter button (or Space / Enter, or `pkill -USR1 armtix-camera`)
 * to take a photo, Escape to quit.
 */
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <linux/videodev2.h>
#include <jpeglib.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "shot.h"

/* ---------------------------------------------------------------- driver */

#define MODULE_ID	32		/* SENSOR_NAME_S5K2L2 (rear) */
#define DEV_PRE		"/dev/video109"	/* companion (preprocessor) */
#define DEV_SS		"/dev/video101"	/* sensor 0, group leader */
#define DEV_VC0		"/dev/video210"	/* sensor 0 virtual channel 0 (raw) */
#define VID_SS0		1
#define VID_SS0VC0	110
#define SCENARIO_RAW	7u		/* ARMtix kernel: sensor driving, normal power */
#define META_SIZE	(64 * 1024)	/* >= sizeof(struct camera2_shot_ext) */
#define SHOT_MAGIC	0x23456789
#define NBUF_MAX	4

#define CID_CAM(x)		(V4L2_CTRL_CLASS_CAMERA | (x))
#define CID_IS_S_STREAM		CID_CAM(0x1000 + 14)
#define CID_IS_S_SENSOR_SIZE	CID_CAM(0x1000 + 69)
#define CID_SET_AE_TARGET	CID_CAM(0x3000 + 1)	/* exposure, us */
#define CID_SET_ANALOG_GAIN	CID_CAM(0x3000 + 18)	/* permille */

/* Samsung's kernel has VIDEO_MAX_PLANES = 17: its struct v4l2_format is
 * bigger than the system header's, so S_FMT needs its own definition */
struct k_pix_format_mplane {
	uint32_t width, height, pixelformat, field, colorspace;
	struct { uint32_t sizeimage, bytesperline; uint16_t reserved[6]; } plane_fmt[17];
	uint8_t num_planes, flags, ycbcr_enc, quantization, xfer_func, reserved[7];
};
struct k_format {
	uint32_t type;
	union { struct k_pix_format_mplane pix_mp; uint8_t raw[376]; void *align; } fmt;
};
_Static_assert(sizeof(struct k_format) == 384, "kernel v4l2_format size");
#define K_VIDIOC_S_FMT	_IOWR('V', 5, struct k_format)

/* legacy (4.4 staging) ION */
struct ion_allocation_data { size_t len, align; unsigned heap_id_mask, flags; int handle; };
struct ion_fd_data { int handle, fd; };
struct ion_handle_data { int handle; };
#define ION_IOC_ALLOC	_IOWR('I', 0, struct ion_allocation_data)
#define ION_IOC_FREE	_IOWR('I', 1, struct ion_handle_data)
#define ION_IOC_SHARE	_IOWR('I', 4, struct ion_fd_data)

struct buf { int fd; void *map; size_t len; };

struct cam {
	int ion, pre, ss, vc;
	int w, h, nbuf;
	struct buf ss_img[NBUF_MAX], ss_meta[NBUF_MAX], vc_img[NBUF_MAX], vc_meta[NBUF_MAX];
	int exposure_us, gain;		/* current sensor settings */
};

static const enum v4l2_buf_type SS_TYPE = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;	/* leader */
static const enum v4l2_buf_type VC_TYPE = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

static int xioctl(int fd, unsigned long req, void *arg, const char *what)
{
	int r = ioctl(fd, req, arg);
	if (r < 0)
		fprintf(stderr, "armtix-camera: %s: %s\n", what, strerror(errno));
	return r;
}

static int set_ctrl(int fd, unsigned id, int value, const char *what)
{
	struct v4l2_control c = { .id = id, .value = value };
	return xioctl(fd, VIDIOC_S_CTRL, &c, what);
}

static int ion_alloc(int ion, struct buf *b, size_t len)
{
	struct ion_allocation_data a = { .len = len, .align = 4096,
		.heap_id_mask = 1 /* system heap */, .flags = 1 /* cached */ };
	if (ioctl(ion, ION_IOC_ALLOC, &a) < 0)
		return -1;
	struct ion_fd_data s = { .handle = a.handle };
	int r = ioctl(ion, ION_IOC_SHARE, &s);
	struct ion_handle_data f = { .handle = a.handle };
	ioctl(ion, ION_IOC_FREE, &f);	/* the dma-buf fd keeps it alive */
	if (r < 0)
		return -1;
	b->fd = s.fd;
	b->len = len;
	b->map = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, s.fd, 0);
	if (b->map == MAP_FAILED) {
		close(s.fd);
		b->map = NULL;
		return -1;
	}
	return 0;
}

static void ion_release(struct buf *b)
{
	if (b->map)
		munmap(b->map, b->len);
	if (b->map || b->fd > 0)
		close(b->fd);
	memset(b, 0, sizeof(*b));
}

/* The leader buffer's metadata tells the driver which capture nodes get
 * this frame: VC0 (the raw image) */
static void fill_shot(void *meta, int w, int h)
{
	struct camera2_shot_ext *s = meta;
	memset(s, 0, sizeof(*s));
	s->node_group.leader.vid = VID_SS0;
	s->node_group.leader.request = 1;
	s->node_group.leader.input.cropRegion[2] = w;
	s->node_group.leader.input.cropRegion[3] = h;
	s->node_group.leader.output.cropRegion[2] = w;
	s->node_group.leader.output.cropRegion[3] = h;
	s->node_group.capture[0].vid = VID_SS0VC0;
	s->node_group.capture[0].request = 1;
	s->node_group.capture[0].input.cropRegion[2] = w;
	s->node_group.capture[0].input.cropRegion[3] = h;
	s->node_group.capture[0].output.cropRegion[2] = w;
	s->node_group.capture[0].output.cropRegion[3] = h;
	s->node_group.capture[0].pixelformat = V4L2_PIX_FMT_SBGGR16;
	s->shot.magicNumber = SHOT_MAGIC;
}

static int queue(int fd, enum v4l2_buf_type type, int i, struct buf *img, struct buf *meta)
{
	struct v4l2_plane p[2] = { 0 };
	p[0].m.fd = img->fd;
	p[0].length = img->len;
	p[1].m.fd = meta->fd;
	p[1].length = meta->len;
	struct v4l2_buffer b = { .index = i, .type = type, .memory = V4L2_MEMORY_DMABUF,
		.m.planes = p, .length = 2 };
	return ioctl(fd, VIDIOC_QBUF, &b);
}

static int set_format(int fd, enum v4l2_buf_type type, int w, int h)
{
	struct k_format f = { .type = type };
	f.fmt.pix_mp.width = w;
	f.fmt.pix_mp.height = h;
	f.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_SBGGR16;	/* 10 bit in 16 bit words */
	f.fmt.pix_mp.num_planes = 2;				/* image + metadata */
	return xioctl(fd, K_VIDIOC_S_FMT, &f, "S_FMT");
}

static void cam_sensor_settings(struct cam *c)
{
	set_ctrl(c->ss, CID_SET_AE_TARGET, c->exposure_us, "exposure");
	set_ctrl(c->ss, CID_SET_ANALOG_GAIN, c->gain, "gain");
}

static void cam_stop(struct cam *c)
{
	if (c->ss > 0) {
		set_ctrl(c->ss, CID_IS_S_STREAM, 0, "stream off");
		enum v4l2_buf_type t = SS_TYPE;
		ioctl(c->ss, VIDIOC_STREAMOFF, &t);
	}
	if (c->vc > 0) {
		enum v4l2_buf_type t = VC_TYPE;
		ioctl(c->vc, VIDIOC_STREAMOFF, &t);
	}
	for (int i = 0; i < NBUF_MAX; i++) {
		ion_release(&c->ss_img[i]);
		ion_release(&c->ss_meta[i]);
		ion_release(&c->vc_img[i]);
		ion_release(&c->vc_meta[i]);
	}
	if (c->vc > 0)
		close(c->vc);
	if (c->ss > 0)
		close(c->ss);
	c->vc = c->ss = -1;
}

/* Open sensor + VC0 at w x h (a sensor mode: 2016x1512 or 4032x3024) and
 * start streaming. The companion (c->pre) stays open and initialized. */
static int cam_start(struct cam *c, int w, int h, int nbuf)
{
	unsigned in;
	c->w = w;
	c->h = h;
	c->nbuf = nbuf;
	c->ss = open(DEV_SS, O_RDWR);
	c->vc = open(DEV_VC0, O_RDWR);
	if (c->ss < 0 || c->vc < 0) {
		fprintf(stderr, "armtix-camera: open sensor: %s\n", strerror(errno));
		goto fail;
	}
	in = (SCENARIO_RAW << 26) | (MODULE_ID << 16) | (VID_SS0 << 8) | 1;
	if (xioctl(c->ss, VIDIOC_S_INPUT, &in, "sensor S_INPUT") < 0 ||
	    xioctl(c->vc, VIDIOC_S_INPUT, &in, "vc0 S_INPUT") < 0)
		goto fail;
	if (set_ctrl(c->ss, CID_IS_S_SENSOR_SIZE, (w << 16) | h, "sensor size") < 0 ||
	    set_format(c->ss, SS_TYPE, w, h) < 0 || set_format(c->vc, VC_TYPE, w, h) < 0)
		goto fail;

	struct v4l2_requestbuffers rb = { .count = nbuf, .memory = V4L2_MEMORY_DMABUF, .type = VC_TYPE };
	if (xioctl(c->vc, VIDIOC_REQBUFS, &rb, "vc0 REQBUFS") < 0)
		goto fail;
	rb.count = nbuf;
	rb.type = SS_TYPE;
	if (xioctl(c->ss, VIDIOC_REQBUFS, &rb, "sensor REQBUFS") < 0)
		goto fail;

	size_t img = (size_t)w * h * 2 + 65536;
	for (int i = 0; i < nbuf; i++) {
		if (ion_alloc(c->ion, &c->vc_img[i], img) || ion_alloc(c->ion, &c->vc_meta[i], META_SIZE) ||
		    ion_alloc(c->ion, &c->ss_img[i], img) || ion_alloc(c->ion, &c->ss_meta[i], META_SIZE)) {
			fprintf(stderr, "armtix-camera: ION allocation failed\n");
			goto fail;
		}
		fill_shot(c->ss_meta[i].map, w, h);
	}
	/* capture buffers first, then the leader's */
	for (int i = 0; i < nbuf; i++)
		if (queue(c->vc, VC_TYPE, i, &c->vc_img[i], &c->vc_meta[i]) < 0)
			goto fail;
	for (int i = 0; i < nbuf; i++)
		if (queue(c->ss, SS_TYPE, i, &c->ss_img[i], &c->ss_meta[i]) < 0)
			goto fail;
	enum v4l2_buf_type t = VC_TYPE;
	if (xioctl(c->vc, VIDIOC_STREAMON, &t, "vc0 STREAMON") < 0)
		goto fail;
	t = SS_TYPE;
	if (xioctl(c->ss, VIDIOC_STREAMON, &t, "sensor STREAMON") < 0 ||
	    set_ctrl(c->ss, CID_IS_S_STREAM, 1, "stream on") < 0)
		goto fail;
	cam_sensor_settings(c);
	return 0;
fail:
	cam_stop(c);
	return -1;
}

/* Give finished leader buffers back (with fresh metadata) */
static void cam_recycle_leader(struct cam *c)
{
	struct pollfd pf = { .fd = c->ss, .events = POLLOUT };
	while (poll(&pf, 1, 0) > 0 && (pf.revents & POLLOUT)) {
		struct v4l2_plane p[2];
		struct v4l2_buffer b = { .type = SS_TYPE, .memory = V4L2_MEMORY_DMABUF, .m.planes = p, .length = 2 };
		if (ioctl(c->ss, VIDIOC_DQBUF, &b) < 0)
			break;
		fill_shot(c->ss_meta[b.index].map, c->w, c->h);
		queue(c->ss, SS_TYPE, b.index, &c->ss_img[b.index], &c->ss_meta[b.index]);
	}
}

/* Wait up to timeout_ms for a raw frame; returns its buffer index or -1.
 * Hand it back with cam_release() when done. */
static int cam_frame(struct cam *c, int timeout_ms)
{
	cam_recycle_leader(c);
	struct pollfd pf = { .fd = c->vc, .events = POLLIN };
	if (poll(&pf, 1, timeout_ms) <= 0 || !(pf.revents & POLLIN))
		return -1;
	struct v4l2_plane p[2];
	struct v4l2_buffer b = { .type = VC_TYPE, .memory = V4L2_MEMORY_DMABUF, .m.planes = p, .length = 2 };
	if (ioctl(c->vc, VIDIOC_DQBUF, &b) < 0)
		return -1;
	return b.index;
}

static void cam_release(struct cam *c, int i)
{
	queue(c->vc, VC_TYPE, i, &c->vc_img[i], &c->vc_meta[i]);
}

/* ---------------------------------------------------------------- image */

static uint8_t gamma_lut[1024];		/* 10 bit linear -> 8 bit display */
static float wb[3] = { 1, 1, 1 };	/* R G B gains (gray world, smoothed) */

static void init_gamma(void)
{
	for (int i = 0; i < 1024; i++) {
		double x = i / 1023.0;	/* sRGB transfer curve */
		double y = x <= 0.0031308 ? 12.92 * x : 1.055 * pow(x, 1 / 2.4) - 0.055;
		gamma_lut[i] = (uint8_t)(y * 255 + 0.5);
	}
}

static inline uint8_t tone(int v)
{
	return gamma_lut[v < 0 ? 0 : v > 1023 ? 1023 : v];
}

/* BGGR -> half-size RGB (one output pixel per 2x2 block), with stats */
static void preview_convert(const uint16_t *raw, int w, int h, uint8_t *rgb, int pitch,
			    double means[3])
{
	int gr = (int)(wb[0] * 256), gg = (int)(wb[1] * 256), gb = (int)(wb[2] * 256);
	uint64_t sr = 0, sg = 0, sb = 0, n = 0;
	for (int y = 0; y < h / 2; y++) {
		const uint16_t *r0 = raw + (size_t)(2 * y) * w, *r1 = r0 + w;
		uint8_t *o = rgb + (size_t)y * pitch;
		for (int x = 0; x < w / 2; x++) {
			int b = r0[2 * x], g = (r0[2 * x + 1] + r1[2 * x]) >> 1, r = r1[2 * x + 1];
			if (((x | y) & 7) == 0) {
				sr += r; sg += g; sb += b; n++;
			}
			o[3 * x] = tone((r * gr) >> 8);
			o[3 * x + 1] = tone((g * gg) >> 8);
			o[3 * x + 2] = tone((b * gb) >> 8);
		}
	}
	means[0] = (double)sr / n;
	means[1] = (double)sg / n;
	means[2] = (double)sb / n;
}

/* Bilinear demosaic of a full BGGR frame to RGB, with white balance, a
 * brightness scale and the tone curve; rotate 90 degrees clockwise if asked */
static uint8_t *photo_convert(const uint16_t *raw, int w, int h, float scale, int rotate,
			      int *ow, int *oh)
{
	uint8_t *out = malloc((size_t)w * h * 3);
	if (!out)
		return NULL;
	*ow = rotate ? h : w;
	*oh = rotate ? w : h;
	float k[3] = { wb[0] * scale, wb[1] * scale, wb[2] * scale };
#define P(xx, yy) ((int)raw[(size_t)((yy) < 0 ? 1 : (yy) >= h ? h - 2 : (yy)) * w + \
			   ((xx) < 0 ? 1 : (xx) >= w ? w - 2 : (xx))])
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int c = P(x, y), r, g, b;
			int cross = (P(x - 1, y) + P(x + 1, y) + P(x, y - 1) + P(x, y + 1)) >> 2;
			int diag = (P(x - 1, y - 1) + P(x + 1, y - 1) + P(x - 1, y + 1) + P(x + 1, y + 1)) >> 2;
			int hor = (P(x - 1, y) + P(x + 1, y)) >> 1, ver = (P(x, y - 1) + P(x, y + 1)) >> 1;
			if (!(y & 1) && !(x & 1)) { b = c; g = cross; r = diag; }	/* B */
			else if ((y & 1) && (x & 1)) { r = c; g = cross; b = diag; }	/* R */
			else if (!(y & 1)) { g = c; b = hor; r = ver; }			/* G on B row */
			else { g = c; r = hor; b = ver; }				/* G on R row */
			size_t o = rotate ? ((size_t)x * *ow + (h - 1 - y)) * 3 : ((size_t)y * w + x) * 3;
			out[o] = tone((int)(r * k[0]));
			out[o + 1] = tone((int)(g * k[1]));
			out[o + 2] = tone((int)(b * k[2]));
		}
	}
#undef P
	return out;
}

static int save_jpeg(const char *path, const uint8_t *rgb, int w, int h)
{
	FILE *f = fopen(path, "wb");
	if (!f)
		return -1;
	struct jpeg_compress_struct ci;
	struct jpeg_error_mgr je;
	ci.err = jpeg_std_error(&je);
	jpeg_create_compress(&ci);
	jpeg_stdio_dest(&ci, f);
	ci.image_width = w;
	ci.image_height = h;
	ci.input_components = 3;
	ci.in_color_space = JCS_RGB;
	jpeg_set_defaults(&ci);
	jpeg_set_quality(&ci, 92, TRUE);
	jpeg_start_compress(&ci, TRUE);
	while (ci.next_scanline < ci.image_height) {
		JSAMPROW row = (JSAMPROW)(rgb + (size_t)ci.next_scanline * w * 3);
		jpeg_write_scanlines(&ci, &row, 1);
	}
	jpeg_finish_compress(&ci);
	jpeg_destroy_compress(&ci);
	return fclose(f);
}

/* ---------------------------------------------------------------- 3A */

#define TARGET_MEAN	110.0	/* raw (10 bit) green mean the exposure aims for */
#define MAX_EXPOSURE	33000	/* us: one frame at 30 fps */
#define MIN_EXPOSURE	100
#define MAX_GAIN	16000
#define MIN_GAIN	1000

static void auto_exposure(struct cam *c, const double m[3])
{
	double ratio = TARGET_MEAN / (m[1] > 1 ? m[1] : 1);
	if (ratio > 0.9 && ratio < 1.1)
		return;
	ratio = ratio < 0.5 ? 0.5 : ratio > 2.0 ? 2.0 : ratio;
	/* total = exposure * gain; exposure first (less noise), then gain */
	double total = (double)c->exposure_us * c->gain / 1000.0 * ratio;
	int e = total > MAX_EXPOSURE ? MAX_EXPOSURE : total < MIN_EXPOSURE ? MIN_EXPOSURE : (int)total;
	int g = (int)(total / e * 1000.0);
	g = g < MIN_GAIN ? MIN_GAIN : g > MAX_GAIN ? MAX_GAIN : g;
	if (e != c->exposure_us || g != c->gain) {
		c->exposure_us = e;
		c->gain = g;
		cam_sensor_settings(c);
	}
}

static void auto_white_balance(const double m[3])
{
	if (m[0] < 4 || m[1] < 4 || m[2] < 4)
		return;
	double t[3] = { m[1] / m[0], 1.0, m[1] / m[2] };
	for (int i = 0; i < 3; i++) {
		if (t[i] > 4) t[i] = 4;
		if (t[i] < 0.25) t[i] = 0.25;
		wb[i] = (float)(wb[i] * 0.8 + t[i] * 0.2);
	}
}

/* ---------------------------------------------------------------- photo */

static void notify(const char *summary, const char *body)
{
	pid_t p = fork();
	if (p == 0) {
		execlp("notify-send", "notify-send", "-a", "Camera", summary, body, (char *)NULL);
		_exit(127);
	}
}

static double photo_ref_mean = TARGET_MEAN;	/* preview green mean when the shutter was pressed */

static int take_photo(struct cam *c, int rotate, char *saved, size_t saved_len)
{
	int ret = -1;
	cam_stop(c);
	if (cam_start(c, 4032, 3024, 2) < 0)
		goto restart;
	/* skip frames until the sensor runs with our exposure */
	int idx = -1;
	for (int n = 0; n < 4; n++) {
		idx = cam_frame(c, 2000);
		if (idx < 0)
			break;
		if (n < 3)
			cam_release(c, idx);
	}
	if (idx < 0) {
		fprintf(stderr, "armtix-camera: no full-resolution frame\n");
		goto restart;
	}
	const uint16_t *raw = c->vc_img[idx].map;
	/* full-resolution readout isn't binned: match the preview's brightness */
	uint64_t s = 0, n = 0;
	for (size_t y = 0; y < 3024; y += 16)
		for (size_t x = 1; x < 4032; x += 16) { s += raw[y * 4032 + x]; n++; }
	double mean = n ? (double)s / n : 1;
	float scale = (float)(photo_ref_mean / (mean > 1 ? mean : 1));
	scale = scale < 0.25f ? 0.25f : scale > 8 ? 8 : scale;

	int ow, oh;
	uint8_t *rgb = photo_convert(raw, 4032, 3024, scale, rotate, &ow, &oh);
	cam_release(c, idx);
	if (!rgb)
		goto restart;
	const char *home = getenv("HOME");
	char dir[512], path[600];
	snprintf(dir, sizeof(dir), "%s/Pictures", home ? home : "/tmp");
	mkdir(dir, 0755);
	time_t t = time(NULL);
	struct tm tm;
	localtime_r(&t, &tm);
	snprintf(path, sizeof(path), "%s/IMG_%04d%02d%02d_%02d%02d%02d.jpg", dir,
		 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	ret = save_jpeg(path, rgb, ow, oh);
	free(rgb);
	if (ret == 0)
		snprintf(saved, saved_len, "%s", path);
restart:
	cam_stop(c);
	if (cam_start(c, 2016, 1512, 4) < 0)
		return -2;	/* preview is gone */
	return ret;
}

/* ---------------------------------------------------------------- UI */

static volatile sig_atomic_t shoot_signal;
static void on_usr1(int sig) { (void)sig; shoot_signal = 1; }

static void draw_circle(SDL_Renderer *r, float cx, float cy, float rad, SDL_FColor col)
{
	enum { SEG = 48 };
	SDL_Vertex v[SEG + 2];
	int idx[SEG * 3];
	v[0] = (SDL_Vertex){ { cx, cy }, col, { 0, 0 } };
	for (int i = 0; i <= SEG; i++) {
		float a = (float)i / SEG * 6.2831853f;
		v[i + 1] = (SDL_Vertex){ { cx + rad * cosf(a), cy + rad * sinf(a) }, col, { 0, 0 } };
	}
	for (int i = 0; i < SEG; i++) {
		idx[3 * i] = 0;
		idx[3 * i + 1] = i + 1;
		idx[3 * i + 2] = i + 2;
	}
	SDL_RenderGeometry(r, NULL, v, SEG + 2, idx, SEG * 3);
}

int main(int argc, char **argv)
{
	(void)argc; (void)argv;
	struct cam cam = { .ion = -1, .pre = -1, .ss = -1, .vc = -1, .exposure_us = 20000, .gain = 2000 };
	init_gamma();
	signal(SIGCHLD, SIG_IGN);	/* notify-send children */
	signal(SIGUSR1, on_usr1);

	/* vendor hw init: sets the ISP library names the sensor driver needs
	 * (once per boot, harmless to repeat) */
	FILE *f = fopen("/sys/class/camera/rear/fw_update", "r");
	if (f) {
		char l[64];
		if (!fgets(l, sizeof(l), f)) l[0] = 0;
		fclose(f);
	}
	cam.ion = open("/dev/ion", O_RDWR);
	cam.pre = open(DEV_PRE, O_RDWR);
	unsigned in = MODULE_ID;
	if (cam.ion < 0 || cam.pre < 0 || xioctl(cam.pre, VIDIOC_S_INPUT, &in, "companion S_INPUT") < 0 ||
	    cam_start(&cam, 2016, 1512, 4) < 0) {
		fprintf(stderr, "armtix-camera: can't start the camera (permissions? firmware in /system/vendor/firmware?)\n");
		notify("Camera", "Can't start the camera");
		return 1;
	}

	/* GLES2 is what the Mali driver (libhybris) offers; desktop GL would
	 * end up in software rendering */
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		fprintf(stderr, "armtix-camera: SDL: %s\n", SDL_GetError());
		return 1;
	}
	SDL_Window *win;
	SDL_Renderer *ren;
	if (!SDL_CreateWindowAndRenderer("Camera", 480, 900, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY,
					 &win, &ren)) {
		fprintf(stderr, "armtix-camera: SDL window: %s\n", SDL_GetError());
		return 1;
	}
	SDL_SetRenderVSync(ren, 1);
	SDL_Texture *tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 1008, 756);

	int running = 1, flash = 0, frames = 0;
	float btn_x = 0, btn_y = 0, btn_r = 0;
	while (running) {
		int shoot = shoot_signal;
		shoot_signal = 0;
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_EVENT_QUIT)
				running = 0;
			else if (e.type == SDL_EVENT_KEY_DOWN) {
				SDL_Keycode k = e.key.key;
				if (k == SDLK_ESCAPE || k == SDLK_AC_BACK)
					running = 0;
				else if (k == SDLK_SPACE || k == SDLK_RETURN)
					shoot = 1;
			} else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
				float dx = e.button.x - btn_x, dy = e.button.y - btn_y;
				if (dx * dx + dy * dy <= btn_r * btn_r * 2.0f)
					shoot = 1;
			}
		}

		int ww, wh;
		SDL_GetWindowSize(win, &ww, &wh);
		int portrait = wh > ww;

		if (shoot) {
			char saved[600] = "";
			flash = 6;
			photo_ref_mean = TARGET_MEAN;
			int r = take_photo(&cam, portrait, saved, sizeof(saved));
			if (r == 0)
				notify("Photo saved", saved);
			else
				notify("Camera", "Taking the photo failed");
			if (r == -2) {
				fprintf(stderr, "armtix-camera: preview didn't restart\n");
				break;
			}
		}

		int idx = cam_frame(&cam, 50);
		if (idx >= 0) {
			void *pix;
			int pitch;
			double m[3];
			if (SDL_LockTexture(tex, NULL, &pix, &pitch)) {
				preview_convert(cam.vc_img[idx].map, cam.w, cam.h, pix, pitch, m);
				SDL_UnlockTexture(tex);
				if (++frames % 3 == 0) {	/* the sensor applies settings ~2 frames late */
					auto_exposure(&cam, m);
					auto_white_balance(m);
				}
			}
			cam_release(&cam, idx);
		}

		/* layout in window coordinates */
		float ow, oh;
		int pw, ph;
		SDL_GetCurrentRenderOutputSize(ren, &pw, &ph);
		SDL_SetRenderScale(ren, (float)pw / ww, (float)ph / wh);
		SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
		SDL_RenderClear(ren);
		/* fit the 4:3 preview (rotated in portrait) */
		float iw = portrait ? 756 : 1008, ih = portrait ? 1008 : 756;
		float sc = fminf(ww / iw, wh / ih);
		ow = iw * sc;
		oh = ih * sc;
		float ox = (ww - ow) / 2, oy = portrait ? (wh - oh) * 0.35f : (wh - oh) / 2;
		if (portrait) {
			/* rotate around the center of the destination box */
			SDL_FRect d = { ox + (ow - oh) / 2, oy + (oh - ow) / 2, oh, ow };
			SDL_RenderTextureRotated(ren, tex, NULL, &d, 90, NULL, SDL_FLIP_NONE);
		} else {
			SDL_FRect d = { ox, oy, ow, oh };
			SDL_RenderTexture(ren, tex, NULL, &d);
		}
		/* shutter button: below the preview in portrait, right in landscape */
		btn_r = 34;
		if (portrait) {
			btn_x = ww / 2.0f;
			btn_y = oy + oh + (wh - oy - oh) / 2;
			if (btn_y > wh - btn_r - 16) btn_y = wh - btn_r - 16;
		} else {
			btn_x = ww - fmaxf((ww - ox - ow) / 2, btn_r + 16);
			btn_y = wh / 2.0f;
		}
		draw_circle(ren, btn_x, btn_y, btn_r, (SDL_FColor){ 1, 1, 1, 1 });
		draw_circle(ren, btn_x, btn_y, btn_r - 5, (SDL_FColor){ 0, 0, 0, 1 });
		draw_circle(ren, btn_x, btn_y, btn_r - 9, (SDL_FColor){ 1, 1, 1, 1 });
		if (flash > 0) {
			SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
			SDL_SetRenderDrawColor(ren, 255, 255, 255, (Uint8)(flash * 40));
			SDL_RenderFillRect(ren, NULL);
			flash--;
		}
		SDL_RenderPresent(ren);
	}

	cam_stop(&cam);
	close(cam.pre);
	close(cam.ion);
	SDL_DestroyTexture(tex);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();
	return 0;
}
