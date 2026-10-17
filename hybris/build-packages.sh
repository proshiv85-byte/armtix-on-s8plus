#!/bin/bash
# Build the ARMtix GPU/sway packages (run on ARMtix aarch64 as a normal user
# with sudo, or in an ARMtix chroot). Output: hybris/out/*.pkg.tar.*
#
#   hybris/build-packages.sh            # all
#   hybris/build-packages.sh wvkbd ...  # some
set -euo pipefail
H="$(cd "$(dirname "$0")" && pwd)"
OUT="$H/out"
mkdir -p "$OUT"
export MAKEFLAGS="${MAKEFLAGS:--j$(nproc)}"
# Order matters: libhybris-armtix depends on armtix-hybris-gsi
PKGS="${*:-armtix-hybris-gsi libhybris-armtix libudev-zero-armtix wlroots0.20-hybris wvkbd gl4es-armtix glfw-minecraft lan-mouse-armtix armtix-camera}"
for p in $PKGS; do
	echo "==> $p"
	cd "$H/pkg/$p"
	PKGDEST="$OUT" makepkg -sfc --noconfirm
	if [ "$p" = armtix-hybris-gsi ]; then
		sudo pacman -U --noconfirm --needed "$OUT"/armtix-hybris-gsi-*.pkg.tar.*
	fi
done
ls -la "$OUT"
