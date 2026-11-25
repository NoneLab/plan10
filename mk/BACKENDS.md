# Plan 9 / pc64 build backends

The project uses Android.mk-style **declarations**, not the Android build
system. A component describes its inputs through `LOCAL_*` and selects one
`BUILD_*` include. GNU Make owns all targets and recipes.

## 1. Which builder should I use?

| Builder | Purpose | Final output |
| --- | --- | --- |
| `BUILD_OBJECTS` | Source or CRT object module | `.o` |
| `BUILD_STATIC_LIBRARY` | Archive for any target ABI | `.a` |
| `BUILD_KERNEL` | Plan 9 kernel | ELF64 + physical Multiboot image |
| `BUILD_BOOTSTRAP_EXECUTABLE` | `boot`, `kfs`, `init` | native `6.out` |
| `BUILD_USER_EXECUTABLE` | `rc`, regular userspace programs | native `6.out` |
| `BUILD_HOST_EXECUTABLE` | Host-only utilities | native host executable |

`BUILD_OBJECTS` and `BUILD_STATIC_LIBRARY` are mechanically shared, but their
**compiler flags, output paths, and dependency targets are not**. Each module
must compile for exactly one of `kernel`, `userspace` or `host` when it is
consumed by a typed backend.

## 2. Build pipeline

```text
LOCAL_* declaration
    |-- BUILD_OBJECTS -----------------------> .o (CRT included)
    |-- BUILD_STATIC_LIBRARY ----------------> .a
    |-- BUILD_KERNEL ------------------------> kernel.elf -> kernel.mb
    |-- BUILD_BOOTSTRAP_EXECUTABLE --+
    |                               +--------> program.elf -> program.6.out
    |-- BUILD_USER_EXECUTABLE -------+
    `-- BUILD_HOST_EXECUTABLE ---------------> host program
```

The two Plan 9 userspace backends share `mk/link-plan9-executable.mk`.
Conversion with `tools/pc64/elf26out.py` is mandatory; the ELF intermediate
is retained for debugging. `mk/dependencies.mk` validates target types and
resolves static archive dependencies in consumer-before-provider order.

## 3. Common CRT and system static libraries

Both Plan 9 executable backends use the default `crt0-amd64` object module
for `_start`, unless `LOCAL_CRT` explicitly names another userspace object
module. The common CRT passes argc/argv to `main()`, preserves legacy `argv0`,
and calls `exits(nil)`; it does not initialize TLS or C++ constructors.

**System runtime libraries use a distinct declaration**, just as Android's
make build distinguishes system libraries from application dependencies.
`LOCAL_SYSTEM_STATIC_LIBRARIES` is a *plan10-specific extension*; Android.mk
uses `LOCAL_SYSTEM_SHARED_LIBRARIES` instead.

```make
# mk/plan9.mk: default runtime archives
BOOTSTRAP_SYSTEM_STATIC_LIBRARIES ?= libc-plan9
USERSPACE_SYSTEM_STATIC_LIBRARIES ?= libc-plan9
```

| `LOCAL_SYSTEM_STATIC_LIBRARIES` | Behavior |
| --- | --- |
| `default` (after `CLEAR_VARS`) | Use the backend's system libraries |
| `libc-plan9` | Explicitly pin the original Plan 9 libc |
| `libc-experimental` | Replace the system runtime archive |
| Empty `:=` | Link no automatic system archives |

Example regular userspace executable:

```make
include $(CLEAR_VARS)
LOCAL_MODULE := hello
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/hello
LOCAL_SRC_FILES := hello.c
LOCAL_STATIC_LIBRARIES := libmath
LOCAL_SYSTEM_STATIC_LIBRARIES := libc-experimental
include $(BUILD_USER_EXECUTABLE)
```

The executable linker orders application archives first, followed by system
runtime archives. Both groups are registered through the same
`BUILD_STATIC_LIBRARY` backend and validated as **userspace** archives.
`LOCAL_LIBC` and the old per-libc CRT fallback are removed: the common CRT is
independent of the selected libc implementation. Selecting a different libc
archive **does not automatically switch include directories or recompile
libraries**: declare any ABI-specific compile flags and use distinct object
paths for different header/ABI profiles when adding the experimental libc.

### Migrated native applications

`boot`, `kfs`, and `init` pin `libc-plan9` in their bootstrap declarations.
`rc` uses the normal userspace default (`libc-plan9` unless changed). They
no longer link Plan 9 libc port, fmt, 9sys or amd64 runtime objects directly;
these are archived in `sys/src/libc/Plan9.mk` as `libc-plan9.a`.

KFS's custom `print()` and rc's floating-point format implementation remain
part of their program code. As the runtime is an archive, the linker only
extracts needed object members. Other dependencies (`libauth`, `libip`,
`libbio`, `libString`) remain direct objects during this stage and can be
archived separately later.

## 4. Module contracts

- `LOCAL_MODULE`: unique logical module name
- `LOCAL_TARGET`: `kernel`, `userspace`, or `host`
- `LOCAL_SRC_FILES` / `LOCAL_EXTRA_OBJECTS`: compiled sources or existing `.o`
- `LOCAL_OBJECT_LIBRARIES`: named registered object modules (including CRT)
- `LOCAL_STATIC_LIBRARIES`: named archive modules of the same target
- `LOCAL_CRT`: named userspace `BUILD_OBJECTS` module
- `LOCAL_SYSTEM_STATIC_LIBRARIES`: `default` (backend runtime), explicit
  system archives, or empty (disable system archives)
- `LOCAL_MODULE_PATH` / `LOCAL_MODULE_FILENAME`: output path override
- `LOCAL_BUILT_ELF` / `LOCAL_BUILT_MODULE`: outputs of the last executable
  declaration; save these before the next `CLEAR_VARS`

Providers must be included **before** consumers because this is a
parse-time module registry. For a precise list of accepted declarations,
consult the `mk/build-*.mk` file for that backend.

## 5. Automatic KFS root filesystem installation (U3.9)

An executable opts into the KFS root only when it declares an installation
image and a **guest directory**. Neither the target ELF nor its intermediate
build output is used as a guest pathname; the installed file has a normal
Plan 9 path without a `.6.out` suffix.

```make
# sys/src/cmd/rc/Plan9.mk
LOCAL_MODULE := rc-executable
LOCAL_MODULE_FILENAME := rc
LOCAL_INSTALL_IMAGE := kfs
LOCAL_INSTALL_PATH := /bin
include $(BUILD_USER_EXECUTABLE)
# Installs <output>/rc.6.out as /bin/rc (mode 0755).
```

Bootstrap `init` opts in at `/amd64/init`, while `/boot/boot` and `/boot/kfs`
remain kernel-embedded and do not opt in. The same registration mechanism
works with `BUILD_BOOTSTRAP_EXECUTABLE` and `BUILD_USER_EXECUTABLE`.

Data files use `BUILD_INSTALL_DATA` (no compilation or linking):

```make
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := rcmain-data
LOCAL_SRC_FILES := rcmain
LOCAL_INSTALL_IMAGE := kfs
LOCAL_INSTALL_PATH := /rc/lib
include $(BUILD_INSTALL_DATA)
```

Both executable and data declarations support `LOCAL_INSTALL_FILENAME`
(optional output filename override) and `LOCAL_INSTALL_MODE` (octal file
permissions, defaults to `0755` for executables and `0644` for data).

The last module include (`tools/kfs/Plan9.mk`) consumes the registered install
sources and builds a **clean** `$(BUILD_DIR)/rootfs` staging tree using
`tools/kfs/stage_rootfs.py`. This is a build-owned, disposable directory:
removed modules cannot leave stale files. The host `mkbootfs.kfs --root`
imports every regular file and directory recursively, preserving KFS mode
bits and checking its normal on-disk tags/free-list. Symlinks and special
host files are rejected; directories `/tmp`, `/adm/timezone`, and `/lib` are
created even if empty.

`kfs-rootfs-stage` is phony, so `make kfs-bootfs-image` deliberately rebuilds
the RAW image on **every invocation**. No separate generated manifest or
file-change scanner is required. Keep a separate writable image copy for QEMU;
do not use the build output as a persistent working disk.

```sh
make kfs-bootfs-image
make kfs-bootfs-check
make kfs-bootfs-selftest
```

The registry currently uses whitespace-separated Make path lists: do not use
spaces or colons in module output paths. Installation paths must be valid
absolute KFS names (each component at most 27 bytes, no `..`). The staging
script confines destructive cleanup to a directory below `$(BUILD_DIR)`.

## 6. Verification

```sh
make -f tests/mk-backends/Makefile check
make -f tests/mk-backends/Makefile check \
    USERSPACE_SYSTEM_STATIC_LIBRARIES=libc-experimental
make kfs-bootfs-selftest
```

The backend fixtures check system runtime selection, archive dependency
resolution, CRT selection and native `6.out` generation. They do **not** prove a complete Plan 9 kernel boot.
For end-to-end verification, rebuild and boot the native QEMU/KFS image.
