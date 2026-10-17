ARMtix Linux (Artix, OpenRC) for Samsung Galaxy S8+ Exynos — SM-G955F "dream2lte"
Desktop image (sway, GPU-accelerated)
===============================================================================

!!! Flashing this zip ERASES the phone's internal storage (USERDATA) !!!
    Back up first (TWRP -> Backup). The SD card is not touched.

Requirements
  - SM-G955F with an unlocked bootloader and TWRP.
  - SYSTEM/VENDOR must still contain an Android ROM with Samsung's vendor
    blobs (stock Android 9, ROMs based on it, or an AOSP build for dream2lte
    using the same blobs): WiFi/audio/camera firmware and the Mali GPU driver
    (r16p0) are copied from it during installation (this zip contains no
    Samsung blobs).

Install
  1. TWRP -> Install -> select this zip -> swipe.  (takes a few minutes)
  2. TWRP -> Reboot -> System.

First boot
  - sway starts automatically (user armtix, password armtix; root: armtix).
    Change both with `passwd` / `sudo passwd root`.
  - Bottom bar: apps, on-screen keyboard, rotate, close window, power menu.
    Look: `armtix-theme` lists the themes, `armtix-theme modern` switches
    (default: minimal; themes in ~/.config/armtix-themes).
    Own wallpaper: copy it to ~/.config/armtix-themes/<theme>/sway/
    wallpaper.png (portrait) and wallpaper-landscape.png (optional, used
    when rotated), then run `armtix-theme <theme>` again.
    WiFi: "Wi-Fi" in the app menu. Power key: power menu. Volume keys: volume.
  - USB cable to a Linux PC: network, phone = 172.16.42.1
    (ssh armtix@172.16.42.1).
  - GPU: the Mali driver through libhybris. Apps in sway use it by default;
    `mesa-run <app>` forces software rendering, `gl4es-run <app>` runs
    desktop OpenGL 2.1 apps on the GPU.

Screen on/off
  - `armtix-screen off` / `armtix-screen on` (add --save to keep it at boot).
  - Boot default: /etc/armtix/screen.conf (SCREEN=on or SCREEN=off).
    From TWRP: Mount -> Data, then Advanced -> Terminal ->
      echo SCREEN=off > /data/etc/armtix/screen.conf

Camera
  - "Camera" in the app menu: rear camera, tap the round button (or Space)
    to take a photo; photos are saved in ~/Pictures (12 MP JPEG).

x86 programs and Steam (FEX-Emu)
  - Once: `FEXRootFSFetcher -y -x --distro-name=ubuntu --distro-version=24.04`
    (~500 MB download, 1.9 GB on disk).
  - `fex-run <x86 program>`; Steam: `steam-fex` (to get a menu entry:
    rm ~/.local/share/applications/steam-fex.desktop).
  - Games: Steam -> game Properties -> Compatibility -> "Legacy runtime 1.0".

Known limitations
  - No calls/SMS, Bluetooth, GPS, fingerprint. Camera: rear only, fixed focus.
  - The screen does not turn off by itself (see armtix-screen).

Back to Android: restore your TWRP backup (at least BOOT + DATA).
