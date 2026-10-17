# GPU acceleration (Mali-G71) via libhybris, sway

The Exynos 8895's Mali-G71 has no usable open driver for this kernel, so ARMtix
runs the phone's **Android Mali driver** inside normal glibc programs with
[libhybris](https://github.com/libhybris/libhybris). The desktop for it is
**sway** (wlroots), compositing with OpenGL ES on the GPU. Its EGL apps
(Firefox, Qt, GTK 4, ...) render on the GPU too; Mesa (software) stays the
fallback for what the Android driver can't do (`mesa-run <app>` forces it).
Desktop OpenGL ≤ 2.1 apps (e.g. Minecraft ≤ 1.16) run on the GPU through
gl4es with `gl4es-run`.

```
 app (GLES, Wayland) ──EGL──► glvnd ─► libhybris ─► /vendor/lib64/egl/libGLES_mali.so ─► /dev/mali0
        │ gralloc buffer (wayland-android protocol)
        ▼
 sway ─ wlroots (patched) ─ GLES2 renderer on libhybris ─► gralloc buffer ─ copy (damage only) ─► /dev/fb0
```

## Pieces

| Package (`hybris/pkg/`) | What |
|---|---|
| `armtix-hybris-gsi` | `/system` + `/apex`: hybris-patched bionic and the ~20 Android system libraries the driver loads, from the Halium 16 arm64 GSI (3 MB, see `PROVENANCE`). Stock Android bionic can't work: its TLS slots collide with glibc's. |
| `libhybris-armtix` | libhybris as a **GLVND EGL vendor**, ordered *after* Mesa (`60_libhybris.json`), so outside the GPU sway session (software fallback) apps get Mesa. `armtix-sway` puts it first for its apps (`__EGL_VENDOR_LIBRARY_FILENAMES`), Mesa second for GBM/X11/desktop GL; sway asks for the Android platform, which only libhybris provides. `hybris-run <app>` (GPU only) and `mesa-run <app>` (software) override it per app. Private libdir `/usr/lib/libhybris`. Patched for running **without any Android userspace** (properties from `/etc/hybris/props.conf` + build.prop files, gralloc HAL loaded directly, no hwservicemanager) and for a few bugs, see below. |
| `wlroots0.20-hybris` | Drop-in `wlroots0.20` for sway with an **fbdev backend** (the decon display has no KMS; fixed 60 Hz frame clock, damage-only copies on 4 threads with a NEON red/blue swap: `WLR_FBDEV_THREADS`, `WLR_FBDEV_STATS=1` logs the timings) and the **GLES2 renderer on libhybris**: gralloc render buffers (`EGL_NATIVE_BUFFER_ANDROID`), GPU clients through `EGL_WL_bind_wayland_display`. |
| `libudev-zero-armtix` | libudev without udevd (which doesn't run on 4.4), so libinput finds the touchscreen/keys; only used by the sway session. |
| `wvkbd` | On-screen keyboard (not in the repositories). |
| `gl4es-armtix` | [gl4es](https://github.com/ptitSeb/gl4es): desktop OpenGL 2.1 → OpenGL ES 2 on the Mali GPU. `gl4es-run <app>` for Wayland/EGL apps (EGL wrapper patched to offer desktop GL configs and API), `gl4es-x11-run <app>` for X11/GLX apps under Xwayland (pbuffer + copy, ~15 fps cap: the driver reads its tiled buffers back slowly). |
| `glfw-minecraft` | GLFW 3.5.1 in `/usr/lib/glfw-minecraft` that doesn't report Wayland's unsupported features (window icon/position, cursor position) as errors: old Minecraft treats any GLFW error as fatal. |
| `armtix-camera` | Simple camera app for the rear camera: raw Bayer frames from the FIMC-IS V4L2 driver (no ISP; needs the kernel's raw-capture changes in `fimc-is-device-sensor_v2.c` and `fimc-is-video-sensor.c`, part of `kernel/armtix-kernel.patch`), CPU debayer, auto exposure + gray-world white balance, SDL3 preview on the GPU, 12 MP JPEGs in `~/Pictures`. Boot service `armtix-camera` gives the video nodes to group video. Firmware: the installer copies it from VENDOR. |
| `fex-emu-armv8.0-bin` | [FEX-Emu](https://fex-emu.com) (x86/x86-64 on arm64), the upstream ARMv8.0 build (no LSE atomics on this CPU) plus `fex-compat.so`, which emulates what FEX needs from kernels newer than 4.4 (`MAP_FIXED_NOREPLACE`, CPU ID register reads, `openat2`, ...). `fex-run <program>`, `steam-fex` (see below). |
| `lan-mouse-armtix` | [lan-mouse](https://github.com/feschber/lan-mouse) 0.11 receiver: use a PC's keyboard and mouse on the phone (see below). |

Device side (not packaged, no Samsung blobs here): the TWRP installer copies
the phone's `/vendor/lib64` (Mali r16p0, gralloc, ...) and `/vendor/build.prop`
from the VENDOR partition. The Mali **userspace version must match the kernel
driver** (r16p0 in `armtix_dream2lte_defconfig`), which is the case for
Samsung's Android 9 based firmware and ROMs built on it.

Rootfs (`rootfs/overlay`): `armtix-gpu` (boot: `/dev/mali0`, `/dev/ion`,
`/dev/fb0` to group video), `seatd`, `armtix-sway` (session launcher, falls
back to software rendering if the GPU path can't start), sway/waybar/foot/
fuzzel/mako/foot configs as a theme (`armtix-theme`, themes in
`~/.config/armtix-themes`), `armtix-osk` (keyboard toggle), `armtix-rotate`
(also switches between the portrait and landscape wallpaper),
`armtix-power-menu`, `armtix-mc-gpu` (Minecraft wrapper), and a `bwrap`
wrapper (see below). tty1 autologin starts sway (XFCE and Xorg were removed;
Xwayland stays for X11 apps).

`tools/hybris_deploy.sh [ssh-target]` sets all of this up on a phone installed
from an older zip.

## x86 programs and Steam (FEX-Emu)

Once, as the user: `FEXRootFSFetcher -y -x --distro-name=ubuntu
--distro-version=24.04` (x86 Ubuntu RootFS, ~500 MB download, 1.9 GB on
disk). Then `fex-run <x86 program>`.

`steam-fex` sets up and starts the Steam client (needs the kernel's user
namespaces and SysV IPC, both in `armtix_dream2lte_defconfig`). Games: game
*Properties → Compatibility → "Legacy runtime 1.0"* (the newer Steam Linux
Runtime containers don't start on this kernel).

x86 games run as X11 apps under Xwayland and render **in software**: the
Mali driver is arm64-only and can't be used from x86 code. By default FEX
emulates the game's x86 Mesa too, which is slow; FEX's **GL thunks** send
the OpenGL calls to the native arm64 Mesa (llvmpipe) instead, so only the
game is emulated. Per game, e.g. Half-Life,
`~/.config/fex-emu/AppConfig/hl_linux.json` (file name = the executable):

```json
{
  "Config": { "HostEnv": ["GALLIUM_DRIVER=llvmpipe"] },
  "ThunksDB": { "GL": 1 }
}
```

`guest/sdl-nomsaa.c` is a preload for SDL2 games that insist on multisampled
GLX visuals, which llvmpipe under Xwayland doesn't offer (Half-Life asks for
4x MSAA even with `-nomsaa`). Build it as 32-bit x86 on a PC (command in
its header), copy it to `~/.local/lib/fex-guest/sdl-nomsaa.so` and set the
game's launch options in Steam to
`LD_PRELOAD=/home/armtix/.local/lib/fex-guest/sdl-nomsaa.so:$LD_PRELOAD %command%`.

## Minecraft (Prism Launcher)

Not in the image: install `prismlauncher` and `glfw-minecraft` (from here)
first. Minecraft up to 1.16.5 (OpenGL 2.1) runs on the GPU through gl4es,
natively on Wayland; ~40–50 fps at low settings, render distance 6–8. Newer
versions need OpenGL 3.2 core, which gl4es doesn't provide. In the instance
settings (*Edit instance*):

- **Version**: change *LWJGL 3* to **3.3.3** (Wayland support, arm64 natives).
- **Settings → Java**: maximum memory **1536 MB** (the phone has 3.6 GB);
  JVM arguments:
  `-Dorg.lwjgl.opengl.libname=/usr/lib/gl4es/libGL.so.1 -Dorg.lwjgl.egl.libname=/usr/lib/gl4es/libEGL.so.1`
  (otherwise LWJGL takes the GL functions straight from the Mali driver,
  bypassing gl4es).
- **Settings → Workarounds**: *Use system installation of GLFW*, path
  `/usr/lib/glfw-minecraft/libglfw.so.3`; *Wrapper command*
  `/usr/local/bin/armtix-mc-gpu`.

## Keyboard and mouse from a PC (lan-mouse)

Move the PC's cursor past a screen edge and it continues on the phone, keyboard
included (Minecraft's mouse capture works too). Over the USB network
(`172.16.42.1`) it's smooth and doesn't depend on WiFi.

On the PC (`lan-mouse` package, Arch: `extra/lan-mouse`), `~/.config/lan-mouse/config.toml`:

```toml
[[clients]]
position = "right"            # edge of the PC's screen
hostname = "armtix-dream2lte"
ips = ["172.16.42.1"]
activate_on_startup = true
```

Run `lan-mouse daemon` (on Hyprland/sway: `lan-mouse --capture-backend
layer-shell daemon`) once to create its certificate, then authorize it on the
phone in `~/.config/lan-mouse/config.toml`:

```toml
port = 4242
[authorized_fingerprints]
"<sha256 of the PC's ~/.config/lan-mouse/lan-mouse.pem>" = "my-pc"
```

(`openssl x509 -in ~/.config/lan-mouse/lan-mouse.pem -noout -fingerprint -sha256`,
in lower case.) sway starts the receiver when that file exists. Back to the
PC: LeftCtrl+LeftShift+LeftMeta+LeftAlt.

The PC needs a fixed address on the USB link, or NetworkManager keeps trying
DHCP and drops it:

```sh
nmcli connection modify <usb connection> ipv4.method manual \
    ipv4.addresses 172.16.42.2/24 ipv4.never-default yes ipv6.method disabled
```

## Building

On ARMtix (aarch64), as a user with sudo:

```sh
sudo pacman -S --needed base-devel git meson ninja cmake glslang vulkan-headers \
    wayland-protocols xorg-xwayland libdisplay-info libliftoff scdoc python rust
hybris/build-packages.sh          # -> hybris/out/*.pkg.tar.*
sudo pacman -U hybris/out/*.pkg.tar.*
```

The 4.4 kernel has no SysV IPC, which `fakeroot` (used by makepkg) needs:
build fakeroot with `./configure --with-ipc=tcp` into `/usr/local` first and
run `PATH=/usr/local/bin:$PATH hybris/build-packages.sh`. Natively on the
phone wlroots builds in about two minutes, lan-mouse in about ten.

## Things that bit (and how they're handled)

- **CPU reads of GPU buffers need `glFinish()`**: Android's gralloc `lock()`
  doesn't wait for the GPU (Android passes fences), so the fbdev copy read
  half-rendered Mali tiles → garbage blocks on screen. The hybris buffer waits
  for the GPU before the CPU maps it. The fbdev backend also cleans the
  written cache lines (`dc cvac`; `WLR_FBDEV_CLEAN_CACHE=0` turns that off).
- **Frame pacing**: scheduling the next frame a refresh period after each
  (slow) commit halved the frame rate; frames now follow a fixed clock.
- **sway has `cap_sys_nice`**, so the dynamic linker ignores
  `LD_LIBRARY_PATH` (no libudev-zero → no input devices). `armtix-sway`
  starts it through `ld-linux-aarch64.so.1` instead.
- **Firefox on the GPU hung with an empty window**: libhybris wrapped
  `eglSwapBuffers` and `eglSwapBuffersWithDamageEXT` but not the **KHR**
  variant, which Firefox uses: the driver's own function queued buffers the
  Wayland platform never attached, and after 3 frames `dequeueBuffer` waited
  forever for a release. Now routed through the same wrapper.
- **Qt 6 (Telegram, Kvant, ...) found no EGLConfig**: Arch builds Qt for
  desktop GL, so it binds the GLES API but asks for `EGL_OPENGL_BIT` configs.
  libhybris now offers the GLES configs then (only when the GLES API is
  bound: apps that bound desktop GL get nothing and fall back), Qt sees no
  `EGL_OPENGL_BIT` on the config and switches itself to GLES;
  `eglCreateContext` clamps the version to 3.2 and drops desktop-only
  attributes.
- GLVND picks the vendor once per display, so an app that *needs* desktop
  OpenGL over Wayland EGL (some SDL/GLFW games) can't fall back to Mesa by
  itself in the sway session: `mesa-run <app>` or `gl4es-run <app>`. GLX
  apps under Xwayland always use Mesa.
- libhybris bugs fixed in `libhybris-armtix.patch`: its Wayland platform
  stopped waiting for the `android_wlegl` global after the first batch of
  events (random crash in `eglCreateWindowSurface`); a request for the
  Android platform (gl4es probing the GPU) unloaded the Wayland window
  system of the same process; GL apps that exit without tearing down EGL
  crashed in Mali's exit handlers.
- The fuel gauge reports `charge_now=1`, so waybar's battery module shows 0%;
  the bar reads `/sys/class/power_supply/battery/capacity` itself.
- **Image loading needs a `bwrap` wrapper**: gdk-pixbuf decodes images in
  glycin loaders inside a bubblewrap sandbox, and bubblewrap 0.13 needs mount
  syscalls the 4.4 kernel lacks ("Can't open source /usr: Function not
  implemented"), so no wallpaper and no images in GTK apps. glycin ignores
  `GLYCIN_DISABLE_SANDBOX` in Arch's build; `/usr/local/bin/bwrap` runs the
  glycin loaders directly and passes everything else to the real bwrap.
- **sway sometimes crashed after `swaymsg reload`** (double free), so
  `armtix-theme` applies a theme with runtime commands and reloads waybar
  with SIGUSR2 instead.
- **Camera init at boot hung the boot**: reading
  `/sys/class/camera/rear/fw_update` makes the driver request firmware,
  which only `armtix-fwload` answers (no udev); before it runs every request
  waits for a long timeout, and the audio DSP's firmware request behind them
  timed out too. The camera app does that read itself.
- bcmdhd WiFi power saving drops the connection now and then; the `wifi`
  service turns it off.
- Building inside a chroot on the *Android* kernel hangs `meson install`
  (random-pool kernel bug); build natively on ARMtix instead.

## Checking

```sh
eglinfo -B -p wayland | grep -i vendor          # in sway: "Android" = libhybris
mesa-run eglinfo -B -p wayland | grep -i vendor # "Mesa Project"
grep -c libGLES_mali /proc/$(pgrep -xo firefox)/maps  # > 0: app is on the GPU
gl4es-run eglgears_wayland                     # desktop GL on the GPU
grep -iE 'hybris|GL renderer' ~/.local/state/sway.log
```

`WLR_RENDERER=pixman armtix-sway` forces software rendering;
`HYBRIS_EGLPLATFORM=null` runs a GLES program headless.
