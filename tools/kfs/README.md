# U3.5 — external KFS/Wren userspace boot

`mkbootfs.kfs` is a **host** C++17 tool, not a Plan 9 program. It produces the
original Plan 9 KFS/Wren disk format (not Android `mkbootfs`, FAT, qcow2 or a
new filesystem). `kfs_image.cpp` explicitly serializes the 32-bit on-disk
fields used by `sys/src/cmd/disk/kfs`. It creates a block-zero Wren header,
1KiB KFS blocks with eight-byte trailing tags, superblock at block 1, root
Dentry at block 2, 88-byte directory entries, and KFS free-list chains.

The original native `/boot/boot` and `/boot/kfs` remain in `bootlinks()`.
This breaks the bootstrap dependency cycle: the file server must be runnable
before its on-disk root can be mounted. `/amd64/init`, `/bin/rc` and
`/rc/lib/rcmain` are loaded from KFS instead of the kernel.

## Test on a host with Clang, Bison and QEMU

```sh
# Object flags change when the embed options change: clean is essential.
make clean
make -j$(nproc) kfs-bootfs-image kfs-bootfs-check \
    kernel kernel-verify INIT_EMBED=0 RC_EMBED=0
make kfs-bootfs-selftest

# Run against a disposable copy: KFS may update its metadata at runtime.
cp out/amd64/images/kfs-root.raw out/amd64/images/kfs-qemu-test.raw
make qemu INIT_EMBED=0 RC_EMBED=0 \
    QEMU_DISK_IMAGE=out/amd64/images/kfs-qemu-test.raw
```

The build creates `out/amd64/host/bin/mkbootfs.kfs` and a sparse 64MiB raw
image under `out/amd64/images/kfs-root.raw`. A `verify` pass checks actual
superblock tags, file contents, indirect block references and free/used block
accounting; `--file HOST:/GUEST` rereads and compares guest content against
host input bytes. The host C++ self-test checks a directory with 70 entries,
large files requiring indirect blocks, and corruption rejection.

Expected boot confirmation: `bootlinks()` reports *only* `boot` and `kfs`,
original `/boot/boot` mounts `#S/sdC0/fs`, `exec()` loads `/amd64/init` from
the mounted KFS, then init executes on-disk `/bin/rc` which reads
`/rc/lib/rcmain` and provides a `%` prompt.

Do **not** supply `-r` to `/boot/kfs`; that reformats the newly populated
disk. Do **not** overwrite an existing persistent disk with `--force`.
The Make target's `--force` is intended only for its generated output under
`out/amd64/images/`.

This stage does not yet relocate `/boot/kfs` itself and does not implement
Plan 9 C++ userspace support; that is separately planned for U3.6.
