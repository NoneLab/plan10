#!/usr/bin/env python3
"""Check native rc image and the generated parser are linked, not booted.

Not a QEMU runtime test. yyparse alone is insufficient: a stub parser
would allow the original interpreter to link but could not read commands.
"""
import pathlib
import subprocess
import sys
from importlib.util import module_from_spec, spec_from_file_location

root = pathlib.Path(__file__).resolve().parents[2]
spec = spec_from_file_location("elf26out", root / "tools/pc64/elf26out.py")
conv = module_from_spec(spec)
spec.loader.exec_module(conv)
elf, img = map(pathlib.Path, sys.argv[1:3])
image, meta = conv.make_6out(elf.read_bytes())

if image != img.read_bytes():
    sys.exit("FAIL: rc ELF and native 6.out contents differ")

syms = subprocess.check_output(["nm", "-n", str(elf)], text=True)
names = {v[-1] for line in syms.splitlines() if len(v := line.split()) == 3}
for name in ("_start", "main", "yyparse", "yylex", "Xsimple", "execfinit", "p9_syscall5"):
    if name not in names:
        sys.exit(f"FAIL: rc native image missing {name} (check the real yacc parser)")

print(f"Original Plan 9 rc native ELF/6.out and parser linkage: PASS ({len(image)} bytes)")
print("Note: this is not an interactive rc execution or QEMU test")
