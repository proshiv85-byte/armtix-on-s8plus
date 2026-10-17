# ARMtix on the Galaxy S8+ (Exynos)

Artix Linux (ARMtix, aarch64, **OpenRC**) running natively on the
**Samsung Galaxy S8+ SM-G955F ("dream2lte", Exynos 8895)**, with a
GPU-accelerated **sway** desktop on the phone's screen. It uses Samsung's
downstream 4.4.194 kernel with patches to run a normal Linux userland: no
Android runs, its GPU driver is used through libhybris.

## What works

| | |
|---|---|
| Display | 1440×2960 AMOLED via the Samsung `decon` framebuffer: **sway** (wlroots with an fbdev backend), scale ×3 |
| GPU | Mali-G71 with the phone's Android driver through **libhybris**: sway composites with OpenGL ES, and Wayland EGL apps (Firefox, GTK 4, Qt 6, SDL) render on the GPU; desktop OpenGL ≤ 2.1 apps via gl4es (`gl4es-run`), e.g. Minecraft 1.16.5 at ~40–50 fps. See [`hybris/`](hybris/README.md) |
| x86 apps | **FEX-Emu** with a shim for the 4.4 kernel (`fex-run`): Steam runs (`steam-fex`), Half-Life is playable |
| Camera | rear camera (12 MP): **Camera** app with live preview, auto exposure / white balance, JPEG photos in `~/Pictures` |
| Desktop | minimal theme (JetBrains Mono, warm grays): status bar on top, apps / keyboard / rotate / close / power buttons at the bottom; launcher (fuzzel), notifications (mako), terminal (foot). `armtix-theme` switches themes |
| Touch | multitouch screen (libinput), on-screen keyboard (wvkbd) |
| Rotation | portrait / landscape (USB right), separate wallpaper per orientation |
| PC keyboard/mouse | lan-mouse over the USB network: the PC's cursor continues on the phone |
| WiFi | bcmdhd4361, wpa_supplicant + dhcpcd, touch-friendly picker |
| USB | RNDIS network to a PC (`ssh armtix@172.16.42.1`), **OTG host**: USB keyboard / mouse / hubs (hotplug) |
| Sound | loudspeaker (ABOX DSP + MAX98506), PulseAudio, volume keys |
| Buttons | power key → power menu (power off / reboot / recovery), volume keys |
| Screen | `armtix-screen on/off`; boot default in `/etc/armtix/screen.conf` (editable from TWRP) |
| Charging, battery | MAX77865 |
| Swap | 4 GiB swapfile + zswap |

Not working / not done: calls/SMS, Bluetooth, GPS, fingerprint, the front
camera and autofocus, OpenGL > 2.1, headphone jack/mic (untested) and
automatic screen blanking.

## Install (TWRP zip)

Download `ARMtix-dream2lte-*.zip` from the Releases page. There are two
images:

- **desktop**: the sway desktop described above;
- **server** (`-server`): console only (login prompt on the screen), SSH,
  WiFi and USB networking; `armtix-screen off` turns the display off.

> ⚠️ Flashing **erases internal storage** (USERDATA). The SD card is not touched.
> Needs an unlocked bootloader, TWRP, and an Android ROM with Samsung's
> vendor blobs still on SYSTEM/VENDOR (stock Android 9, ROMs based on it, or
> an AOSP build for dream2lte using the same blobs): the WiFi, audio and
> camera firmware and the Mali GPU driver (r16p0) are copied from it during
> install. No Samsung firmware is redistributed here.

TWRP → Install → the zip → swipe, then **Reboot → System**.
Login `armtix` / `armtix` (root: `armtix`), change them with `passwd`.
The phone boots straight into sway. WiFi: "Wi-Fi" in the app menu.

Over USB the phone is `172.16.42.1` and the PC needs `172.16.42.2/24` on the
USB network interface.

## Repository layout

| Path | |
|---|---|
| `kernel/armtix-kernel.patch` | all kernel changes against `BASE_COMMIT` (USB, display, boot logo, camera raw capture, ...) |
| `kernel/armtix_dream2lte_defconfig`, `kernel/dts/` | kernel config and the dream2lte device tree |
| `rootfs/overlay/` | files added to the ARMtix rootfs: services, sway session and theme, scripts (`armtix-*`) |
| `rootfs/server-overlay/` | what the server image does differently |
| `hybris/` | packages (PKGBUILDs + patches): libhybris, Halium bionic subset, wlroots with fbdev backend + libhybris renderer, gl4es, FEX-Emu, the camera app, lan-mouse, wvkbd; `build-packages.sh`. See [`hybris/README.md`](hybris/README.md) |
| `tools/` | `build_boot.sh` + `repack_boot.py` + `make_initramfs.sh` (boot image); `make_twrp_zip.sh` + `rootfs_export.py` + `build_server_rootfs.sh` + `twrp/update-binary` (TWRP zips + installer); `hybris_deploy.sh` |
| `docs/` | the READMEs that go into the zips |
| `prebuilt/boot-armtix.img` | ready-to-flash boot image (kernel + DTBH dtb + initramfs) |

## Building

**Kernel**: Samsung's `exynos-linux-stable` dreamlte tree (`tw90-android`,
see `kernel/BASE_COMMIT`), apply `kernel/armtix-kernel.patch`, use
`kernel/armtix_dream2lte_defconfig`, then (GCC 16 cross compiler)
`tools/build_boot.sh`. Important: the DT must be packed as a Samsung **DTBH**
image with `dt_size` set in the boot header, or S-Boot ignores it;
`tools/repack_boot.py` enforces this.

**Rootfs**: ARMtix OpenRC base tarball + `rootfs/overlay` + the packages from
`hybris/` (built natively on the phone, see `hybris/README.md`).

**TWRP zips**, from a phone running ARMtix (needs root SSH access):

```sh
tools/make_twrp_zip.sh                # desktop image, from the running phone
tools/make_twrp_zip.sh --server       # server image, built on the phone in a chroot
tools/make_twrp_zip.sh --reuse-rootfs # repack only (installer, boot image, README)
```

The desktop image is the phone's rootfs, cleaned by `tools/rootfs_export.py`:
no WiFi passwords, SSH keys, logs, caches or Samsung blobs; passwords reset
to `armtix`; the home directory comes from `rootfs/overlay`, not the phone;
personal apps listed in `DROP_PKGS` (`make_twrp_zip.sh`) are left out.

## Credits

Port by t.me/smasnug_ports, with Claude (Anthropic) as engineering assistant.
Kernel: Samsung / exynos-linux-stable. Distribution: Artix Linux / ARMtix.
GPU: libhybris, Halium. x86: FEX-Emu.
