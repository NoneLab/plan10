#!/usr/bin/env python3
"""Embed an unchanged Plan 9 boot-time data file as a C byte array.

Unlike genboot6.py, this accepts ordinary text/data, not an executable.
"""
import argparse
from pathlib import Path

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--symbol", required=True)
parser.add_argument("input", type=Path)
parser.add_argument("output", type=Path)
args = parser.parse_args()

if not args.symbol.isidentifier():
    parser.error("symbol must be a C identifier")

data = args.input.read_bytes()
lines = [f"/* Generated from {args.input}; do not edit. */",
         f"static unsigned char {args.symbol}[] = {{"]

for offset in range(0, len(data), 12):
    lines.append("    " + ", ".join(f"0x{b:02x}" for b in data[offset:offset + 12]) + ",")

lines += ["    0x00, /* terminal NUL is not part of the embedded file */", "};", ""]
args.output.parent.mkdir(parents=True, exist_ok=True)
args.output.write_text("\n".join(lines))