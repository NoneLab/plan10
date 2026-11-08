#!/usr/bin/env python3
"""Reproduce ../port/mkerrstr without invoking the Plan 9 rc shell."""

import argparse
import re
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()

    pattern = re.compile(r"^extern\s+char\s+(\w+)\[\];\s*/\*\s*(.*?)\s*\*/")
    contents = ["/* Generated from port/error.h.  Do not edit. */"]

    for line in args.input.read_text().splitlines():
        match = pattern.match(line)
        if match:
            name, message = match.groups()
            contents.append(f'char {name}[] = "{message}";')

    if not contents[1:]:
        raise SystemExit(f"ERROR: no error string definitions in {args.input}")

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text("\n".join(contents) + "\n")


if __name__ == "__main__":
    main()
