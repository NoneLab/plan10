#!/usr/bin/env python3
"""Build a clean KFS staging directory from the registered Plan9.mk modules."""

import argparse
import os
import shutil
from pathlib import Path


def guest_path(text: str) -> Path:
    # KFS Dentry names are 28 bytes including NUL. Never allow a host path
    # to escape the staging directory, even with an accidental module typo.
    if not text.startswith("/") or text == "/":
        raise ValueError(f"expected absolute guest file/directory path: {text}")
    parts = text[1:].split("/")
    if any(not part or part in (".", "..") or len(part.encode("utf-8")) >= 28 for part in parts):
        raise ValueError(f"invalid KFS path component: {text}")
    return Path(*parts)


def parse_file(arg: str):
    # Make passes HOST:/absolute/guest:MODE (no whitespace in paths).
    host, sep, rest = arg.partition(":")
    if not sep:
        raise ValueError(f"invalid --file mapping: {arg}")
    guest, sep, mode = rest.rpartition(":")
    if not sep or not mode or any(ch not in "01234567" for ch in mode):
        raise ValueError(f"invalid file mode/mapping: {arg}")
    value = int(mode, 8)
    if not (0 <= value <= 0o777):
        raise ValueError(f"invalid file mode: {mode}")
    return Path(host), guest_path(guest), value


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--file", action="append", default=[])
    parser.add_argument("--dir", action="append", default=[])
    args = parser.parse_args()

    build_dir = args.build_dir.resolve(strict=False)
    root = args.root.resolve(strict=False)
    # Never recursively erase an arbitrary user-selected directory.
    if root == build_dir or build_dir not in root.parents:
        parser.error("--root must be a child of --build-dir")
    if args.root.is_symlink():
        parser.error("staging --root must not be a symlink")

    files = [parse_file(item) for item in args.file]
    destinations = [dest for _, dest, _ in files]
    if len(destinations) != len(set(destinations)):
        parser.error("duplicate KFS installation path")

    temp = root.with_name(root.name + f".partial.{os.getpid()}")
    if temp.exists() or temp.is_symlink():
        parser.error(f"temporary staging directory already exists: {temp}")
    temp.parent.mkdir(parents=True, exist_ok=True)
    try:
        temp.mkdir()
        for directory in args.dir:
            (temp / guest_path(directory)).mkdir(parents=True, exist_ok=True)
        for source, target, mode in files:
            if source.is_symlink() or not source.is_file():
                raise ValueError(f"source must be a regular file: {source}")
            output = temp / target
            output.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(source, output)
            output.chmod(mode)
        if root.exists():
            if not root.is_dir():
                raise ValueError(f"staging path is not a directory: {root}")
            shutil.rmtree(root)
        temp.rename(root)
    except Exception:
        shutil.rmtree(temp, ignore_errors=True)
        raise
    print(f"KFS staging ready: {root} ({len(files)} files)")


if __name__ == "__main__":
    main()
