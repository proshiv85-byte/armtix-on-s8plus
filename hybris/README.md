# GPU acceleration (Mali-G71) via libhybris, sway

The Exynos 8895's Mali-G71 has no usable open driver for this kernel, so ARMtix
runs the phone's **Android Mali driver** inside normal glibc programs with
[libhybris](https://github.com/libhybris/libhybris). The desktop for it is
**sway** (wlroots), compositing with OpenGL ES on the GPU. Apps use Mesa
(software) by default and opt in to the GPU: OpenGL ES apps with `hybris-run`,
desktop OpenGL ≤ 2.1 apps (e.g. Minecraft ≤ 1.16) through gl4es with
`gl4es-run`.

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
| `libhybris-armtix` | libhybris as a **GLVND EGL vendor**, ordered *after* Mesa (`60_libhybris.json`): apps use Mesa (llvmpipe) by default, because big apps (Firefox, Qt/GTK toolkits) need EGL features the Android driver lacks; sway asks for the Android platform, which only libhybris provides. `hybris-run <app>` puts a GLES app on the GPU. Private libdir `/usr/lib/libhybris`. Patched for running **without any Android userspace** (properties from `/etc/hybris/props.conf` + build.prop files, gralloc HAL loaded directly, no hwservicemanager) and for a few bugs, see below. |
| `wlroots0.20-hybris` | Drop-in `wlroots0.20` for sway with an **fbdev backend** (the decon display has no KMS; fixed 60 Hz frame clock, damage-only copies) and the **GLES2 renderer on libhybris**: gralloc render buffers (`EGL_NATIVE_BUFFER_ANDROID`), GPU clients through `EGL_WL_bind_wayland_display`. |
| `libudev-zero-armtix` | libudev without udevd (which doesn't run on 4.4), so libinput finds the touchscreen/keys; only used by the sway session. |
| `wvkbd` | On-screen keyboard (not in the repositories). |
| `gl4es-armtix` | [gl4es](https://github.com/ptitSeb/gl4es): desktop OpenGL 2.1 → OpenGL ES 2 on the Mali GPU. `gl4es-run <app>` for Wayland/EGL apps (EGL wrapper patched to offer desktop GL configs and API), `gl4es-x11-run <app>` for X11/GLX apps under Xwayland (pbuffer + copy, ~15 fps cap: the driver reads its tiled buffers back slowly). |
| `glfw-minecraft` | GLFW 3.5.1 in `/usr/lib/glfw-minecraft` that doesn't report Wayland's unsupported features (window icon/position, cursor position) as errors: old Minecraft treats any GLFW error as fatal. |
| `lan-mouse-armtix` | [lan-mouse](https://github.com/feschber/lan-mouse) 0.11 receiver: use a PC's keyboard and mouse on the phone (see below). |

Device side (not packaged, no Samsung blobs here): the TWRP installer copies
the phone's `/vendor/lib64` (Mali r16p0, gralloc, ...) and `/vendor/build.prop`
from the VENDOR partition. The Mali **userspace version must match the kernel
driver** (r16p0 in `armtix_dream2lte_defconfig`), which is the case for
Samsung's Android 9 based firmware and ROMs built on it.

Rootfs (`rootfs/overlay`): `armtix-gpu` (boot: `/dev/mali0`, `/dev/ion`,
`/dev/fb0` to group video), `seatd`, `armtix-sway` (session launcher, falls
back to software rendering if the GPU path can't start), sway/waybar/foot/
fuzzel/mako configs, `armtix-osk` (keyboard toggle), sway-aware
`armtix-rotate` and `armtix-power-menu`, `armtix-mc-gpu` (Minecraft wrapper).
tty1 autologin starts sway; `echo xfce > ~/.config/armtix-session` switches
back to XFCE.

`tools/hybris_deploy.sh [ssh-target]` sets all of this up on a phone installed
from an older zip.

## Minecraft (Prism Launcher)

Minecraft up to 1.16.5 (OpenGL 2.1) runs on the GPU through gl4es, natively
on Wayland; ~40–50 fps at low settings, render distance 6–8. Newer versions
need OpenGL 3.2 core, which gl4es doesn't provide. In the instance settings
(*Edit instance*):

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
- With libhybris as the *first* EGL vendor, Firefox opened a blank window
  and other big apps misbehaved → apps default to Mesa.
- libhybris bugs fixed in `libhybris-armtix.patch`: its Wayland platform
  stopped waiting for the `android_wlegl` global after the first batch of
  events (random crash in `eglCreateWindowSurface`); a request for the
  Android platform (gl4es probing the GPU) unloaded the Wayland window
  system of the same process; GL apps that exit without tearing down EGL
  crashed in Mali's exit handlers.
- The fuel gauge reports `charge_now=1`, so waybar's battery module shows 0%;
  the bar reads `/sys/class/power_supply/battery/capacity` itself.
- bcmdhd WiFi power saving drops the connection now and then; the `wifi`
  service turns it off.
- Building inside a chroot on the *Android* kernel hangs `meson install`
  (random-pool kernel bug); build natively on ARMtix instead.

## Checking

```sh
eglinfo -p wayland | grep -i vendor            # apps: "Mesa Project"
hybris-run eglinfo -p wayland | grep -i vendor # "Android" = libhybris
gl4es-run eglgears_wayland                     # desktop GL on the GPU
grep -iE 'hybris|GL renderer' ~/.local/state/sway.log
```

`WLR_RENDERER=pixman armtix-sway` forces software rendering;
`HYBRIS_EGLPLATFORM=null` runs a GLES program headless.
