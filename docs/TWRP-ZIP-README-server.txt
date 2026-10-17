ARMtix Linux (Artix, OpenRC) for Samsung Galaxy S8+ Exynos — SM-G955F "dream2lte"
Server image (minimal: no desktop)
===============================================================================

!!! Flashing this zip ERASES the phone's internal storage (USERDATA) !!!
    Back up first (TWRP -> Backup). The SD card is not touched.

What's in it
  Artix base + OpenRC, OpenSSH, sudo, WiFi (wpa_supplicant + dhcpcd), USB
  networking, nano, htop, python. About 1 GB installed, 4 GB swap file.
  More software: `sudo pacman -S <package>`.

Requirements
  - SM-G955F with an unlocked bootloader and TWRP.
  - SYSTEM/VENDOR must still contain a Samsung Android 9 based ROM: the WiFi
    firmware is copied from it during installation.

Before the first boot (optional, in TWRP after installing)
  TWRP -> Mount -> tick "Data", then Advanced -> Terminal (the new
  system is at /data):
  - Screen off from the start:
      echo SCREEN=off > /data/etc/armtix/screen.conf
  - WiFi (instead of setting it up over USB later), one line:
      printf 'network={\n ssid="YourNetwork"\n psk="YourPassword"\n}\n' >> /data/etc/wpa_supplicant/wpa_supplicant.conf

Install
  1. TWRP -> Install -> select this zip -> swipe.
  2. (optional: the settings above)
  3. TWRP -> Reboot -> System.

Access
  - USB cable to a Linux PC: phone = 172.16.42.1 -> ssh armtix@172.16.42.1
  - WiFi: ssh armtix@<address from your router>
  - Login armtix / armtix (root: armtix). Change both: `passwd`, `sudo passwd root`.
  - The phone screen shows a normal login prompt (when the screen is on).

Screen
  - `armtix-screen off` / `on` / `toggle` / `status` (add --save to keep the
    setting for the next boot).
  - Boot default: /etc/armtix/screen.conf (SCREEN=on or SCREEN=off), editable
    from TWRP as /data/etc/armtix/screen.conf.

WiFi later
  - `sudo nano /etc/wpa_supplicant/wpa_supplicant.conf`, add
        network={
            ssid="YourNetwork"
            psk="YourPassword"
        }
    then `sudo rc-service wifi restart`.
  - Or: `sudo wpa_cli -i wlan0` (add_network / set_network / save_config).

Power
  - `sudo armtix-power poweroff | reboot | recovery` (recovery = TWRP).

Known limitations
  - No calls/SMS, camera, Bluetooth, GPS. Audio is not set up.

Back to Android: restore your TWRP backup (at least BOOT + DATA).
