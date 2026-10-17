#!/usr/bin/env python3
"""Sanitize an ARMtix rootfs tar stream for redistribution (TWRP zip).

    ssh phone 'tar -C / --one-file-system --numeric-owner -cpf - .' |
        rootfs_export.py --shadow-hash HASH [--xorg-conf FILE]
                         [--drop-list FILE] [--home DIR] > rootfs.tar

--drop-list: paths to leave out (one per line, e.g. the files of packages
that are on the phone but don't belong in the image, see make_twrp_zip.sh).
--home: the user's home comes from DIR (the curated overlay) instead of the
phone, so photos, app data, game shortcuts etc. never end up in the image.

Drops personal/machine-specific data (WiFi passwords, SSH host keys, ids,
leases, logs, caches, browser/app data, Samsung blobs in /vendor, swapfile)
and resets passwords + rotation to defaults. Owners are kept numeric.
"""
import argparse, fnmatch, io, os, stat, sys, tarfile

# glob patterns on the normalized path (no leading "./"); a match excludes the
# entry (and, for directories matched with "/*", only their contents)
EXCLUDE = [
    "swapfile",
    "vendor", "vendor/*",                       # Samsung firmware: installer copies it from SYSTEM
    "etc/wifi", "etc/wifi/*",                   # old copy of the WiFi firmware (uses /vendor/etc/wifi)
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
    "home/armtix/.config/lan-mouse", "home/armtix/.config/lan-mouse/*",   # authorized PC fingerprint
    "home/armtix/.java", "home/armtix/.java/*",
    "home/armtix/.config/pulse/*",
    "home/armtix/.local/share/*", "home/armtix/.local/state/*",
    "home/armtix/Downloads/*", "home/armtix/Desktop/*",
    # build trees and per-user setups (FEX RootFS + Steam are in .local/share)
    "home/armtix/ClassiCube", "home/armtix/ClassiCube/*",
    "home/armtix/yay-bin", "home/armtix/yay-bin/*",
    "home/armtix/srv-build", "home/armtix/srv-build/*",
    "home/armtix/.steam", "home/armtix/.steam/*", "home/armtix/.steampath", "home/armtix/.steampid",
    "home/armtix/core",
]
KEEP = [                                        # re-include (checked first)
    "root/disabled-services", "root/disabled-services/*",
    "home/armtix/.config/pulse/default.pa",
    "home/armtix/.local/share/applications", "home/armtix/.local/share/applications/*",
]

WPA_TEMPLATE = b"""# group wheel: the armtix user can manage WiFi (armtix-wifi, wpa_cli)
ctrl_interface=DIR=/run/wpa_supplicant GROUP=wheel
update_config=1
country=US

# Add networks with "Wi-Fi" in the desktop's app menu, `wpa_cli -i wlan0`
# (add_network / set_network / save_config), or a block like this one,
# then `sudo rc-service wifi restart`:
#
# network={
#     ssid="YourNetwork"
#     psk="YourPassword"
# }
"""
RESOLV = b"# written by dhcpcd / usbnet at boot\n"


def excluded(path):
    if any(fnmatch.fnmatchcase(path, p) for p in KEEP):
        return False
    return any(fnmatch.fnmatchcase(path, p) for p in EXCLUDE)


USER_UID = USER_GID = 1000
HOME = "home/armtix"
HOME_DIRS = ["Pictures", "Downloads"]           # created empty


def add_home(dst, root):
    """Add the home directory's contents from the overlay at `root`."""
    def add(path, rel):
        st = os.lstat(path)
        t = tarfile.TarInfo(f"./{HOME}/{rel}")
        t.uid, t.gid, t.mtime = USER_UID, USER_GID, int(st.st_mtime)
        t.mode = stat.S_IMODE(st.st_mode)
        if stat.S_ISDIR(st.st_mode):
            t.type = tarfile.DIRTYPE
            dst.addfile(t)
        elif stat.S_ISLNK(st.st_mode):
            t.type, t.linkname = tarfile.SYMTYPE, os.readlink(path)
            dst.addfile(t)
        else:
            t.size = st.st_size
            with open(path, "rb") as f:
                dst.addfile(t, f)
    n = 0
    for d, dirs, files in os.walk(root):
        dirs.sort()
        for name in sorted(dirs) + sorted(files):
            p = os.path.join(d, name)
            add(p, os.path.relpath(p, root)); n += 1
    for name in HOME_DIRS:
        if not os.path.exists(os.path.join(root, name)):
            t = tarfile.TarInfo(f"./{HOME}/{name}")
            t.type, t.mode, t.uid, t.gid = tarfile.DIRTYPE, 0o755, USER_UID, USER_GID
            dst.addfile(t); n += 1
    return n


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--shadow-hash", required=True, help="crypt hash for root + armtix")
    ap.add_argument("--xorg-conf", help="clean 10-armtix-dream2lte.conf (Xorg/XFCE images only)")
    ap.add_argument("--drop-list", help="file with paths to leave out, one per line")
    ap.add_argument("--home", help="take home/armtix from this directory")
    a = ap.parse_args()
    drop = set()
    if a.drop_list:
        with open(a.drop_list) as f:
            drop = {l.strip().strip("/") for l in f if l.strip()}
    replace = {
        "etc/wpa_supplicant/wpa_supplicant.conf": WPA_TEMPLATE,
        "etc/resolv.conf": RESOLV,
    }
    if a.xorg_conf:
        replace["etc/X11/xorg.conf.d/10-armtix-dream2lte.conf"] = open(a.xorg_conf, "rb").read()
    src = tarfile.open(fileobj=sys.stdin.buffer, mode="r|")
    dst = tarfile.open(fileobj=sys.stdout.buffer, mode="w|", format=tarfile.PAX_FORMAT)
    n_in = n_out = 0
    for m in src:
        n_in += 1
        path = m.name[2:] if m.name.startswith("./") else m.name
        if path in ("", "."):
            dst.addfile(m); continue
        if excluded(path) or path.rstrip("/") in drop:
            continue
        if a.home and path.startswith(HOME + "/"):
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
    if a.home:
        n_out += add_home(dst, a.home)
    dst.close()
    print(f"rootfs_export: {n_in} entries in, {n_out} out", file=sys.stderr)


if __name__ == "__main__":
    main()
