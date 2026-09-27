#!/usr/bin/env bash
# Build a TWRP-flashable ARMtix zip from the RUNNING phone (over ssh) plus
# work/deploy/boot-armtix.img.
#
#   tools/make_twrp_zip.sh [ssh-target]     (default root@172.16.42.1)
#   tools/make_twrp_zip.sh --reuse-rootfs   (no phone: reuse the rootfs.tar.xz,
#                                            caps.txt in work/twrp-zip; only
#                                            refresh installer/boot.img/README)
#
# The rootfs is sanitized by tools/rootfs_export.py (no WiFi passwords, SSH
# host keys, logs, caches, browser/app data, Samsung blobs; passwords reset
# to "armtix"). Output: work/deploy/ARMtix-dream2lte-<date>.zip
set -euo pipefail
W="$(cd "$(dirname "$0")/.." && pwd)"
REUSE=0
if [ "${1:-}" = --reuse-rootfs ]; then REUSE=1; shift; fi
TARGET=${1:-root@172.16.42.1}
DATE=$(date +%Y%m%d)
OUT="$W/work/deploy/ARMtix-dream2lte-$DATE.zip"
STAGE="$W/work/twrp-zip"
BOOT="$W/work/deploy/boot-armtix.img"
BB="$W/work/initramfs/dl/ex/bin/busybox.static"
XORG="$W/work/rootfs/overlay/etc/X11/xorg.conf.d/10-armtix-dream2lte.conf"
SSH=(ssh -o ConnectTimeout=10 -o StrictHostKeyChecking=accept-new ${SSH_EXTRA:-} "$TARGET")

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
cp "$W/docs/TWRP-ZIP-README.txt" "$STAGE/README.txt" 2>/dev/null || true
KVER=$(strings "$W/work/deploy/Image" | grep -m1 'Linux version' | sed -E 's/Linux version ([^ ]+).* (#[0-9]+) .*/\1 \2/' || true)
printf 'BUILD_DATE=%s\nKERNEL_VERSION="%s"\n' "$DATE" "$KVER" > "$STAGE/armtix/build-info"
(cd "$STAGE/armtix" && sha256sum boot.img > sha256sums.boot)

if [ $REUSE = 0 ]; then
echo ">> file capabilities"
"${SSH[@]}" 'getcap -r /usr 2>/dev/null' | awk '{print $1, $2}' > "$STAGE/armtix/caps.txt"
cat "$STAGE/armtix/caps.txt"

echo ">> rootfs (streamed from $TARGET, sanitized, xz)"
HASH=$(openssl passwd -6 armtix)
"${SSH[@]}" 'sync; tar -C / --one-file-system --numeric-owner -cpf - \
	--exclude=./swapfile --exclude=./vendor --exclude="./var/cache/pacman/pkg/*" \
	--exclude=./home/armtix/.local/share/PrismLauncher --exclude=./home/armtix/.config/mozilla \
	--exclude=./home/armtix/.cache --exclude=./root/boot-new.img --exclude="./tmp/*" \
	--exclude="./var/tmp/*" --exclude="./mnt/system/*" . 2>/dev/null; true' |
	python3 "$W/tools/rootfs_export.py" --shadow-hash "$HASH" --xorg-conf "$XORG" |
	xz -T0 -6 > "$STAGE/armtix/rootfs.tar.xz"
(cd "$STAGE/armtix" && sha256sum rootfs.tar.xz > sha256sums.rootfs)
else
	echo ">> reusing $STAGE/armtix/rootfs.tar.xz (not contacting the phone)"
	(cd "$STAGE/armtix" && sha256sum -c sha256sums.rootfs)
fi
ls -la "$STAGE/armtix"

echo ">> zip"
rm -f "$OUT"
(cd "$STAGE" && zip -r -0 -q "$OUT" META-INF armtix README.txt 2>/dev/null || zip -r -0 -q "$OUT" META-INF armtix)
ls -la "$OUT"; sha256sum "$OUT" | tee "$OUT.sha256"
