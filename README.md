# ARMtix on the Galaxy S8+ (Exynos)

Artix Linux (ARMtix, aarch64, **OpenRC**) running natively on the
**Samsung Galaxy S8+ SM-G955F ("dream2lte", Exynos 8895)**, with a
GPU-accelerated **sway** desktop on the phone's screen. No Android runs.

There are two kernels to choose from, each with its own TWRP zip:

- **mainline** (Linux 7.3 with the
  [dream2lte-mainline](https://github.com/proshiv85-byte/dream2lte-mainline)
  patches, *new, in development*): the GPU runs on Mesa's open `panfrost`
  driver, Bluetooth, headphones and hardware video decoding work; no camera
  and no loudspeaker yet.
- **4.4** (Samsung's downstream 4.4.194 kernel with patches): the GPU runs on
  the phone's Android driver through libhybris; camera, loudspeaker and
  FEX-Emu/Steam work; no Bluetooth.

The rootfs is the same ARMtix on both; the zips differ in the kernel, what
the installer copies from the phone, and a few kernel-specific settings.

## What works

| | mainline kernel | 4.4 kernel |
|---|---|---|
| Display | 1440×2960 AMOLED, DRM driver; brightness, screen off; sway at scale ×3 | Samsung `decon` framebuffer: sway with an fbdev backend, scale ×3 |
| GPU | Mali-G71 with Mesa **panfrost** (OpenGL ES and OpenGL): sway and Wayland apps (Firefox, GTK, Qt, SDL) render on it | Mali-G71 with the Android driver through **libhybris**; desktop OpenGL ≤ 2.1 via gl4es (`gl4es-run`). See [`hybris/`](hybris/README.md) |
| Video decoding | hardware (MFC): H.264, HEVC, VP8, VP9 through V4L2; Firefox plays H.264/HEVC in hardware (patched libavcodec) | software |
| Touch | multitouch screen (libinput), on-screen keyboard (wvkbd) | same |
| WiFi | BCM4361 (`brcmfmac`), wpa_supplicant + dhcpcd, touch-friendly picker | bcmdhd4361, same tools |
| Bluetooth | BCM4361 (BlueZ): pairing, A2DP headphones | — |
| Sound | headphone jack, USB audio, Bluetooth headphones; loudspeaker silent for now | loudspeaker (ABOX DSP + MAX98506), volume keys |
| Camera | — | rear camera (12 MP): **Camera** app with live preview, JPEG photos in `~/Pictures` |
| x86 apps | — (untested) | **FEX-Emu** (`fex-run`): Steam runs (`steam-fex`), Half-Life is playable (see [`hybris/`](hybris/README.md#x86-programs-and-steam-fex-emu)) |
| CPU | frequency scaling (up to 2.3 GHz), idle states, thermal throttling | Samsung's |
| USB | RNDIS network to a PC (`ssh armtix@172.16.42.1`), **OTG host** | same |
| Charging, battery | MAX77865: charge current by charger type, temperature limits | MAX77865 |
| microSD | read at ~3 MB/s (low card clock for now) | yes |

Both: power key → power menu, volume keys, rotation, lan-mouse (the PC's
cursor continues on the phone), `armtix-screen on/off`, 4 GiB swap + zswap,
themes (`armtix-theme`; the mainline image starts with the dusk theme and its
wallpaper, the 4.4 images with the minimal one).

Not working on either: calls/SMS, GPS, fingerprint, automatic screen
blanking. Mainline also: camera, loudspeaker, earpiece, microphones, suspend.

## Install (TWRP zip)

Download a zip from the Releases page:

- `ARMtix-dream2lte-mainline-*.zip`: the desktop with the **mainline** kernel;
- `ARMtix-dream2lte-*.zip`: the desktop with the **4.4** kernel;
- `ARMtix-dream2lte-server-*.zip`: console only (login prompt on the
  screen), SSH, WiFi and USB networking, 4.4 kernel; `armtix-screen off`
  turns the display off.

> ⚠️ Flashing **erases internal storage** (USERDATA). The SD card is not touched.
> Needs an unlocked bootloader, TWRP, and an Android ROM with Samsung's
> vendor blobs still on SYSTEM/VENDOR (stock Android 9, ROMs based on it, or
> an AOSP build for dream2lte using the same blobs). No Samsung firmware is
> redistributed here: the installer copies it from the phone. The 4.4 zips
> take the WiFi, audio and camera firmware and the Mali GPU driver (r16p0);
> the mainline zip takes the WiFi, Bluetooth, audio and video decoder
> firmware from VENDOR, and the CPU frequency plugin from the Samsung kernel
> in BOOT (or from TWRP's kernel when BOOT already holds mainline).

TWRP → Install → the zip → swipe, then **Reboot → System**.
Login `armtix` / `armtix` (root: `armtix`), change them with `passwd`.
The phone boots straight into sway. WiFi: "Wi-Fi" in the app menu.

Over USB the phone is `172.16.42.1` and the PC needs `172.16.42.2/24` on the
USB network interface.

## Repository layout

| Path | |
|---|---|
| `kernel/armtix-kernel.patch` | all 4.4 kernel changes against `BASE_COMMIT` (USB, display, boot logo, camera raw capture, ...) |
| `kernel/armtix_dream2lte_defconfig`, `kernel/dts/` | 4.4 kernel config and the dream2lte device tree |
| `rootfs/overlay/` | files added to the ARMtix rootfs: services, sway session and theme, scripts (`armtix-*`) |
| `rootfs/server-overlay/` | what the server image does differently |
| `hybris/` | packages (PKGBUILDs + patches): libhybris, Halium bionic subset, wlroots with fbdev backend + libhybris renderer, gl4es, FEX-Emu, the camera app, lan-mouse, wvkbd; `build-packages.sh`. See [`hybris/README.md`](hybris/README.md) |
| `tools/` | `build_boot.sh` + `repack_boot.py` + `make_initramfs.sh` (4.4 boot image); `make_twrp_zip.sh` + `rootfs_export.py` + `build_server_rootfs.sh` + `twrp/update-binary` (4.4 TWRP zips + installer); `hybris_deploy.sh` |
| `docs/` | the READMEs that go into the 4.4 zips |
| `prebuilt/boot-armtix.img` | ready-to-flash 4.4 boot image (kernel + DTBH dtb + initramfs) |

The mainline kernel, its boot shim and the mainline zip's builder and
installer live in
[dream2lte-mainline](https://github.com/proshiv85-byte/dream2lte-mainline).

## Building

**4.4 kernel**: Samsung's `exynos-linux-stable` dreamlte tree (`tw90-android`,
see `kernel/BASE_COMMIT`), apply `kernel/armtix-kernel.patch`, use
`kernel/armtix_dream2lte_defconfig`, then (GCC 16 cross compiler)
`tools/build_boot.sh`. Important: the DT must be packed as a Samsung **DTBH**
image with `dt_size` set in the boot header, or S-Boot ignores it;
`tools/repack_boot.py` enforces this.

**Mainline kernel**: see the
[dream2lte-mainline README](https://github.com/proshiv85-byte/dream2lte-mainline#building).

**Rootfs**: ARMtix OpenRC base tarball + `rootfs/overlay` + the packages from
`hybris/` (built natively on the phone, see `hybris/README.md`).

**TWRP zips**, from a phone running ARMtix (needs root SSH access):

```sh
tools/make_twrp_zip.sh                # 4.4 desktop image, from the running phone
tools/make_twrp_zip.sh --server       # server image, built on the phone in a chroot
                                      # (+ --proxy: downloads through the PC, no WiFi needed)
tools/make_twrp_zip.sh --reuse-rootfs # repack only (installer, boot image, README)
```

The mainline image is built the same way, from a phone running the mainline
kernel, with `tools/make-twrp-zip.sh` in dream2lte-mainline (it uses
`tools/rootfs_export.py` and `rootfs/overlay` from here).

The desktop images are the phone's rootfs, cleaned by
`tools/rootfs_export.py`: no WiFi passwords, SSH keys, logs, caches or
Samsung blobs; passwords reset to `armtix`; the home directory comes from
`rootfs/overlay`, not the phone; personal apps listed in `DROP_PKGS`
(`make_twrp_zip.sh`) are left out.

## Credits

Port by t.me/smasnug_ports, with Claude (Anthropic) as engineering assistant.
Kernels: Samsung / exynos-linux-stable, Linux mainline. Distribution: Artix
Linux / ARMtix. GPU: Mesa (panfrost); libhybris, Halium. x86: FEX-Emu.
