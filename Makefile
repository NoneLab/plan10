# Plan 9 Fourth Edition -- native AMD64 / Clang build
#
# Single GNU Make entry point.
# Build plumbing lives under mk/; each component owns a source-local Plan9.mk.
# No recursive make or Linux Kbuild-style component Makefiles are required.
#
# The original 386 mkfiles and their source tree are kept intact.

.DEFAULT_GOAL := all

include mk/toolchain.mk
include mk/common.mk
include mk/plan9.mk

# Select a platform: this source tree currently supports the native pc64 port.
# Its Plan9.mk explicitly includes the other component declarations in
# dependency order (no filesystem traversal or implicit source discovery).
include sys/src/9/pc64/Plan9.mk


.PHONY: all check check-tools help clean

all: kernel kfs-bootfs-image

help:
	@printf '%s\n' \
		'Plan 9 Fourth Edition / pc64 (GNU Make + Clang)' \
		'  make check          - ABI, /port objects, machine and entry tests' \
		'  make port-audit     - Clang syntax inventory, with logged exclusions' \
		'  make kernel-deps    - link current native sources and report undefined symbols'  \
		'  make kernel         - link actual Plan 9 pc64 kernel ELF' \
		'  make boot64-native - build original /boot/boot as 6.out' \
		'  make kernel-verify  - validate ELF64, Multiboot, Plan 9 symbols' \
		'  make user-abi-check - verify native AMD64 Ring 3/syscall objects' \
		'  make qemu-preflight - validate qemu-system-x86_64 installation' \
		'  make qemu           - build genuine kernel and run QEMU' \
		'  make qemu-gdb       - QEMU stopped at entry with GDB :1234' \
		'  make clean          - remove generated out/amd64 files'

check: abi-check port-smoke machine-smoke machine-unit-tests boot-entry-check user-abi-check

check-tools:
	@$(CLANG) --version | head -n 1
	@$(LLD) --version | head -n 1

clean:
	rm -rf -- "$(BUILD_DIR)"
