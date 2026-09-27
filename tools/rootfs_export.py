#!/usr/bin/env python3
"""Sanitize an ARMtix rootfs tar stream for redistribution (TWRP zip).

    ssh phone 'tar -C / --one-file-system --numeric-owner -cpf - .' |
        rootfs_export.py --shadow-hash HASH --xorg-conf FILE > rootfs.tar

Drops personal/machine-specific data (WiFi passwords, SSH host keys, ids,
leases, logs, caches, browser/app data, Samsung blobs in /vendor, swapfile)
and resets passwords + rotation to defaults. Owners are kept numeric.
"""
import argparse, fnmatch, io, sys, tarfile

# glob patterns on the normalized path (no leading "./"); a match excludes the
# entry (and, for directories matched with "/*", only their contents)
EXCLUDE = [
    "swapfile",
    "vendor", "vendor/*",                       # Samsung firmware: installer copies it from SYSTEM
    "var/cache/pacman/pkg/*",
    "var/log/*.log", "var/log/*.log.*", "var/log/wtmp", "var/log/btmp",
    "var/log/lastlog", "var/log/dmesg", "var/log/old/*", "var/log/*/*",
    "var/tmp/*", "tmp/*",
    "var/lib/dhcpcd/*",
    "var/lib/seedrng/*",
    "etc/machine-id", "var/lib/dbus/machine-id",
    "etc/ssh/ssh_host_*",
    "etc/X11/armtix-rotation",
    "etc/pacman.conf.bak-armtixp", "etc/dhcpcd.conf.bak-armtixp",
    "root/.*", "root/*",                        # debug files; root/disabled-services is kept below
    "mnt/system/*",
    "home/armtix/.bash_history", "home/armtix/.python_history", "home/armtix/.lesshst",
    "home/armtix/.cache", "home/armtix/.cache/*",
    "home/armtix/.mozilla", "home/armtix/.mozilla/*",
    "home/armtix/.ssh", "home/armtix/.ssh/*", "home/armtix/.gnupg", "home/armtix/.gnupg/*",
    "home/armtix/.pki", "home/armtix/.pki/*", "home/armtix/.dbus", "home/armtix/.dbus/*",
    "home/armtix/.Xauthority", "home/armtix/.ICEauthority", "home/armtix/.serverauth.*",
    "home/armtix/.xsession.log*", "home/armtix/.xsession-errors*", "home/armtix/.onboard.log",
    "home/armtix/.config/mozilla", "home/armtix/.config/mozilla/*",
    "home/armtix/.config/Thunar", "home/armtix/.config/Thunar/*",
    "home/armtix/.config/armtix-volume-set",
    "home/armtix/.config/pulse/*",
    "home/armtix/.local/share/*", "home/armtix/.local/state/*",
    "home/armtix/Downloads/*", "home/armtix/Desktop/*",
]
KEEP = [                                        # re-include (checked first)
    "root/disabled-services", "root/disabled-services/*",
    "home/armtix/.config/pulse/default.pa",
    "home/armtix/.local/share/applications", "home/armtix/.local/share/applications/*",
]

WPA_TEMPLATE = b"""# group wheel: the desktop user can manage WiFi (armtix-wifi)
ctrl_interface=DIR=/run/wpa_supplicant GROUP=wheel
update_config=1
country=US

# Add networks with the "Wi-Fi" button on the panel, or:
#   wpa_cli -i wlan0   (add_network / set_network / save_config)
"""
RESOLV = b"# written by dhcpcd / usbnet at boot\n"


def excluded(path):
    if any(fnmatch.fnmatchcase(path, p) for p in KEEP):
        return False
    return any(fnmatch.fnmatchcase(path, p) for p in EXCLUDE)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--shadow-hash", required=True, help="crypt hash for root + armtix")
    ap.add_argument("--xorg-conf", required=True, help="clean 10-armtix-dream2lte.conf")
    a = ap.parse_args()
    replace = {
        "etc/wpa_supplicant/wpa_supplicant.conf": WPA_TEMPLATE,
        "etc/X11/xorg.conf.d/10-armtix-dream2lte.conf": open(a.xorg_conf, "rb").read(),
        "etc/resolv.conf": RESOLV,
    }
    src = tarfile.open(fileobj=sys.stdin.buffer, mode="r|")
    dst = tarfile.open(fileobj=sys.stdout.buffer, mode="w|", format=tarfile.PAX_FORMAT)
    n_in = n_out = 0
    for m in src:
        n_in += 1
        path = m.name[2:] if m.name.startswith("./") else m.name
        if path in ("", "."):
            dst.addfile(m); continue
        if excluded(path):
            continue
        m.uname = m.gname = ""                   # numeric owners only
        m.pax_headers = {k: v for k, v in m.pax_headers.items()
                         if not k.startswith("SCHILY.xattr")}
        if path == "etc/shadow" and m.isfile():
            lines = src.extractfile(m).read().decode().splitlines(True)
            out = []
            for l in lines:
                f = l.split(":")
                if f[0] in ("root", "armtix"):
                    f[1] = a.shadow_hash
                out.append(":".join(f))
            data = "".join(out).encode()
            m.size = len(data)
            dst.addfile(m, io.BytesIO(data))
        elif path in replace and m.isfile():
            data = replace[path]
            m.size = len(data)
            dst.addfile(m, io.BytesIO(data))
        elif m.isfile():
            dst.addfile(m, src.extractfile(m))
        else:
            dst.addfile(m)
        n_out += 1
    dst.close()
    print(f"rootfs_export: {n_in} entries in, {n_out} out", file=sys.stderr)


if __name__ == "__main__":
    main()
