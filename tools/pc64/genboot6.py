#!/usr/bin/env python3
"""Embed an *existing validated* Plan 9 AMD64 6.out file into the root device."""
import argparse
import pathlib
import struct
import sys

MAGIC = 0x8000 | (4 * 26 * 26 + 7)
HEADER = struct.Struct('>8IQ')


def validate(image):
    if len(image) < HEADER.size:
        raise ValueError('truncated 6.out executable')
    magic, text, data, bss, syms, entrylo, spsz, pcsz, entry = HEADER.unpack_from(image)
    if magic != MAGIC:
        raise ValueError('not a native Plan 9 S_MAGIC 6.out file')
    if len(image) != HEADER.size + text + data:
        raise ValueError(f'file length {len(image)} does not match header text+data {HEADER.size + text + data}')
    if not text or not (0x1028 <= entry < 0x1028 + text):
        raise ValueError('missing text or invalid entry address')
    if entry != entrylo:
        raise ValueError('low entry word mismatches extended entry')
    if HEADER.size + text + data > 64 * 1024 * 1024:
        raise ValueError('early-boot image unexpectedly large')
    return entry, text, data, bss


def emit_header(path, image, symbol="p9_boot6_image"):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open('w') as fp:
        fp.write('/* Generated only from a validated native Plan 9 AMD64 6.out. */\n')
        fp.write(f'static uchar {symbol}[] = {{\n')
        for offset in range(0, len(image), 16):
            fp.write('    ' + ', '.join(f'0x{x:02x}' for x in image[offset:offset+16]) + ',\n')
        fp.write('};\n')


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('image', type=pathlib.Path)
    ap.add_argument('header', type=pathlib.Path, nargs='?')
    ap.add_argument('--symbol', default='p9_boot6_image',
                    choices=['p9_boot6_image', 'p9_kfs6_image', 'p9_init6_image', 'p9_rc6_image'],
                    help='C array symbol for the generated header')
    args = ap.parse_args()
    image = args.image.read_bytes()
    try:
        e, t, d, b = validate(image)
    except ValueError as ex:
        sys.exit(f'INVALID boot image: {ex}')
    if args.header:
        emit_header(args.header, image, args.symbol)
    print(f'Valid AMD64 S_MAGIC boot image: entry={e:#x} text={t} data={d} bss={b}')


if __name__ == '__main__':
    main()
