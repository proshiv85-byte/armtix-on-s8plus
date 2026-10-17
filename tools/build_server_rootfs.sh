#!/bin/sh
# Build a minimal ARMtix "server" root filesystem ON THE PHONE, as root:
# base + OpenRC + SSH, WiFi and USB networking; no desktop, no GPU stack.
# Packages come from the phone's own pacman (repos + keyring).
#
# tools/make_twrp_zip.sh --server runs it: it unpacks the server overlay at
# $OV (default /tmp/armtix-server-ov) and streams $B/root (default
# /home/armtix/srv-build/root) back into the zip.
set -eu
B=${B:-/home/armtix/srv-build}
R=$B/root
# Package cache outside the root (kept between runs; can be pre-filled when
# the phone's own downloads are unreliable: see PKG_URLS below)
CACHE=$B/pkg
OV=${OV:-/tmp/armtix-server-ov}
PKGS="base openrc elogind-openrc dbus-openrc artix-mirrorlist
      openssh openssh-openrc sudo wpa_supplicant wpa_supplicant-openrc
      dhcpcd dhcpcd-openrc iw nano htop python libcap"

cleanup() {
	for m in dev/pts dev proc sys; do umount -l "$R/$m" 2>/dev/null || true; done
}
trap cleanup EXIT

[ -d "$OV/etc" ] || { echo "no overlay at $OV" >&2; exit 1; }
cleanup
rm -rf "$R"
mkdir -p "$R/var/lib/pacman" "$R/var/cache/pacman/pkg" "$R/dev" "$R/proc" "$R/sys" "$R/etc" "$CACHE"
mount --bind /dev "$R/dev"; mount --bind /dev/pts "$R/dev/pts" 2>/dev/null || true
mount -t proc proc "$R/proc"; mount -t sysfs sys "$R/sys"

# PKG_URLS=1: only print the package URLs (to download them elsewhere into
# $CACHE), don't install
if [ "${PKG_URLS:-0}" = 1 ]; then
	pacman -r "$R" --cachedir "$CACHE" -Sy >/dev/null
	pacman -r "$R" --cachedir "$CACHE" -Sp --noconfirm $PKGS
	exit 0
fi

echo ">> installing packages"
# The host's pacman.conf (repos, no sandbox on this kernel) and keyring
pacman -r "$R" --cachedir "$CACHE" -Sy --noconfirm $PKGS 2>&1 |
	grep -vE "Protocol driver not attached|udev.conf|^error: command failed to execute correctly" || true
[ -x "$R/usr/bin/openrc-init" ] || { echo "openrc missing" >&2; exit 1; }

echo ">> system configuration"
cp /etc/pacman.conf "$R/etc/pacman.conf"
cp /etc/pacman.d/mirrorlist "$R/etc/pacman.d/mirrorlist" 2>/dev/null || true
cp -a "$OV/." "$R/"
echo 'LANG="C.UTF-8"' > "$R/etc/locale.conf"
chroot "$R" pacman-key --init >/dev/null 2>&1 || true
chroot "$R" pacman-key --populate artix >/dev/null 2>&1 || true

echo ">> users and groups"
# esysusers (systemd-sysusers) can't create the system accounts here: it
# needs statx() (kernel 4.11+). Take the running ARMtix system's account files
# instead (system accounts + armtix; rootfs_export.py resets the passwords).
for f in passwd group shadow gshadow; do cp -p /etc/$f "$R/etc/$f"; done
mkdir -p "$R/home/armtix"
cp -a "$R/etc/skel/." "$R/home/armtix/" 2>/dev/null || true
chroot "$R" chown -R armtix:armtix /home/armtix
chmod 700 "$R/home/armtix"

echo ">> services"
enable() {	# enable <runlevel> <service>...
	lvl=$1; shift
	for s in "$@"; do
		if [ -e "$R/etc/init.d/$s" ]; then
			chroot "$R" rc-update add "$s" "$lvl" >/dev/null || echo "  could not add $s"
		else
			echo "  (no service $s, skipped)"
		fi
	done
}
enable sysinit cgroups devfs dmesg etmpfiles-dev kmod-static-nodes sysfs udev udev-trigger
enable boot armtix-bootcheck armtix-fwload armtix-rundir binfmt bootmisc esysusers \
	etmpfiles-setup fsck hostname hwclock keymaps localmount loopback modules mtab \
	net.lo procfs root save-keymaps save-termencoding seedrng swap sysctl termencoding
enable default agetty.tty1 agetty.ttySAC2 armtix-screen dbus local netmount sshd usbnet wifi
# openrc enables gettys on tty1-6 itself; the phone only shows tty1
for n in 2 3 4 5 6; do chroot "$R" rc-update del agetty.tty$n default >/dev/null 2>&1 || true; done
for lvl in sysinit boot default; do echo "  $lvl: $(ls "$R/etc/runlevels/$lvl" | tr '\n' ' ')"; done

rm -rf "$R/var/cache/pacman/pkg/"*
echo ">> done: $(du -sh "$R" | cut -f1) in $R"
