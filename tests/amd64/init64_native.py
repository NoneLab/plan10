#!/usr/bin/env python3
"""Validate the *original* native init.c binary, not QEMU execution.

The image must be an actual 6.out conversion of a fully linked ELF64 image,
with the historical init main(), namespace and libc dependencies resolved.
"""
import pathlib
import subprocess
import sys
from importlib.util import spec_from_file_location, module_from_spec

root = pathlib.Path(__file__).resolve().parents[2]
spec = spec_from_file_location("elf26out", root / "tools/pc64/elf26out.py")
conv = module_from_spec(spec)
spec.loader.exec_module(conv)
elfpath, imgpath = map(pathlib.Path, sys.argv[1:3])
image, meta = conv.make_6out(elfpath.read_bytes())
if image != imgpath.read_bytes():
    sys.exit("FAIL: original init ELF and generated 6.out do not match")
symbols = subprocess.check_output(["nm", "-n", str(elfpath)], text=True)
names = {s.split()[-1] for s in symbols.splitlines() if len(s.split()) == 3}
for name in ("_start", "main", "newns", "p9_syscall5", "fd2path"):
    if name not in names:
        sys.exit(f"FAIL: missing native init symbol: {name}")
undef = subprocess.check_output(["nm", "-u", str(elfpath)], text=True).strip()
if undef:
    sys.exit(f"FAIL: unresolved init ELF symbols:\n{undef}")
print(f"Original Plan 9/amd64 init: PASS (entry={meta[3]:#x}, image={len(image)} bytes)")
print("This tests linkage/format; /boot/init execution in QEMU is NOT tested")