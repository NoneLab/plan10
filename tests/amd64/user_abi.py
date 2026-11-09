#!/usr/bin/env python3
"""Read-only instruction-level verification; not a CPL3 QEMU execution test."""
import subprocess
import sys


def disassemble(path):
    return subprocess.check_output(["objdump", "-d", path], text=True)


def verify(user, kernel_entry, dispatch):
    u = disassemble(user)
    k = disassemble(kernel_entry)
    d = disassemble(dispatch)
    if "<p9_syscall5>:" not in u or "int    $0x40" not in u:
        raise SystemExit("user syscall trampoline is missing int $0x40")
    if "<p9_amd64_iretuser>:" not in k or "iretq" not in k:
        raise SystemExit("kernel CPL3 return code is missing iretq")
    if "<p9_amd64_syscall>:" not in d:
        raise SystemExit("native syscall dispatcher is missing")
    print("Plan 9/pc64 user ABI: PASS (native assembly and dispatcher objects)")
    print("This is NOT a QEMU Ring 3, syscall or initcode execution test")


if __name__ == "__main__":
    verify(*sys.argv[1:4])