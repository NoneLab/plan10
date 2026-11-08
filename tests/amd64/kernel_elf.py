#!/usr/bin/env python3
"""Check the native Plan 9 ELF64 and its QEMU Multiboot-v1 flat image.

QEMU's Multiboot-v1 loader on several supported versions refuses ELF64 input.
Instead, objcopy emits the physical PT_LOAD contents into 9pc64.mb; header
address fields instruct QEMU to load that exact image.  This verifies that the
flat binary is byte-for-byte the *same kernel*, not a secondary boot kernel.
These are static checks, not evidence of an actual QEMU runtime boot.
"""
from __future__ import annotations

import pathlib
import struct
import subprocess
import sys

MULTIBOOT_MAGIC = 0x1BADB002
KZERO = 0xFFFFFFFF80000000


def verify(path: pathlib.Path, multiboot_path: pathlib.Path) -> None:
    image = path.read_bytes()
    raw = multiboot_path.read_bytes()
    assert image[:4] == b"\x7fELF", "not an ELF file"
    assert image[4] == 2, "not an ELF64 image"
    assert image[5] == 1, "not little-endian"

    elf_type, machine = struct.unpack_from("<HH", image, 16)
    assert (elf_type, machine) == (2, 62), "expected x86_64 executable ELF"

    entry = struct.unpack_from("<Q", image, 24)[0]
    phoff = struct.unpack_from("<Q", image, 32)[0]
    phentsize, phnum = struct.unpack_from("<HH", image, 54)
    assert phentsize == 56 and 1 <= phnum <= 32, "invalid program headers"

    segments = []
    for n in range(phnum):
        ph = struct.unpack_from("<IIQQQQQQ", image, phoff + n * phentsize)
        ptype, flags, off, va, pa, filesz, memsz, align = ph
        if ptype == 1:
            assert off + filesz <= len(image), "truncated PT_LOAD"
            assert memsz >= filesz and memsz != 0, "invalid PT_LOAD size"
            assert (va - off) % align == 0, "bad PT_LOAD virtual alignment"
            assert (pa - off) % align == 0, "bad PT_LOAD physical alignment"
            segments.append((flags, off, va, pa, filesz, memsz))

    assert len(segments) == 3, "expected boot, text, and data PT_LOAD segments"
    _, _, boot_va, boot_pa, _, boot_mem = segments[0]
    assert boot_va == boot_pa == 0x100000, "boot entry not in physical low RAM"
    assert boot_pa <= entry < boot_pa + boot_mem, "entry not in boot segment"

    text_flags, _, text_va, text_pa, _, text_mem = segments[1]
    data_flags, _, data_va, data_pa, _, data_mem = segments[2]
    assert text_flags & 5 == 5 and text_flags & 2 == 0, "text not RX"
    assert data_flags & 2 and data_flags & 4, "data not RW"
    assert text_va == KZERO + text_pa, "higher-half text mapping mismatch"
    assert data_va == KZERO + data_pa, "higher-half data mapping mismatch"
    assert boot_pa + boot_mem < text_pa, "boot segment overlaps kernel"
    assert text_pa + text_mem <= data_pa, "kernel text/data overlap"
    assert data_pa + data_mem < (1 << 30), "outside initial direct map"

    header_offset = None
    for off in range(0, min(len(image), 8192) - 31, 4):
        magic, flags, checksum = struct.unpack_from("<III", image, off)
        if magic == MULTIBOOT_MAGIC and (magic + flags + checksum) & 0xFFFFFFFF == 0:
            header_offset = off
            break
    assert header_offset is not None, "missing Multiboot-v1 header in first 8KiB"

    # QEMU's 32-bit Multiboot loader needs explicit physical address fields
    # when supplied with an image originally linked as ELF64.
    header = struct.unpack_from("<8I", image, header_offset)
    magic, flags, checksum, header_pa, load_pa, load_end, bss_end, mb_entry = header
    assert flags & 0x10000, "Multiboot address fields are missing"
    assert not (flags & ~0x10003), "unexpected Multiboot flags"
    assert (magic + flags + checksum) & 0xFFFFFFFF == 0, "bad Multiboot checksum"
    assert header_pa == boot_pa + header_offset - segments[0][1], \
        "Multiboot header address doesn't match ELF physical address"
    assert load_pa == boot_pa == 0x100000, "incorrect physical load base"
    assert mb_entry == entry, "Multiboot entry differs from native kernel entry"

    expected_load_end = max(pa + fs for _, _, _, pa, fs, _ in segments)
    expected_bss_end = max(pa + mem for _, _, _, pa, _, mem in segments)
    assert load_end == expected_load_end, "Multiboot load end disagrees with ELF"
    assert bss_end == expected_bss_end, "Multiboot BSS end disagrees with ELF"
    assert load_end <= bss_end, "negative Multiboot zero-fill region"
    assert len(raw) == load_end - load_pa, "flat physical image size mismatch"
    assert struct.unpack_from("<8I", raw, 0) == header, \
        "flat image lost its Multiboot header at the physical load start"

    # Compare physical memory bytes, not only symbol names and section sizes.
    # This ensures the QEMU image contains precisely the linked Plan 9 kernel.
    for _, off, _, pa, filesz, _ in segments:
        start = pa - load_pa
        assert raw[start:start + filesz] == image[off:off + filesz], \
            f"PT_LOAD bytes differ in flat image at 0x{pa:x}"

    symbols = subprocess.check_output(["nm", "--defined-only", str(path)], text=True)
    names = {line.split()[-1] for line in symbols.splitlines() if line.strip()}
    mandatory = {"main", "p9_multiboot_header", "p9_amd64_multiboot_entry",
                 "p9_amd64_kentry", "p9_amd64_trap", "mmuswitch",
                 "schedinit", "__kernel_end"}
    assert mandatory <= names, f"missing Plan 9 kernel symbols: {mandatory - names}"
    undefined = subprocess.check_output(["nm", "-u", str(path)], text=True)
    assert not undefined.strip(), f"undefined symbols:\n{undefined}"
    print(f"Plan 9/pc64 ELF image: PASS ({path}, entry=0x{entry:x})")
    print(f"QEMU Multiboot-v1 flat image: PASS ({multiboot_path}, {len(raw):,} bytes)")
    print("Verified: PT_LOAD byte identity, physical load/BSS spans, Multiboot addresses")
    print("Note: this is not a QEMU runtime/boot test")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        raise SystemExit("usage: kernel_elf.py <kernel.elf> <kernel.mb>")
    verify(pathlib.Path(sys.argv[1]), pathlib.Path(sys.argv[2]))
