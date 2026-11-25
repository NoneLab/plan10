#!/usr/bin/env python3
"""Host regression test for KFS --root import and clean GNU Make staging."""

import subprocess
import sys
import tempfile
from pathlib import Path


def run(*args, success=True):
    result = subprocess.run([str(arg) for arg in args], text=True,
                            capture_output=True, check=False)
    if (result.returncode == 0) != success:
        raise AssertionError(f"unexpected exit {result.returncode}: {args}\n"
                             f"stdout: {result.stdout}\nstderr: {result.stderr}")
    return result.stdout


def main():
    if len(sys.argv) != 2:
        raise SystemExit("usage: rootfs_stage.py PATH_TO_MKBOOTFS")
    builder = Path(sys.argv[1]).resolve()
    stage = Path("tools/kfs/stage_rootfs.py").resolve()
    with tempfile.TemporaryDirectory(prefix="plan10-kfs-stage-") as directory:
        top = Path(directory)
        source = top / "source"
        source.mkdir()
        executable = source / "hello.6.out"
        executable.write_bytes(b"example plan9 executable\x00\n")
        data = source / "rcmain"
        data.write_bytes(b"echo hello from disk\n")
        root = top / "build" / "rootfs"
        image = top / "filesystem.raw"
        args = [sys.executable, stage, "--build-dir", root.parent,
                "--root", root, "--dir", "/tmp", "--dir", "/adm/timezone",
                "--file", f"{executable}:/bin/hello:0755",
                "--file", f"{data}:/rc/lib/rcmain:0644"]
        run(*args)
        assert (root / "tmp").is_dir()
        assert (root / "adm/timezone").is_dir()
        assert (root / "bin/hello").stat().st_mode & 0o777 == 0o755
        assert (root / "rc/lib/rcmain").stat().st_mode & 0o777 == 0o644

        run(builder, "create", "--root", root, "--output", image,
            "--size", "8M", "--force")
        listing = run(builder, "verify", "--image", image,
                      "--file", f"{executable}:/bin/hello",
                      "--file", f"{data}:/rc/lib/rcmain", "--list")
        assert "/bin/hello" in listing and "/rc/lib/rcmain" in listing
        assert "/adm/timezone" in listing
        assert any(line.startswith("- 755") and "/bin/hello" in line
                   for line in listing.splitlines())

        # A removed module must disappear from a rebuilt staging tree.
        run(sys.executable, stage, "--build-dir", root.parent, "--root", root,
            "--dir", "/tmp", "--file", f"{data}:/rc/lib/rcmain:0644")
        assert not (root / "bin/hello").exists()
        run(builder, "create", "--root", root, "--output", image,
            "--size", "8M", "--force")
        listing = run(builder, "verify", "--image", image, "--list")
        assert "/bin/hello" not in listing and "/rc/lib/rcmain" in listing

        # Neither import path may follow symlinks outside the staging root.
        (root / "escape").symlink_to(data)
        run(builder, "create", "--root", root, "--output", image,
            "--size", "8M", "--force", success=False)
        run(sys.executable, stage, "--build-dir", root.parent, "--root", top,
            "--file", f"{data}:/bin/leak:0755", success=False)
        run(sys.executable, stage, "--build-dir", root.parent, "--root", root,
            "--file", f"{data}:/bin/../leak:0755", success=False)

    print("PASS: recursive KFS import, modes, empty directories, stale cleanup and path safety")


if __name__ == "__main__":
    main()
