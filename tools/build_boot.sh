#!/usr/bin/env bash
# Build boot-armtix.img: kernel Image + dtbs -> DTBH dt.img -> boot.img.
#
#   tools/build_boot.sh            # build kernel + dtb, repack
#   tools/build_boot.sh --no-kernel  # only rebuild dtb + repack (DTS-only change)
#
# The DT MUST be packed as a Samsung DTBH image with dt_size set in the boot
# header (repack_boot.py enforces this). A raw .dtb with dt_size=0 is silently
# ignored by S-Boot and the kernel boots on a stale DTB left in RAM.
set -euo pipefail
W="$(cd "$(dirname "$0")/.." && pwd)"
K="$W/work/kernel/src"
MK=(make -C "$K" ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- HOSTCFLAGS="-fcommon" KCFLAGS="-Wno-error")
DTB="$K/arch/arm64/boot/dts/exynos/exynos8895-dream2lte_eur_open_10.dtb"
DTIMG="$K/arch/arm64/boot/dtb.img"

if [ "${1:-}" != "--no-kernel" ]; then
	"${MK[@]}" -j"$(nproc)" Image
	cp "$K/arch/arm64/boot/Image" "$W/work/deploy/Image"
fi
"${MK[@]}" dtbs
"$K/scripts/dtbtool_exynos/dtbtool" --pagesize 2048 --platform 0x50a6 \
	--subtype 0x217584da -o "$DTIMG" "$DTB"

python3 "$W/tools/repack_boot.py" \
	--kernel "$W/work/deploy/Image" \
	--ramdisk "$W/work/initramfs/initramfs.cpio" \
	--dtb "$DTIMG" \
	--cmdline "androidboot.selinux=permissive earlycon=exynos4210,mmio32,0x14000000" \
	-o "$W/work/deploy/boot-armtix.img"
sha256sum "$W/work/deploy/boot-armtix.img"
