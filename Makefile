# Plan 9 Fourth Edition -- native AMD64 / Clang build
#
# Entry point for GNU Make.  Machine-specific configuration and source
# selection live alongside the new AMD64 port in sys/src/9/pc64/Makefile.
#
# The original 386 mkfiles and their source tree are kept intact.

.DEFAULT_GOAL := all

CLANG          ?= clang
CLANGPP		   ?= clang++
LLD            ?= ld.lld
OBJCOPY        ?= objcopy
PYTHON         ?= python3

BUILD_DIR      ?= out/amd64
KERNEL_ELF     ?= $(BUILD_DIR)/9pc64.elf
KERNEL_MB      ?= $(BUILD_DIR)/9pc64.mb

# QEMU settings are independently overridable from the command line.
# This port is currently uniprocessor and uses ISA COM1 for early output.
QEMU           ?= qemu-system-x86_64
QEMU_MACHINE   ?= pc
QEMU_CPU       ?= qemu64
QEMU_MEMORY_MB ?= 1024
QEMU_SMP       ?= 1
QEMU_GDB_PORT  ?= 1234
QEMU_LOG_FLAGS ?= guest_errors,cpu_reset
QEMU_EXTRA_ARGS ?=

include sys/src/9/pc64/Makefile


.PHONY: all check check-tools help clean

all: kernel

help:
	@printf '%s\n' \
		'Plan 9 Fourth Edition / pc64 (GNU Make + Clang)' \
		'  make check          - ABI, /port objects, machine and entry tests' \
		'  make port-audit     - Clang syntax inventory, with logged exclusions' \
		'  make kernel-deps    - link current native sources and report undefined symbols'  \
		'  make kernel         - link actual Plan 9 pc64 kernel ELF' \
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
