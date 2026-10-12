#!/usr/bin/env bash
# Set up GPU acceleration (libhybris) + sway on a RUNNING ARMtix phone over
# ssh (USB network), for installs made from a zip without them:
#
#   tools/hybris_deploy.sh [ssh-target]      (default root@172.16.42.1)
#
# Installs the hybris/out packages and sway, copies the Mali driver from the
# phone's VENDOR partition and the session files from rootfs/overlay, and
# enables the services. The phone needs internet (WiFi) for pacman.
set -euo pipefail
W="$(cd "$(dirname "$0")/.." && pwd)"
TARGET=${1:-root@172.16.42.1}
SSH=(ssh -o ConnectTimeout=10 -o StrictHostKeyChecking=accept-new ${SSH_EXTRA:-} "$TARGET")
O="$W/rootfs/overlay"

ls "$W"/hybris/out/*.pkg.tar.* >/dev/null || {
	echo "no packages in hybris/out - run hybris/build-packages.sh on ARMtix first" >&2; exit 1; }

echo "==> packages"
"${SSH[@]}" 'mkdir -p /root/hybris-pkgs && rm -f /root/hybris-pkgs/*'
scp -q "$W"/hybris/out/*.pkg.tar.* "$TARGET:/root/hybris-pkgs/"
"${SSH[@]}" 'set -e
	pacman -Syu --needed --noconfirm --ask 4 sway swaybg seatd seatd-openrc foot fuzzel waybar mako \
		grim slurp wl-clipboard ttf-font-awesome mesa-utils libnotify yad dbus \
		xorg-xwayland xorg-xrandr glfw
	# the hybris wlroots replaces stock wlroots0.20 (sway links against it)
	pacman -U --noconfirm --ask 4 --overwrite "/system/*" --overwrite "/apex/*" \
		/root/hybris-pkgs/*.pkg.tar.*'

echo "==> Mali driver from VENDOR"
"${SSH[@]}" 'set -e
	dev=$(grep -l "^PARTNAME=VENDOR$" /sys/class/block/*/uevent | head -1 | xargs dirname | xargs basename)
	[ -n "$dev" ] || { echo "VENDOR partition not found" >&2; exit 1; }
	mkdir -p /mnt/vendor-ro
	mount -o ro /dev/$dev /mnt/vendor-ro
	if [ -f /mnt/vendor-ro/lib64/egl/libGLES_mali.so ]; then
		rm -rf /vendor/lib64 && cp -a /mnt/vendor-ro/lib64 /vendor/
		cp -a /mnt/vendor-ro/build.prop /vendor/ 2>/dev/null || true
		grep -q r16p0 /vendor/lib64/egl/libGLES_mali.so || echo "WARNING: not a Mali r16p0 driver"
		echo "copied $(du -sh /vendor/lib64 | cut -f1) from /dev/$dev"
	else
		echo "WARNING: no Mali driver on /dev/$dev" >&2
	fi
	umount /mnt/vendor-ro'

echo "==> session files"
tar -C "$O" -cf - \
	etc/init.d/armtix-gpu etc/runlevels/boot/armtix-gpu \
	usr/local/bin/armtix-sway usr/local/bin/armtix-osk usr/local/bin/armtix-rotate \
	usr/local/bin/armtix-power-menu usr/local/bin/armtix-mc-gpu etc/init.d/wifi \
	home/armtix/.bash_profile home/armtix/.config/sway home/armtix/.config/waybar \
	home/armtix/.config/foot home/armtix/.config/fuzzel home/armtix/.config/mako |
	"${SSH[@]}" 'tar -C / --no-same-owner -xf - && chown -R armtix: /home/armtix/.bash_profile /home/armtix/.config'

echo "==> services, groups"
"${SSH[@]}" 'set -e
	getent group seat >/dev/null || groupadd -r seat
	usermod -aG video,input,seat armtix
	rc-service armtix-gpu start
	rc-service seatd start || true
	echo "done - restart the screen session: rc-service agetty.tty1 restart"'
