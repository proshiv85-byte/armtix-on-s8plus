# ARMtix on the Galaxy S8+ (Exynos)

Artix Linux (ARMtix, aarch64, **OpenRC**) running natively on the
**Samsung Galaxy S8+ SM-G955F ("dream2lte", Exynos 8895)**, with an XFCE
desktop on the phone's screen. It uses Samsung's downstream 4.4.194 kernel with
patches to run a normal Linux userland (no Android).

## What works

| | |
|---|---|
| Display | 1440×2960 AMOLED via the Samsung `decon` framebuffer: **sway** (default) or Xorg `fbdev` + XFCE (HiDPI ×2) |
| GPU | Mali-G71 with the phone's Android driver through **libhybris**: sway composites with OpenGL ES; GLES apps (`hybris-run`) and desktop OpenGL ≤ 2.1 apps via gl4es (`gl4es-run`) on the GPU, e.g. **Minecraft 1.16.5 at ~40–50 fps** (see [`hybris/`](hybris/README.md)) |
| Boot | kernel boot logo (one Tux per CPU core) |
| PC keyboard/mouse | lan-mouse over the USB network: the PC's cursor continues on the phone |
| Touch | multitouch screen (Xorg `evdev`), on-screen keyboard (Onboard) |
| Rotation | portrait / landscape (USB right) button on the panel |
| WiFi | bcmdhd4361, wpa_supplicant + dhcpcd, touch-friendly picker on the panel |
| USB | RNDIS network to a PC (`ssh root@172.16.42.1`), **OTG host**: USB keyboard / mouse / hubs (hotplug) |
| Sound | loudspeaker (ABOX DSP + MAX98506), PulseAudio, volume keys + OSD |
| Buttons | power key → power menu (power off / reboot / recovery), volume keys |
| Charging, battery | MAX77865 |
| Swap | 4 GiB swapfile + zswap |

Not working / not done: GPU acceleration for XFCE and big apps (Firefox, GTK/Qt
toolkits use Mesa's software renderer), OpenGL > 2.1, fast GPU rendering for
X11 apps, calls/SMS,
camera, Bluetooth, GPS, fingerprint, headphone jack/mic (untested), screen
blanking, and a **cold power-on boots TWRP** (use TWRP → Reboot → System).

## Install (TWRP zip)

Download `ARMtix-dream2lte-*.zip` from the Releases page.

> ⚠️ Flashing **erases internal storage** (USERDATA). The SD card is not touched.
> Needs an unlocked bootloader, TWRP, and a Samsung-based Android still on
> SYSTEM/VENDOR: WiFi and audio firmware and the Mali GPU driver (r16p0) are
> copied from it during install (no Samsung firmware is redistributed here).

TWRP → Install → the zip → swipe, then **Reboot → System**.
Login `armtix` / `armtix` (root: `armtix`) — change them with `passwd`.
The phone boots into sway; `echo xfce > ~/.config/armtix-session` switches the
screen session to XFCE.

## Repository layout

| Path | |
|---|---|
| `kernel/armtix-kernel.patch` | all kernel changes against `BASE_COMMIT` |
| `kernel/armtix_dream2lte_defconfig`, `kernel/dts/` | kernel config and the dream2lte device tree |
| `rootfs/overlay/` | files added to the ARMtix rootfs (services, Xorg config, scripts) |
| `tools/` | `build_boot.sh` + `repack_boot.py` + `make_initramfs.sh` (boot image), `make_twrp_zip.sh` + `rootfs_export.py` + `twrp/update-binary` (TWRP zip + installer) |
| `hybris/` | GPU support: libhybris, Halium bionic subset, wlroots with fbdev backend + libhybris renderer, packaging (`build-packages.sh`); `tools/hybris_deploy.sh` sets it up on a running phone |
| `prebuilt/boot-armtix.img` | ready-to-flash boot image (kernel + DTBH dtb + initramfs) |

## Building

Kernel: Samsung's `exynos-linux-stable` dreamlte tree (`tw90-android`, see
`kernel/BASE_COMMIT`), apply `kernel/armtix-kernel.patch`, use
`kernel/armtix_dream2lte_defconfig`, then (GCC 16 cross compiler)
`tools/build_boot.sh`. Important: the DT must be packed as a Samsung **DTBH**
image with `dt_size` set in the boot header, or S-Boot ignores it —
`tools/repack_boot.py` enforces this.

Rootfs: ARMtix OpenRC base tarball + `rootfs/overlay`.

TWRP zip: `tools/make_twrp_zip.sh` (from a running phone) or
`tools/make_twrp_zip.sh --reuse-rootfs`.

## Credits

Port by t.me/smasnug_ports, with Claude (Anthropic) as engineering assistant.
Kernel: Samsung / exynos-linux-stable. Distribution: Artix Linux / ARMtix.
