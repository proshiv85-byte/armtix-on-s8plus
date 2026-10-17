/*
 * fex-compat: run FEX-Emu on the Galaxy S8+'s Linux 4.4 kernel.
 *
 * Preloaded into the FEX (host, arm64) process by fex-run. It emulates in
 * userspace what FEX needs from newer kernels:
 *
 *  - mmap(MAP_FIXED_NOREPLACE) (4.17): 4.4 ignores the flag and treats the
 *    address as a hint, so FEX's allocator got memory at random places.
 *  - EL0 reads of the CPU ID registers (4.11, HWCAP_CPUID): SIGILL on 4.4.
 *  - openat2() with EMU_RESOLVE_IN_ROOT (5.6): FEX resolves every guest path
 *    inside the x86 RootFS with it.
 *  - faccessat2() (5.8), close_range() (5.9).
 *
 * It removes itself from LD_PRELOAD (x86 programs never see it) and puts
 * itself back when FEX re-executes itself for a guest execve().
 */
#define _GNU_SOURCE
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <ucontext.h>
#include <unistd.h>

#ifndef MAP_FIXED_NOREPLACE
#define MAP_FIXED_NOREPLACE 0x100000
#endif
#define NR_openat2 437
#define NR_faccessat2 439
#define NR_close_range 436
#define EMU_RESOLVE_NO_SYMLINKS 0x04
#define EMU_RESOLVE_BENEATH 0x08
#define EMU_RESOLVE_IN_ROOT 0x10
#define EMU_CLOSE_RANGE_CLOEXEC 4

struct emu_open_how {
	uint64_t flags, mode, resolve;
};

static long (*real_syscall)(long, ...);
static char self_path[512];

/* ---- mmap(MAP_FIXED_NOREPLACE) ---- */

static uintptr_t task_size(void)
{
	static uintptr_t ts;
	if (!ts) {
		/* the stack is at the top of the address space */
		uintptr_t sp = (uintptr_t)__builtin_frame_address(0);
		ts = 1UL << 36;
		while (ts <= sp)
			ts <<= 1;
	}
	return ts;
}

static long sys(long n, long a, long b, long c, long d, long e, long f)
{
	/* FEX's static constructors map memory before ours runs */
	if (!real_syscall)
		real_syscall = dlsym(RTLD_NEXT, "syscall");
	return real_syscall(n, a, b, c, d, e, f);
}

static void *mmap_noreplace(void *addr, size_t len, int prot, int flags, int fd, off_t off)
{
	flags &= ~MAP_FIXED_NOREPLACE;
	if ((uintptr_t)addr + len > task_size() || (uintptr_t)addr + len < (uintptr_t)addr) {
		errno = ENOMEM;
		return MAP_FAILED;
	}
	void *r = (void *)sys(SYS_mmap, (long)addr, len, prot, flags, fd, off);
	if (r == MAP_FAILED || r == addr)
		return r;
	sys(SYS_munmap, (long)r, len, 0, 0, 0, 0);
	errno = EEXIST;
	return MAP_FAILED;
}

void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t off)
{
	if (flags & MAP_FIXED_NOREPLACE)
		return mmap_noreplace(addr, len, prot, flags, fd, off);
	return (void *)sys(SYS_mmap, (long)addr, len, prot, flags, fd, off);
}
void *mmap64(void *, size_t, int, int, int, off_t) __attribute__((alias("mmap")));

/* ---- ID registers ---- */

static struct sigaction old_sigill;

/* Values a newer kernel exposes for the Exynos 8895: ARMv8.0 with AES/PMULL,
 * SHA1, SHA2, CRC32, FP, ASIMD; everything else reads as 0 (absent). */
static uint64_t id_reg(unsigned crm, unsigned op2)
{
	switch (crm << 3 | op2) {
	case 0 << 3 | 0: return 0x410fd034;	/* MIDR_EL1: Cortex-A53 r0p4 */
	case 0 << 3 | 5: return 0x80000000;	/* MPIDR_EL1 */
	case 4 << 3 | 0: return 0x11;		/* ID_AA64PFR0: EL0/EL1 AArch64, FP, ASIMD */
	case 6 << 3 | 0: return 0x11120;	/* ID_AA64ISAR0: AES+PMULL, SHA1, SHA2, CRC32 */
	default: return 0;
	}
}

static void sigill(int sig, siginfo_t *si, void *ctx)
{
	ucontext_t *uc = ctx;
	uint32_t insn = *(uint32_t *)uc->uc_mcontext.pc;

	/* mrs Xt, S3_0_C0_<CRm>_<op2> */
	if ((insn & 0xfffff000) == 0xd5380000) {
		unsigned rt = insn & 0x1f;
		if (rt != 31)
			uc->uc_mcontext.regs[rt] = id_reg((insn >> 8) & 0xf, (insn >> 5) & 7);
		uc->uc_mcontext.pc += 4;
		return;
	}
	if ((old_sigill.sa_flags & SA_SIGINFO) && old_sigill.sa_sigaction)
		old_sigill.sa_sigaction(sig, si, ctx);
	else if (old_sigill.sa_handler != SIG_DFL && old_sigill.sa_handler != SIG_IGN)
		old_sigill.sa_handler(sig);
	else
		signal(SIGILL, SIG_DFL);	/* re-executes and dies normally */
}

/* ---- openat2 ---- */

static int same_file(int a, int b)
{
	struct stat sa, sb;
	return fstat(a, &sa) == 0 && fstat(b, &sb) == 0 &&
	       sa.st_dev == sb.st_dev && sa.st_ino == sb.st_ino;
}

static void replace_fd(int *fd, int nfd)
{
	close(*fd);
	*fd = nfd;
}

/* Walks the path one component at a time: symlinks (and "..") can't leave
 * dirfd, absolute ones restart from it (EMU_RESOLVE_IN_ROOT) or fail
 * (EMU_RESOLVE_BENEATH). Returns the fd or -errno. */
static long emu_openat2(int dirfd, const char *path, const struct emu_open_how *how, size_t size)
{
	int flags = how->flags, links = 0, root, cur, ret;
	int beneath = !(how->resolve & EMU_RESOLVE_IN_ROOT);
	char *rest, *tmp, *comp;
	size_t cap = 2 * PATH_MAX;

	if (size < sizeof(*how) || !path)
		return -EINVAL;
	if (!(how->resolve & (EMU_RESOLVE_IN_ROOT | EMU_RESOLVE_BENEATH | EMU_RESOLVE_NO_SYMLINKS)))
		return sys(SYS_openat, dirfd, (long)path, flags, how->mode, 0, 0);

	root = dirfd == AT_FDCWD ? open(".", O_PATH | O_DIRECTORY | O_CLOEXEC)
				 : fcntl(dirfd, F_DUPFD_CLOEXEC, 0);
	if (root < 0)
		return -errno;
	cur = fcntl(root, F_DUPFD_CLOEXEC, 0);
	rest = malloc(cap);
	tmp = malloc(cap);
	if (cur < 0 || !rest || !tmp) {
		ret = -ENOMEM;
		goto out;
	}
	if (path[0] == '/' && beneath) {
		ret = -EXDEV;
		goto out;
	}
	snprintf(rest, cap, "%s", path);

	for (;;) {
		char *p = rest, *next;
		int last, nfd;
		struct stat st;

		while (*p == '/')
			p++;
		if (!*p) {
			ret = openat(cur, ".", flags, how->mode);
			ret = ret < 0 ? -errno : ret;
			goto out;
		}
		comp = p;
		next = strchr(p, '/');
		if (next)
			*next++ = '\0';
		else
			next = p + strlen(p);
		last = *next == '\0';
		/* copy the component out before the rest moves over it */
		snprintf(tmp, cap, "%s", comp);
		comp = tmp;
		memmove(rest, next, strlen(next) + 1);

		if (!strcmp(comp, "."))
			continue;
		if (!strcmp(comp, "..")) {
			if (same_file(cur, root)) {
				if (beneath) {
					ret = -EXDEV;
					goto out;
				}
				continue;
			}
			nfd = openat(cur, "..", O_PATH | O_DIRECTORY | O_CLOEXEC);
			if (nfd < 0) {
				ret = -errno;
				goto out;
			}
			replace_fd(&cur, nfd);
			continue;
		}
		if (last && (flags & O_CREAT) && (flags & O_EXCL)) {
			ret = openat(cur, comp, flags, how->mode);
			ret = ret < 0 ? -errno : ret;
			goto out;
		}
		nfd = openat(cur, comp, O_PATH | O_NOFOLLOW | O_CLOEXEC);
		if (nfd < 0) {
			ret = -errno;
			if (last && errno == ENOENT && (flags & O_CREAT)) {
				ret = openat(cur, comp, flags, how->mode);
				ret = ret < 0 ? -errno : ret;
			}
			goto out;
		}
		if (fstat(nfd, &st) < 0) {
			ret = -errno;
			close(nfd);
			goto out;
		}
		if (S_ISLNK(st.st_mode) && !(last && (flags & O_NOFOLLOW))) {
			char target[PATH_MAX];
			ssize_t n = readlinkat(nfd, "", target, sizeof(target) - 1);

			close(nfd);
			if (how->resolve & EMU_RESOLVE_NO_SYMLINKS) {
				ret = -ELOOP;
				goto out;
			}
			if (n < 0 || ++links > 40) {
				ret = n < 0 ? -errno : -ELOOP;
				goto out;
			}
			target[n] = '\0';
			if (target[0] == '/') {
				if (beneath) {
					ret = -EXDEV;
					goto out;
				}
				nfd = fcntl(root, F_DUPFD_CLOEXEC, 0);
				if (nfd < 0) {
					ret = -errno;
					goto out;
				}
				replace_fd(&cur, nfd);
			}
			if (strlen(target) + strlen(rest) + 2 > cap) {
				ret = -ENAMETOOLONG;
				goto out;
			}
			snprintf(tmp, cap, "%s/%s", target, rest);
			snprintf(rest, cap, "%s", tmp);
			continue;
		}
		if (last) {
			close(nfd);
			ret = openat(cur, comp, flags, how->mode);
			ret = ret < 0 ? -errno : ret;
			goto out;
		}
		if (!S_ISDIR(st.st_mode)) {
			close(nfd);
			ret = -ENOTDIR;
			goto out;
		}
		replace_fd(&cur, nfd);
	}
out:
	free(rest);
	free(tmp);
	if (cur >= 0)
		close(cur);
	close(root);
	return ret;
}

/* ---- faccessat2, close_range ---- */

static long emu_faccessat2(int dirfd, const char *path, int mode, int flags)
{
	if (flags & AT_SYMLINK_NOFOLLOW) {
		struct stat st;
		if (fstatat(dirfd, path, &st, AT_SYMLINK_NOFOLLOW) < 0)
			return -errno;
		if (mode == F_OK || S_ISLNK(st.st_mode))
			return 0;
	}
	/* AT_EACCESS: same as the real ids, FEX isn't setuid */
	return sys(SYS_faccessat, dirfd, (long)path, mode, 0, 0, 0) < 0 ? -errno : 0;
}

static long emu_close_range(unsigned first, unsigned last, unsigned flags)
{
	DIR *d = opendir("/proc/self/fd");
	struct dirent *e;

	if (!d)
		return -errno;
	while ((e = readdir(d))) {
		char *end;
		unsigned long fd = strtoul(e->d_name, &end, 10);

		if (*end || fd < first || fd > last || (int)fd == dirfd(d))
			continue;
		if (flags & EMU_CLOSE_RANGE_CLOEXEC)
			fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);
		else
			close(fd);
	}
	closedir(d);
	return 0;
}

/* ---- execveat of /proc/self/exe: keep the shim in FEX's children ---- */

static long exec_with_shim(long dirfd, const char *path, char *const argv[],
			   char *const envp[], long flags)
{
	size_t n = 0, i, j = 0;
	char **env, *preload = NULL;
	long ret;

	while (envp && envp[n])
		n++;
	env = calloc(n + 2, sizeof(*env));
	if (!env)
		return sys(SYS_execveat, dirfd, (long)path, (long)argv, (long)envp, flags, 0);
	for (i = 0; i < n; i++) {
		if (!strncmp(envp[i], "LD_PRELOAD=", 11) && !preload) {
			if (asprintf(&preload, "LD_PRELOAD=%s:%s", self_path, envp[i] + 11) < 0)
				preload = NULL;
			if (preload)
				env[j++] = preload;
			continue;
		}
		env[j++] = envp[i];
	}
	if (!preload && asprintf(&preload, "LD_PRELOAD=%s", self_path) >= 0)
		env[j++] = preload;
	env[j] = NULL;
	ret = sys(SYS_execveat, dirfd, (long)path, (long)argv, (long)env, flags, 0);
	free(preload);
	free(env);
	return ret;
}

/* ---- syscall() ---- */

static long ret_errno(long r)
{
	if (r < 0 && r > -4096) {
		errno = -r;
		return -1;
	}
	return r;
}

long syscall(long n, ...)
{
	va_list ap;
	long a, b, c, d, e, f;

	va_start(ap, n);
	a = va_arg(ap, long);
	b = va_arg(ap, long);
	c = va_arg(ap, long);
	d = va_arg(ap, long);
	e = va_arg(ap, long);
	f = va_arg(ap, long);
	va_end(ap);

	switch (n) {
	case NR_openat2: {
		long r = sys(n, a, b, c, d, 0, 0);
		if (r != -1 || errno != ENOSYS)
			return r;
		return ret_errno(emu_openat2(a, (const char *)b, (const struct emu_open_how *)c, d));
	}
	case NR_faccessat2: {
		long r = sys(n, a, b, c, d, 0, 0);
		if (r != -1 || errno != ENOSYS)
			return r;
		return ret_errno(emu_faccessat2(a, (const char *)b, c, d));
	}
	case NR_close_range: {
		long r = sys(n, a, b, c, 0, 0, 0);
		if (r != -1 || errno != ENOSYS)
			return r;
		return ret_errno(emu_close_range(a, b, c));
	}
	case SYS_mmap:
		if (d & MAP_FIXED_NOREPLACE)
			return (long)mmap_noreplace((void *)a, b, c, d, e, f);
		break;
	case SYS_execveat:
		if (self_path[0] && b && !strcmp((const char *)b, "/proc/self/exe"))
			return exec_with_shim(a, (const char *)b, (char *const *)c,
					      (char *const *)d, e);
		break;
	}
	return sys(n, a, b, c, d, e, f);
}

/* ---- setup ---- */

__attribute__((constructor)) static void fex_compat_init(void)
{
	struct sigaction sa;
	const char *pre = getenv("LD_PRELOAD");
	Dl_info info;

	real_syscall = dlsym(RTLD_NEXT, "syscall");

	if (dladdr((void *)fex_compat_init, &info) && info.dli_fname)
		snprintf(self_path, sizeof(self_path), "%s", info.dli_fname);

	/* Drop ourselves from LD_PRELOAD: FEX hands its environment to the
	 * x86 program, whose loader would complain about an arm64 library. */
	if (pre && self_path[0]) {
		char *copy = strdup(pre), *out = calloc(1, strlen(pre) + 1), *tok, *save;
		if (copy && out) {
			for (tok = strtok_r(copy, ": ", &save); tok; tok = strtok_r(NULL, ": ", &save)) {
				if (!strcmp(tok, self_path))
					continue;
				if (*out)
					strcat(out, ":");
				strcat(out, tok);
			}
			if (*out)
				setenv("LD_PRELOAD", out, 1);
			else
				unsetenv("LD_PRELOAD");
		}
		free(copy);
		free(out);
	}

	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = sigill;
	sa.sa_flags = SA_SIGINFO | SA_NODEFER;
	sigaction(SIGILL, &sa, &old_sigill);
}
