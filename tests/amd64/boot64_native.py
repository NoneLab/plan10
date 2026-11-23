#!/usr/bin/env python3
"""Native original Plan 9 boot.c executable and 6.out identity check.

This is a link/format test, not an assertion of runtime boot success.
"""
import pathlib
import struct
import subprocess
import sys

from importlib.util import spec_from_file_location, module_from_spec

root = pathlib.Path(__file__).resolve().parents[2]
spec = spec_from_file_location("elf26out", root / "tools/pc64/elf26out.py")
conv = module_from_spec(spec)
spec.loader.exec_module(conv)

elf = pathlib.Path(sys.argv[1]).read_bytes()
image = pathlib.Path(sys.argv[2]).read_bytes()
repacked, meta = conv.make_6out(elf)
if repacked != image:
    raise SystemExit("FAIL: native boot ELF and 6.out payload differ")

symbols = subprocess.check_output(["nm", "-n", sys.argv[1]], text=True)
syms = {}
for line in symbols.splitlines():
    cols = line.split()
    if len(cols) == 3 and cols[1].upper() in ("T", "D", "B"):
        syms[cols[2]] = int(cols[0], 16)

for name in ("_start", "main", "boot", "p9_syscall5", "malloc", "auth_proxy"):
    if name not in syms:
        raise SystemExit(f"FAIL: required original boot symbol missing: {name}")

if syms["_start"] != meta[3] or not (0x1028 <= syms["boot"] < 0x1028 + meta[0]):
    raise SystemExit("FAIL: boot entry or original boot() symbol out of text range")

undef = subprocess.check_output(["nm", "-u", sys.argv[1]], text=True).strip()
if undef:
    raise SystemExit(f"FAIL: boot executable has undefined symbols:\n{undef}")

magic, text, data, bss, *rest = struct.unpack_from(">8IQ", image)
if magic != conv.S_MAGIC or not text or not data:
    raise SystemExit("FAIL: native S_MAGIC text/data image invalid")

print(f"Plan 9/pc64 original boot(): PASS (entry={meta[3]:#x}, "+
      f"boot={syms['boot']:#x}, image={len(image)} bytes)")
      
print("Original source linked without undefined symbols; QEMU runtime NOT tested")