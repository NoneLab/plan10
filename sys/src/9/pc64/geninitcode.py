#!/usr/bin/env python3
"""Embed the original source-built amd64 initcode in the real Plan 9 kernel."""
import pathlib
import sys

source = pathlib.Path(sys.argv[1])
destination = pathlib.Path(sys.argv[2])
image = bytes(32) + source.read_bytes()

if len(image) > 4096:
    raise SystemExit(f"initcode too large: {len(image)} bytes (maximum 4096)")

destination.parent.mkdir(parents=True, exist_ok=True)

body = "\n".join(
    "    " + ", ".join(f"0x{byte:02x}" for byte in image[i:i+12]) + ","
    for i in range(0, len(image), 12)
)

destination.write_text(
    "/* Generated from the original port/initcode.c and native AMD64 entry/libc. */\n"
    "static unsigned char initcode_amd64[] = {\n"
    + body + "\n};\n"
)

print(f"Plan 9 AMD64 initcode: {len(image)} bytes (32-byte entry prefix)")