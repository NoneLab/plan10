#!/usr/bin/env python3
"""Static U2 image-layout check; this does not exercise QEMU Ring 3."""
import pathlib
import struct
import sys

if len(sys.argv) != 3:
    raise SystemExit("usage: user_initcode.py INITCODE.ELF INITCODE.BIN")

elf = pathlib.Path(sys.argv[1]).read_bytes()
raw = pathlib.Path(sys.argv[2]).read_bytes()
if elf[:6] != b"\x7fELF\x02\x01":
    raise SystemExit("FAIL: initcode ELF must be little-endian ELF64")

entry = struct.unpack_from("<Q", elf, 24)[0]
if entry != 0x1020:
    raise SystemExit(f"FAIL: native initcode _start is {entry:#x}, expected 0x1020")

if not 0 < len(raw) <= 4096-32:
    raise SystemExit(f"FAIL: user image is not contained in one page ({len(raw)} bytes)")
    
print(f"Plan 9/pc64 first user instruction: 0x{entry:x}, image={len(raw)+32} bytes: PASS")
print("This is NOT a QEMU Ring 3 or syscall execution test")