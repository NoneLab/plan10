#!/usr/bin/env python3
"""Read-only Clang syntax audit of the original Plan 9 /port sources.

This is deliberately *not* a kernel build.  An audit pass means only that
Clang accepted a translation unit, not that it links or runs on AMD64.
"""

import argparse
import json
import re
import subprocess
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--clang", default="clang")
    parser.add_argument("--output", default="out/amd64/audit")
    args = parser.parse_args()

    root = Path(__file__).resolve().parents[4]
    output = (root / args.output).resolve()
    output.mkdir(parents=True, exist_ok=True)

    flags = [
        f"--target=x86_64-unknown-none-elf",
        "-std=c11",
        "-ffreestanding",
        "-fno-builtin",
        "-fno-stack-protector",
        "-fno-pic",
        "-mno-red-zone",
        "-Wno-comment",
        "-Iamd64/include",
        "-Isys/include",
        "-Isys/src/9/pc64",
        "-Isys/src/9/pc",     # diagnostic fallback for legacy 386 I/O headers
        "-Isys/src/9/port",
        f"-I{args.output}/../generated",
        "-fsyntax-only",
    ]

    # debugalloc.c is an abandoned alternate implementation, not part of
    # the original pc/mkfile /port link list.  It references allocator
    # metadata (B2D, MAGIC_F, MAXPOOL) absent from the supplied source.
    # Never hide this by compiling an empty version; record the exclusion.
    excluded = {"debugalloc.c": "historical incomplete alternate pool allocator; excluded by pc/mkfile"}
    records = []
    for source in sorted((root / "sys/src/9/port").glob("*.c")):
        if source.name in excluded:
            records.append({
                "source": source.name,
                "syntax_ok": None,
                "excluded": True,
                "reason": excluded[source.name],
            })
            continue
        command = [args.clang, *flags, str(source.relative_to(root))]
        result = subprocess.run(command, cwd=root, capture_output=True, text=True)
        diagnostics = result.stdout + result.stderr
        (output / f"{source.stem}.log").write_text(diagnostics)
        errors = re.findall(r"^.*error: (.*)$", diagnostics, re.M)
        warnings = re.findall(r"^.*warning: (.*)$", diagnostics, re.M)
        records.append({
            "source": source.name,
            "syntax_ok": result.returncode == 0,
            "errors": len(errors),
            "warnings": len(warnings),
            "first_error": errors[0] if errors else None,
        })

    (output / "report.json").write_text(json.dumps(records, indent=2) + "\n")
    tested = [record for record in records if not record.get("excluded")]
    passed = sum(record["syntax_ok"] for record in tested)
    clean = sum(record["syntax_ok"] and not record["warnings"] for record in tested)

    print(f"Plan 9 AMD64 Clang syntax audit: {passed}/{len(tested)} active units accepted")
    print(f"  Excluded obsolete alternatives: {len(excluded)} (listed in report.json)")
    print(f"  Without diagnostics: {clean}/{len(tested)}")
    print(f"  Detailed report: {output / 'report.json'}")
    print("  NOTE: syntax acceptance is not kernel ABI, link, or boot validation")

    # A non-zero exit keeps the incomplete migration visible to CI.
    if passed != len(tested):
        raise SystemExit(1)


if __name__ == "__main__":
    main()
