#!/usr/bin/env python3
"""Repack a Samsung Exynos dream2lte boot.img (header v0).

Layout replicated from the stock/Hades image:
  [header 2048][kernel][ramdisk][dt.img]   (all page-aligned)

The DT blob MUST be a Samsung DTBH image (scripts/dtbtool_exynos output) and
its size MUST be in the header's dt_size field (offset 40). With dt_size=0
S-Boot loads no DTB at all and the kernel boots on whatever DTB is left in RAM
(e.g. TWRP's after a warm reboot).

Fixed addresses taken from the extracted Hades boot.img:
  kernel_addr 0x10008000, ramdisk_addr 0x11000000,
  second_addr 0x10f00000, tags_addr 0x10000100, page_size 2048
"""
import struct, sys, os

PAGE = 2048
KERNEL_ADDR = 0x10008000
RAMDISK_ADDR = 0x11000000
SECOND_ADDR = 0x10f00000
TAGS_ADDR = 0x10000100
NAME = 'armtix-dream2lte'  # 16 bytes max

def pad_to(d, size):
    return d + b'\x00' * (size - len(d))

def aligned(x, page=PAGE):
    return (x + page - 1) // page * page

def build(kernel_path, ramdisk_path, dtb_path, cmdline, out_path):
    kernel = open(kernel_path, 'rb').read()
    ramdisk = open(ramdisk_path, 'rb').read()
    dtb = open(dtb_path, 'rb').read() if dtb_path else b''
    assert not dtb or dtb[:4] == b'DTBH', \
        'dtb must be a DTBH image (dtbtool_exynos output), not a raw .dtb'
    dtb = pad_to(dtb, aligned(len(dtb)))
    assert len(kernel) > 0 and len(ramdisk) > 0, 'empty input'
    cmdline_b = cmdline.encode('utf-8')
    assert len(cmdline_b) < 512, 'cmdline too long'

    header = bytearray(PAGE)
    header[0:8] = b'ANDROID!'
    struct.pack_into('<8I', header, 8,
                     len(kernel), KERNEL_ADDR, len(ramdisk), RAMDISK_ADDR,
                     0, SECOND_ADDR, TAGS_ADDR, PAGE)
    struct.pack_into('<I', header, 40, len(dtb))  # Samsung dt_size
    # name @48 (16 bytes), cmdline @64 (512 bytes), id @576 (32 bytes, zeros)
    header[48:64] = pad_to(NAME.encode('utf-8'), 16)
    header[64:64 + len(cmdline_b)] = cmdline_b

    k_off = PAGE
    r_off = k_off + aligned(len(kernel))
    d_off = r_off + aligned(len(ramdisk))

    img = bytearray(d_off + len(dtb))
    img[0:PAGE] = header
    img[k_off:k_off + len(kernel)] = kernel
    img[r_off:r_off + len(ramdisk)] = ramdisk
    if dtb:
        img[d_off:d_off + len(dtb)] = dtb

    open(out_path, 'wb').write(bytes(img))
    print(f'kernel: {len(kernel)} bytes @ {k_off:#x}')
    print(f'ramdisk: {len(ramdisk)} bytes @ {r_off:#x}')
    print(f'dtb: {len(dtb)} bytes @ {d_off:#x}')
    print(f'total: {len(img)} bytes -> {out_path}')

if __name__ == '__main__':
    import argparse
    ap = argparse.ArgumentParser()
    ap.add_argument('--kernel', required=True)
    ap.add_argument('--ramdisk', required=True)
    ap.add_argument('--dtb', default=None)
    ap.add_argument('--cmdline', required=True)
    ap.add_argument('-o', '--output', required=True)
    a = ap.parse_args()
    build(a.kernel, a.ramdisk, a.dtb, a.cmdline, a.output)
