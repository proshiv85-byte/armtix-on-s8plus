#!/usr/bin/env bash
# Build the minimal ARMtix initramfs for dream2lte.
# busybox-static (aarch64) + init script; rootfs lives on userdata (sda27).
set -euo pipefail
W="$(cd "$(dirname "$0")/.." && pwd)"
BUSYBOX="$W/work/initramfs/dl/ex/bin/busybox.static"
OUT="$W/work/initramfs/initramfs.cpio"

[ -f "$BUSYBOX" ] || { echo "busybox.static missing at $BUSYBOX" >&2; exit 1; }

BUILD="$W/work/initramfs/build"
rm -rf "$BUILD"
mkdir -p "$BUILD"/{bin,dev,proc,sys,newroot,etc}

cp "$BUSYBOX" "$BUILD/bin/busybox"
cd "$BUILD/bin"
for a in sh mount switch_root sleep cat echo mkdir mknod ls ln rm reboot poweroff dd grep wc seq; do
  ln -s busybox "$a"
done
cd "$BUILD"

cat > init <<'EOF'
#!/bin/busybox sh
# ARMtix initramfs for Samsung Galaxy S8+ (dream2lte)
export PATH=/bin

mount -t proc proc /proc 2>/dev/null
mount -t sysfs sysfs /sys 2>/dev/null
mount -t devtmpfs devtmpfs /dev 2>/dev/null

ROOT=""
ROOTFS=""
for p in $(cat /proc/cmdline); do
  case "$p" in
    # first occurrence wins: ours (CONFIG_CMDLINE) comes before S-Boot's
    root=*) [ -n "$ROOT" ] || ROOT="${p#root=}" ;;
    rootfstype=*) [ -n "$ROOTFS" ] || ROOTFS="${p#rootfstype=}" ;;
  esac
done
[ -n "$ROOT" ] || ROOT="/dev/sda27"
[ -n "$ROOTFS" ] || ROOTFS="ext4"

echo "=== ARMtix initramfs ==="
echo "root=$ROOT rootfstype=$ROOTFS"

mkdir -p /newroot
tries=0
mounted=0
while [ $tries -lt 200 ]; do
  if mount -t "$ROOTFS" "$ROOT" /newroot 2>/dev/null; then
    mounted=1
    break
  fi
  sleep 0.25
  tries=$((tries + 1))
done

if [ $mounted -eq 0 ]; then
  echo "FATAL: cannot mount $ROOT (tried ${tries}x); falling back to /dev/sda27"
  mount -t "$ROOTFS" /dev/sda27 /newroot 2>/dev/null || {
    echo "FATAL: no root filesystem found. Halting in 60s."
    sleep 60
    reboot -f
  }
fi

# /sbin/init on ARMtix is an ABSOLUTE symlink (-> /usr/bin/openrc-init); it
# only resolves after switch_root, so accept a symlink here too.
[ -x /newroot/sbin/init ] || [ -L /newroot/sbin/init ] || {
  echo "FATAL: /sbin/init missing on root filesystem. Halting in 60s."
  sleep 60
  reboot -f
}

echo "Root mounted, switching to OpenRC ..."
umount /proc /sys /dev 2>/dev/null
exec switch_root /newroot /sbin/init
EOF
chmod +x init

# plain newc cpio (same format as the Hades ramdisk)
find . -print0 | cpio --null -o --format=newc > "$OUT"
echo "initramfs: $OUT ($(stat -c%s "$OUT") bytes)"
