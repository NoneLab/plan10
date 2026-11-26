# Native AMD64 Plan 9 build declaration.
# Loaded by the repository-root Makefile after mk/plan9.mk.
# The pc64 platform defines its ABI and kernel link graph; subcomponents
# declare their own objects and rules in source-local Plan9.mk files.

# pc64 output names (user overridable)
BUILD_DIR      ?= out/amd64
KERNEL_ELF     ?= $(BUILD_DIR)/9pc64.elf
KERNEL_MB      ?= $(BUILD_DIR)/9pc64.mb

# Native pc64 compiler and ABI options

TARGET_TRIPLE := x86_64-unknown-none-elf

ARCH_INCLUDES := \
    -Iamd64/include \
    -Isys/include \
    -Isys/src/9/pc64 \
    -I$(BUILD_DIR)/generated \
    -Isys/src/9/port \
    -Isys/src/libc/9syscall

ARCH_CFLAGS := \
    --target=$(TARGET_TRIPLE) \
    -std=gnu11 \
	-nostdlibinc \
    -O1 \
    -ffreestanding \
    -fno-builtin \
    -fno-stack-protector \
    -fno-omit-frame-pointer \
    -fno-pic \
    -fno-plt \
    -mno-red-zone \
    -mcmodel=kernel \
    -fno-asynchronous-unwind-tables \
    -MMD \
    -MP \
    -Wall \
    -Wextra \
    -Wno-comment \
    -Wno-unknown-pragmas \
    -Werror=implicit-function-declaration \
    -Werror=incompatible-pointer-types

ARCH_CPPFLAGS := $(ARCH_INCLUDES)

# Native boot and userspace libraries share one AMD64 freestanding ABI.
BOOT64_USER_CFLAGS := --target=$(TARGET_TRIPLE) -std=gnu11 -O1 -g -MMD -MP -nostdlibinc \
    -ffreestanding -fno-builtin -fno-stack-protector -fno-pic \
    -fno-plt -fno-asynchronous-unwind-tables -mno-red-zone \
    -ffunction-sections -fdata-sections -DP9_NATIVE_BOOT64 \
    -Iamd64/include -Isys/include -Isys/src/9/boot \
    -Isys/src/libc/9syscall -Isys/src/libc/amd64


# Embedded original /boot/boot image (optional).
BOOT64_IMAGE ?= $(BUILD_DIR)/boot64/boot.6.out
ifneq ($(strip $(BOOT64_IMAGE)),)
BOOT_IMAGE_DEFINES += -DP9_HAVE_BOOT6_IMAGE
BOOT_IMAGE_HEADERS += $(BUILD_DIR)/generated/boot6_amd64.h
$(BUILD_DIR)/generated/boot6_amd64.h: $(BOOT64_IMAGE) tools/pc64/genboot6.py
	@mkdir -p "$(dir $@)"
	$(PYTHON) tools/pc64/genboot6.py "$(BOOT64_IMAGE)" "$@"
endif

# U3.3: embed a genuine, linked KFS user server alongside /boot/boot.
# This makes /boot/kfs executable available; it does NOT provide #S storage.
KFS_IMAGE ?= $(BUILD_DIR)/kfs/kfs.6.out
KFS_EMBED ?= 1
ifeq ($(KFS_EMBED),1)
BOOT_IMAGE_DEFINES += -DP9_HAVE_KFS_IMAGE
BOOT_IMAGE_HEADERS += $(BUILD_DIR)/generated/kfs6_amd64.h
$(BUILD_DIR)/generated/kfs6_amd64.h: $(KFS_IMAGE) tools/pc64/genboot6.py
	@mkdir -p "$(dir $@)"
	$(PYTHON) tools/pc64/genboot6.py --symbol p9_kfs6_image "$(KFS_IMAGE)" "$@"
else ifneq ($(KFS_EMBED),0)
$(error KFS_EMBED must be 0 or 1)
endif

INIT_IMAGE ?= $(BUILD_DIR)/init/init.6.out
INIT_EMBED ?= 0
ifeq ($(INIT_EMBED),1)

BOOT_IMAGE_DEFINES += -DP9_HAVE_INIT_IMAGE
BOOT64_USER_CFLAGS += -DP9_BOOT_INIT_EMBED
BOOT_IMAGE_HEADERS += $(BUILD_DIR)/generated/init6_amd64.h
$(BUILD_DIR)/generated/init6_amd64.h: $(INIT_IMAGE) tools/pc64/genboot6.py
	@mkdir -p "$(dir $@)"
	$(PYTHON) tools/pc64/genboot6.py --symbol p9_init6_image "$(INIT_IMAGE)" "$@"

else ifneq ($(INIT_EMBED), 0)
$(error INIT_EMBED must be 0 or 1)
endif

RC_IMAGE ?= $(BUILD_DIR)/rc/rc.6.out
RC_EMBED ?= 0
ifeq ($(RC_EMBED),1)

BOOT64_USER_CFLAGS += -DP9_BOOT_RC_EMBED
BOOT_IMAGE_DEFINES += -DP9_HAVE_RC_IMAGE -DP9_HAVE_RCMAIN_DATA
BOOT_IMAGE_HEADERS += $(BUILD_DIR)/generated/rc6_amd64.h $(BUILD_DIR)/generated/rcmain_data.h

$(BUILD_DIR)/generated/rc6_amd64.h: $(RC_IMAGE) tools/pc64/genboot6.py
	@mkdir -p "$(dir $@)"
	$(PYTHON) tools/pc64/genboot6.py --symbol p9_rc6_image "$(RC_IMAGE)" "$@"

$(BUILD_DIR)/generated/rcmain_data.h: rc/lib/rcmain tools/pc64/genbootdata.py
	@mkdir -p "$(dir $@)"
	$(PYTHON) tools/pc64/genbootdata.py --symbol p9_rcmain_data "$<" "$@"

else ifneq ($(RC_EMBED),0)
$(error RC_EMBED must be 0 or 1)
endif

BOOT_IMAGE_OBJ := $(BUILD_DIR)/obj/pc64/bootimage-embedded.o
$(BOOT_IMAGE_OBJ): sys/src/9/pc64/bootimage.c $(BOOT_IMAGE_HEADERS)
	@mkdir -p "$(dir $@)"
	$(CLANG) $(ARCH_CFLAGS) $(ARCH_CPPFLAGS) $(BOOT_IMAGE_DEFINES) -c "$<" -o "$@"

.PHONY: boot64-image-check
boot64-image-check:
ifeq ($(strip $(BOOT64_IMAGE)),)
	@echo "ERROR: BOOT64_IMAGE must be an authentic, linked Plan 9 AMD64 6.out" >&2; exit 1
else
	$(PYTHON) tools/pc64/genboot6.py "$(BOOT64_IMAGE)"
endif

# AMD64 machine object declarations and boot-image embedding.
LOCAL_PATH := $(call my-dir)

# pc64-kernel-c
include $(CLEAR_VARS)
LOCAL_MODULE := pc64-kernel-c
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/pc64
LOCAL_SRC_FILES := \
    dat.c config.c main.c console.c \
    clock.c portglobals.c bootmem.c mmu.c \
    mmuproc.c segments.c trap64.c syscall64.c note64.c \
    userentry.c userinit.c exec64.c traputil.c \
    procmach.c power.c errstr.c kentry.c \
    atomics.c sdide.c
LOCAL_CFLAGS := $(ARCH_CFLAGS) $(ARCH_CPPFLAGS)
LOCAL_ASFLAGS := --target=$(TARGET_TRIPLE) -mno-red-zone
include $(BUILD_OBJECTS)
MACHINE_C_OBJS := $(LOCAL_BUILT_OBJECTS) $(BOOT_IMAGE_OBJ)

# pc64-kernel-asm
include $(CLEAR_VARS)
LOCAL_MODULE := pc64-kernel-asm
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/pc64
LOCAL_SRC_FILES := \
    lowlevel.S trapvec.S processentry.S useriret.S \
    entry.S atomic.S
LOCAL_CFLAGS := $(ARCH_CFLAGS) $(ARCH_CPPFLAGS)
LOCAL_ASFLAGS := --target=$(TARGET_TRIPLE) -mno-red-zone
include $(BUILD_OBJECTS)
MACHINE_ASM_OBJS := $(LOCAL_BUILT_OBJECTS)


KERNEL_LDSCRIPT := sys/src/9/pc64/kernel.ld

# Generated native kernel headers
# Plan 9's rc-based generator produces definitions consumed by proc.c.
# This GNU Make version keeps generated source out of the original tree.
$(BUILD_DIR)/generated/errstr.h: sys/src/9/port/error.h sys/src/9/pc64/generrstr.py
	$(PYTHON) sys/src/9/pc64/generrstr.py "$<" "$@"

# ---------------------------------------------------------------------------
# AMD64 syscall dispatch table
#
# Keep the original Plan 9 syscall table unchanged.
# Generate an AMD64-specific copy using C11 syntax.
# ---------------------------------------------------------------------------

SYSCALL_TABLE_SRC := sys/src/9/port/systab.h

SYSCALL_TABLE_HDR := \
    $(BUILD_DIR)/generated/systab_amd64.h

SYSCALL_NUMBERS_HDR := \
    sys/src/libc/9syscall/sys.h

$(SYSCALL_TABLE_HDR): \
    $(SYSCALL_TABLE_SRC) \
    $(SYSCALL_NUMBERS_HDR)
	@mkdir -p "$(dir $@)"

	sed -E \
	    -e 's@^#include "/sys/src/libc/9syscall/sys.h"@#include "sys.h"@' \
	    -e 's/^([[:space:]]*\[[^]]+\])[[:space:]]+/\1 = /' \
	    "$<" > "$@"


# ---------------------------------------------------------------------------
# Kernel syscall dispatcher dependencies
# ---------------------------------------------------------------------------

$(BUILD_DIR)/obj/pc64/syscall64.o: \
    $(SYSCALL_TABLE_HDR)

include sys/src/lib10c/Plan9.mk

# Source-local modules share this single dependency graph.
# Include order matters: the machine ABI is set before user libraries.
include sys/src/9/port/Plan9.mk
include sys/src/libc/amd64/Plan9.mk
include sys/src/9/pc64/init/Plan9.mk

# Machine-local objects and relocatable machine check
MACHINE_OBJS := $(MACHINE_C_OBJS) $(MACHINE_ASM_OBJS)
MACHINE_RELOC := $(BUILD_DIR)/obj/pc64-machine.o

$(MACHINE_RELOC): $(MACHINE_OBJS)
	@mkdir -p "$(dir $@)"
	$(LLD) -m elf_x86_64 -r $(MACHINE_OBJS) -o "$@"

.PHONY: machine-smoke machine-unit-tests boot-entry-check

machine-smoke: $(MACHINE_RELOC)
	@echo "Plan 9 AMD64 machine-layer relocatable link: PASS"

# Host tests and original Plan 9 libraries.
include tests/amd64/Plan9.mk
include sys/src/libc/Plan9.mk
include sys/src/libauth/Plan9.mk
include sys/src/libip/Plan9.mk
include sys/src/libbio/Plan9.mk

# Original native boot executable and 6.out are managed beside boot.c.
include sys/src/9/boot/Plan9.mk

# U3.3: original user-level KFS; do not silently substitute a fake server.
include sys/src/cmd/disk/kfs/Plan9.mk

include sys/src/cmd/Plan9.mk
include sys/src/cmd/rc/Plan9.mk
include sys/src/cmd/newhello/Plan9.mk
include rc/lib/Plan9.mk
include tools/kfs/Plan9.mk

# pc64 kernel ELF and physical Multiboot image
# ---------------------------------------------------------------------------
# Genuine Plan 9 kernel image / QEMU
#
# QEMU's Multiboot-v1 loader does not universally accept ELF64 kernels.
# Link one native ELF64 kernel for GDB, then export its exact PT_LOAD contents
# as a flat physical Multiboot image with explicit load addresses. The
# 32-bit -> long-mode transition lives in pc64/entry.S; it is NOT a separate
# kernel.  A linked image must contain the actual Plan 9 main(), original
# /port subsystems, and the selected Plan 9 devices.  Missing implementations
# MUST cause a real compile/link failure rather than generate a fake image.
# ---------------------------------------------------------------------------

# Report a deterministic list of missing platform/driver/libc symbols.
# This does not count as a successful kernel link, but makes porting
# dependencies actionable and records the exact point of interruption.
.PHONY: kernel-deps
kernel-deps: $(MACHINE_OBJS) $(PORT_KERNEL_OBJS) $(KERNEL_LIBC_OBJS) $(KERNEL_9SYS_OBJS) $(KERNEL_FMT_OBJS)
	@mkdir -p "$(BUILD_DIR)/audit"
	@$(LLD) -m elf_x86_64 -r $(MACHINE_OBJS) $(PORT_KERNEL_OBJS) $(KERNEL_LIBC_OBJS) $(KERNEL_9SYS_OBJS) $(KERNEL_FMT_OBJS) \
		-o "$(BUILD_DIR)/audit/combined.o"
	@nm -u "$(BUILD_DIR)/audit/combined.o" \
		| sed -n 's/^ *U //p' \
		| sort -u \
		| sed -e '/^__kernel_end$$/d' -e '/^end$$/d' -e '/^etext$$/d' \
		> "$(BUILD_DIR)/audit/unresolved.txt"
	@echo "Unresolved Plan 9 kernel symbols (excluding linker-script symbols):"
	@wc -l < "$(BUILD_DIR)/audit/unresolved.txt"
	@echo "List: $(BUILD_DIR)/audit/unresolved.txt"

# Resolve the actual Plan 9 main(), original /port kernel and selected
# device implementations.  Never generate a substitute test kernel.
# The kernel is an ELF64 binary, not a 6.out userspace executable.
include $(CLEAR_VARS)
LOCAL_MODULE := pc64-kernel
LOCAL_TARGET := kernel
LOCAL_KERNEL_ELF := $(KERNEL_ELF)
LOCAL_KERNEL_MB := $(KERNEL_MB)
LOCAL_KERNEL_LDSCRIPT := $(KERNEL_LDSCRIPT)
LOCAL_KERNEL_OBJECTS := $(MACHINE_OBJS) $(PORT_KERNEL_OBJS) \
    $(KERNEL_LIBC_OBJS) $(KERNEL_9SYS_OBJS) $(KERNEL_FMT_OBJS)
include $(BUILD_KERNEL)

# Linked image layout and original Plan 9 kernel symbol validation.
.PHONY: kernel-verify
kernel-verify: $(KERNEL_ELF) $(KERNEL_MB)
	$(PYTHON) tests/amd64/kernel_elf.py "$(KERNEL_ELF)" "$(KERNEL_MB)"

.PHONY: kernel qemu qemu-gdb qemu-preflight

kernel: $(KERNEL_ELF) $(KERNEL_MB)
	@echo "Native Plan 9/pc64 kernel linked: $(KERNEL_ELF)"
	@echo "QEMU Multiboot physical image: $(KERNEL_MB)"

# QEMU execution, logging and GDB
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
QEMU_DISK_IMAGE ?=
ifneq ($(strip $(QEMU_DISK_IMAGE)),)
QEMU_DISK_ARGS := -drive "file=$(QEMU_DISK_IMAGE),format=raw,if=ide,index=0,media=disk"
else
QEMU_DISK_ARGS := -drive "file=$(BUILD_DIR)/images/kfs-root.raw,format=raw,if=ide,index=0,media=disk"
endif

qemu-preflight:
	@command -v "$(QEMU)" >/dev/null || { \
		printf '%s\n' 'ERROR: qemu-system-x86_64 not found in PATH.' \
			'Install QEMU (for Debian/Ubuntu: sudo apt install qemu-system-x86).' >&2; \
		exit 1; \
	}
	@if [ -n "$(QEMU_DISK_IMAGE)" ] && [ ! -f "$(QEMU_DISK_IMAGE)" ]; then \
		echo "ERROR: QEMU_DISK_IMAGE does not exist: $(QEMU_DISK_IMAGE)" >&2; exit 1; \
	fi
	@$(QEMU) --version | head -n 1

# ISA COM1 (0x3f8) is multiplexed with the QEMU monitor on the terminal.
# Ctrl-A X exits; Ctrl-A C switches to the monitor. The chardev saves an
# independent serial.log for a bug report, even when QEMU stays running.
# QEMU diagnostics (guest errors and CPU resets) go to qemu.log.
qemu: kernel-verify qemu-preflight
	@mkdir -p "$(BUILD_DIR)"
	@printf '%s\n' 'Booting the native Plan 9/pc64 kernel (Ctrl-A X to exit).' \
		"Serial log: $(BUILD_DIR)/serial.log" \
		"QEMU log:   $(BUILD_DIR)/qemu.log"
	$(QEMU) \
		-machine "$(QEMU_MACHINE)" \
		-cpu "$(QEMU_CPU)" \
		-accel tcg \
		-m "$(QEMU_MEMORY_MB)" \
		-smp "$(QEMU_SMP)" \
		-display none \
		-no-reboot \
		-no-shutdown \
		-chardev "stdio,id=p9serial,mux=on,signal=off,logfile=$(BUILD_DIR)/serial.log,logappend=off" \
		-serial chardev:p9serial \
		-mon chardev=p9serial,mode=readline \
		-d "$(QEMU_LOG_FLAGS)" \
		-D "$(BUILD_DIR)/qemu.log" \
		-kernel "$(KERNEL_MB)" \
		$(QEMU_DISK_ARGS) $(QEMU_EXTRA_ARGS)

# With the target-specific variable, all QEMU options stay in one recipe.
qemu-gdb: QEMU_EXTRA_ARGS += -S -gdb tcp::$(QEMU_GDB_PORT)
qemu-gdb: qemu
