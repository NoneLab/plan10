#!/usr/bin/env python3
"""Read-only checks of the ACTUAL Plan 9/pc64 Multiboot entry object.

No test kernel or fake boot image is linked. QEMU remains unverified until
make kernel produces a genuine linked 9pc64.elf and make qemu runs it.
"""
import struct
import sys
from pathlib import Path


def verify(path):
    data = Path(path).read_bytes()
    if data[:4] != b"\x7fELF" or data[4] != 2 or data[5] != 1:
        raise ValueError("expected little-endian ELF64")
    # ELF64 section metadata and the standard null-terminated section names.
    eh = struct.unpack_from("<16sHHIQQQIHHHHHH", data)
    shoff, shentsize, shnum, names_idx = eh[6], eh[11], eh[12], eh[13]
    sections = [struct.unpack_from("<IIQQQQIIQQ", data, shoff + i * shentsize)
                for i in range(shnum)]
    nametab = sections[names_idx]
    names = data[nametab[4]:nametab[4] + nametab[5]]

    def name_at(idx):
        start = sections[idx][0]
        return names[start:names.index(b"\0", start)].decode()

    found = {name_at(i): entry for i, entry in enumerate(sections)}
    for name in (".multiboot", ".boot.text", ".boot.paging", ".boot.bss"):
        if name not in found:
            raise ValueError("missing mandatory boot section " + name)
    hdr = found[".multiboot"]
    magic, flags, checksum = struct.unpack_from("<III", data, hdr[4])
    if magic != 0x1BADB002 or (magic + flags + checksum) & 0xFFFFFFFF:
        raise ValueError("invalid Multiboot v1 header/checksum")
    pg = found[".boot.paging"]
    if pg[5] != 4 * 4096:
        raise ValueError("expected PML4, 2xPDPT, and 2MiB PD")
    for i in (0, 1, 255, 511):
        pde, = struct.unpack_from("<Q", data, pg[4] + 3 * 4096 + i * 8)
        if pde != i * (2 * 1024 * 1024) + 0x83:
            raise ValueError("incorrect identity/higher-half 2MiB mapping")
    if not 0 < hdr[4] < 8192:
        raise ValueError("Multiboot header is too large for GRUB/QEMU scan")
    print("Plan 9 pc64 Multiboot header & early PML4/PDPT/PD object: PASS")
    print("This is an object check, NOT a QEMU boot result")


if __name__ == "__main__":
    verify(sys.argv[1])
