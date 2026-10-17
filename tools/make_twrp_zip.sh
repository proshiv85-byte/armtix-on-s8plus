#!/usr/bin/env bash
# Build a TWRP-flashable ARMtix zip plus work/deploy/boot-armtix.img.
#
#   tools/make_twrp_zip.sh [ssh-target]           full desktop image: the
#                                                 RUNNING phone's rootfs
#   tools/make_twrp_zip.sh --server [ssh-target]  minimal server image, built
#                                                 on the phone in a chroot
#                                                 (tools/build_server_rootfs.sh)
#   tools/make_twrp_zip.sh [--server] --reuse-rootfs
#                                                 no phone: reuse the staged
#                                                 rootfs.tar.xz + caps.txt, only
#                                                 refresh installer/boot.img/README
# ssh-target defaults to root@172.16.42.1 (needs root to read the rootfs).
#
# The rootfs is sanitized by tools/rootfs_export.py (no WiFi passwords, SSH
# host keys, logs, caches, browser/app data, Samsung blobs; passwords reset
# to "armtix"). The home directory comes from work/rootfs/overlay, not the
# phone, and personal apps (DROP_PKGS below) are left out. Output: work/deploy/ARMtix-dream2lte[-server]-<date>.zip
set -euo pipefail
W="$(cd "$(dirname "$0")/.." && pwd)"
VARIANT=full
REUSE=0
while [ $# -gt 0 ]; do
	case $1 in
		--server) VARIANT=server; shift ;;
		--reuse-rootfs) REUSE=1; shift ;;
		*) break ;;
	esac
done
TARGET=${1:-root@172.16.42.1}
DATE=$(date +%Y%m%d)
if [ $VARIANT = server ]; then
	OUT="$W/work/deploy/ARMtix-dream2lte-server-$DATE.zip"
	STAGE="$W/work/twrp-zip-server"
	README="$W/docs/TWRP-ZIP-README-server.txt"
else
	OUT="$W/work/deploy/ARMtix-dream2lte-$DATE.zip"
	STAGE="$W/work/twrp-zip"
	README="$W/docs/TWRP-ZIP-README.txt"
fi
# Inputs: the development tree's work/ when there is one, otherwise the
# repository layout (rootfs/, prebuilt/, the overlay's static busybox)
RFS="$W/work/rootfs"; [ -d "$RFS/overlay" ] || RFS="$W/rootfs"
BOOT="$W/work/deploy/boot-armtix.img"; [ -f "$BOOT" ] || BOOT="$W/prebuilt/boot-armtix.img"
BB="$W/work/initramfs/dl/ex/bin/busybox.static"; [ -f "$BB" ] || BB="$RFS/overlay/usr/local/sbin/busybox"
mkdir -p "$W/work/deploy"
SSH=(ssh -o ConnectTimeout=10 -o StrictHostKeyChecking=accept-new ${SSH_EXTRA:-} "$TARGET")

# Files of the (desktop) overlay that the server image uses, plus its own
# overrides in work/rootfs/server-overlay
SERVER_FILES="
etc/conf.d/agetty.console etc/conf.d/agetty.ttySAC2 etc/conf.d/hostname
etc/dhcpcd.conf etc/fstab etc/hostname etc/securetty
etc/sysctl.d/90-armtix-swap.conf etc/ssh/sshd_config etc/armtix/screen.conf
etc/sudoers.d/10-wheel etc/sudoers.d/30-armtix-power etc/sudoers.d/40-armtix-screen
etc/init.d/agetty.console etc/init.d/agetty.ttySAC2 etc/init.d/armtix-bootcheck
etc/init.d/armtix-fwload etc/init.d/armtix-rundir etc/init.d/armtix-screen
etc/init.d/usbnet etc/init.d/wifi
etc/local.d/console.start etc/local.d/zswap.start
usr/local/bin/armtix-screen
usr/local/sbin/armtix-fwload usr/local/sbin/armtix-power usr/local/sbin/busybox
usr/local/sbin/httpd usr/share/udhcpc/default.script var/www/cgi-bin/status.sh
"

# Desktop image: packages installed on the phone that don't belong in it
# (personal apps and what only they need). Left out of the image's files and
# pacman database; the phone keeps them.
DROP_PKGS="telegram-desktop kvant prismlauncher jre17-openjdk glfw-minecraft glfw"
KEEP_PKGS="xdg-utils"	# would go with them, but the image wants it

# Paths owned only by DROP_PKGS and their no longer needed dependencies,
# plus their pacman database entries (runs on the phone, removes nothing)
drop_list() {
	"${SSH[@]}" "DROP='$DROP_PKGS' KEEP='$KEEP_PKGS' sh -s" <<'EOS'
set -e
pkgs=$(pacman -Rsp --print-format %n $DROP | grep -vxF -e "$(printf '%s\n' $KEEP)" || true)
[ -n "$pkgs" ] || exit 0
echo "$pkgs" > /tmp/armtix-drop-pkgs
pacman -Qlq $pkgs | sort -u > /tmp/armtix-drop-own
pacman -Qq | grep -vxF -f /tmp/armtix-drop-pkgs | xargs pacman -Qlq | sort -u > /tmp/armtix-keep-own
comm -23 /tmp/armtix-drop-own /tmp/armtix-keep-own
for p in $pkgs; do
	d=/var/lib/pacman/local/$(pacman -Q "$p" | tr ' ' -)
	find "$d"
done
rm -f /tmp/armtix-drop-pkgs /tmp/armtix-drop-own /tmp/armtix-keep-own
EOS
}

server_overlay_tar() {
	local t; t=$(mktemp -d)
	(cd "$RFS/overlay" && cp -a --parents $SERVER_FILES "$t/")
	cp -a "$RFS/server-overlay/." "$t/"
	chmod 440 "$t"/etc/sudoers.d/*
	tar -C "$t" --owner=0 --group=0 --numeric-owner -cf - .
	rm -rf "$t"
}

if [ $REUSE = 1 ]; then
	for f in rootfs.tar.xz sha256sums.rootfs caps.txt; do
		[ -f "$STAGE/armtix/$f" ] || { echo "no $STAGE/armtix/$f - run a full build first" >&2; exit 1; }
	done
	rm -rf "$STAGE/META-INF"; mkdir -p "$STAGE/META-INF/com/google/android"
else
	rm -rf "$STAGE"; mkdir -p "$STAGE/META-INF/com/google/android" "$STAGE/armtix"
fi
cp "$W/tools/twrp/update-binary" "$STAGE/META-INF/com/google/android/update-binary"
echo "# dummy - the installer is update-binary" > "$STAGE/META-INF/com/google/android/updater-script"
cp "$BOOT" "$STAGE/armtix/boot.img"
cp "$BB" "$STAGE/armtix/busybox"
cp "$README" "$STAGE/README.txt"
KVER=$(strings "$W/work/deploy/Image" 2>/dev/null | grep -m1 'Linux version' | sed -E 's/Linux version ([^ ]+).* (#[0-9]+) .*/\1 \2/' || true)
printf 'BUILD_DATE=%s\nKERNEL_VERSION="%s"\nVARIANT=%s\n' "$DATE" "$KVER" "$VARIANT" > "$STAGE/armtix/build-info"
(cd "$STAGE/armtix" && sha256sum boot.img > sha256sums.boot)

if [ $REUSE = 0 ]; then
HASH=$(openssl passwd -6 armtix)
if [ $VARIANT = server ]; then
	echo ">> server rootfs (built on $TARGET)"
	server_overlay_tar | "${SSH[@]}" 'rm -rf /tmp/armtix-server-ov && mkdir -p /tmp/armtix-server-ov && tar -xpf - -C /tmp/armtix-server-ov'
	"${SSH[@]}" 'sh -s' < "$W/tools/build_server_rootfs.sh"
	R=/home/armtix/srv-build/root
	echo ">> file capabilities"
	"${SSH[@]}" "getcap -r $R/usr 2>/dev/null" | awk -v r="$R" '{sub("^" r, "", $1); print $1, $2}' > "$STAGE/armtix/caps.txt"
	cat "$STAGE/armtix/caps.txt"
	echo ">> rootfs (streamed, sanitized, xz)"
	"${SSH[@]}" "tar -C $R --numeric-owner -cpf - . 2>/dev/null; true" |
		python3 "$W/tools/rootfs_export.py" --shadow-hash "$HASH" |
		xz -T0 -6 > "$STAGE/armtix/rootfs.tar.xz"
else
	echo ">> file capabilities"
	"${SSH[@]}" 'getcap -r /usr 2>/dev/null' | awk '{print $1, $2}' > "$STAGE/armtix/caps.txt"
	cat "$STAGE/armtix/caps.txt"
	echo ">> packages left out: $DROP_PKGS (+ dependencies)"
	drop_list > "$STAGE/drop-list.txt"
	echo "   $(wc -l < "$STAGE/drop-list.txt") paths"
	echo ">> rootfs (streamed from $TARGET, sanitized, xz)"
	"${SSH[@]}" 'sync; tar -C / --one-file-system --numeric-owner -cpf - \
		--exclude=./swapfile --exclude=./vendor --exclude="./var/cache/pacman/pkg/*" \
		--exclude=./home/armtix/.local/share/PrismLauncher --exclude=./home/armtix/.config/mozilla \
		--exclude=./home/armtix/.local/share/Steam --exclude=./home/armtix/.local/share/fex-emu \
		--exclude=./home/armtix/.local/share/TelegramDesktop --exclude=./home/armtix/srv-build \
		--exclude=./home/armtix/.cache --exclude=./root/boot-new.img --exclude="./tmp/*" \
		--exclude="./var/tmp/*" --exclude="./mnt/system/*" . 2>/dev/null; true' |
		python3 "$W/tools/rootfs_export.py" --shadow-hash "$HASH" \
			--drop-list "$STAGE/drop-list.txt" --home "$RFS/overlay/home/armtix" |
		xz -T0 -6 > "$STAGE/armtix/rootfs.tar.xz"
fi
(cd "$STAGE/armtix" && sha256sum rootfs.tar.xz > sha256sums.rootfs)
else
	echo ">> reusing $STAGE/armtix/rootfs.tar.xz (not contacting the phone)"
	(cd "$STAGE/armtix" && sha256sum -c sha256sums.rootfs)
fi
ls -la "$STAGE/armtix"

echo ">> zip"
rm -f "$OUT"
(cd "$STAGE" && zip -r -0 -q "$OUT" META-INF armtix README.txt)
ls -la "$OUT"; sha256sum "$OUT" | tee "$OUT.sha256"
