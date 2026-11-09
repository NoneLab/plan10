# GNU Make rules for the in-progress native AMD64 Plan 9 kernel port.
#
# All paths are relative to the Plan 9 tree root.  Unlike the original
# 386 mkfile, these rules use Clang's native ELF object format.


# ---------------------------------------------------------------------------
# Compiler and ABI
# ---------------------------------------------------------------------------

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
    -std=c11 \
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


# ---------------------------------------------------------------------------
# Machine-independent source smoke builds
#
# These are genuine Plan 9 /port source files, but they do not yet constitute
# an executable kernel.  More translation units will join this list as their
# anonymous embeddings and pointer conversions are fixed.
# ---------------------------------------------------------------------------

# The 28 mandatory original /port units are also compiled by check, not
# merely accepted by a syntax-only audit.  Additional port/device sources
# are covered as independent Clang smoke-build targets.
PORT_EXTRA_NAMES := \
    devdup \
    devkbin \
    devkbmap \
    devkprof \
    devroot \
    devssl \
    devtls \
    fpi \
    fpimem \
    initcode \
    mul64fract \
    syscallfmt \
    thwack \
    unthwack

PORT_SMOKE_NAMES = $(sort $(PORT_KERNEL_NAMES) $(PORT_EXTRA_NAMES))

PORT_SMOKE_OBJS = \
    $(addprefix $(BUILD_DIR)/obj/port/,$(addsuffix .o,$(PORT_SMOKE_NAMES)))


# ---------------------------------------------------------------------------
# Object compilation and ABI validation
# ---------------------------------------------------------------------------

# Clang emits a .d file beside each .o.  Dependencies are optional during
# a fresh build, but header edits trigger recompilation afterward.
-include $(PORT_SMOKE_OBJS:.o=.d) $(BUILD_DIR)/obj/abi.d

$(BUILD_DIR)/obj/port/%.o: sys/src/9/port/%.c | $(BUILD_DIR)/generated/errstr.h
	@mkdir -p "$(dir $@)"
	$(CLANG) $(ARCH_CFLAGS) $(ARCH_CPPFLAGS) -c "$<" -o "$@"

$(BUILD_DIR)/obj/port/%.o: sys/src/9/port/%.cc | $(BUILD_DIR)/generated/errstr.h
	@mkdir -p "$(dir $@)"
	$(CLANGPP) $(ARCH_CFLAGS) $(ARCH_CPPFLAGS) -c "$<" -o "$@"

# Plan 9's rc-based generator produces definitions consumed by proc.c.
# This GNU Make version keeps generated source out of the original tree.
$(BUILD_DIR)/generated/errstr.h: sys/src/9/port/error.h sys/src/9/pc64/generrstr.py
	$(PYTHON) sys/src/9/pc64/generrstr.py "$<" "$@"

$(BUILD_DIR)/obj/abi.o: tests/amd64/abi.c
	@mkdir -p "$(dir $@)"
	$(CLANG) $(ARCH_CFLAGS) $(ARCH_CPPFLAGS) -c "$<" -o "$@"

.PHONY: abi-check port-smoke port-audit

abi-check: $(BUILD_DIR)/obj/abi.o
	@echo "AMD64 ABI compile-time checks: PASS"

port-smoke: $(PORT_SMOKE_OBJS)
	@echo "Plan 9 machine-independent smoke objects: $(words $(PORT_SMOKE_OBJS)) compiled"

port-audit: $(BUILD_DIR)/generated/errstr.h
	$(PYTHON) sys/src/9/pc64/audit.py \
		--clang "$(CLANG)" \
		--output "$(BUILD_DIR)/audit"


# ---------------------------------------------------------------------------
# Stage 3: native AMD64 ABI consistency and Stage 4: machine-layer objects
#
# These are genuine components of the eventual Plan 9 kernel, not an
# independently bootable test kernel.  ld.lld -r checks that the objects
# can be combined without duplicate definitions.  External references to
# unported Plan 9 kernel subsystems remain unresolved by design.
# ---------------------------------------------------------------------------

# Original Plan 9 initcode source with an AMD64 replacement entry, built
# for AMD64 as a single embedded user text page, not a substitute program.
INIT_USER_CFLAGS := --target=$(TARGET_TRIPLE) -std=c11 -Os -g \
    -ffreestanding -fno-builtin -fno-stack-protector -fno-pic \
    -fno-asynchronous-unwind-tables -fno-unwind-tables \
    -fno-exceptions -mno-red-zone -mcmodel=small -fno-plt \
    -ffunction-sections -fdata-sections \
    -Isys/include -Iamd64/include -Isys/src/libc/9syscall \
    -Isys/src/libc/amd64

INIT_USER_OBJ := \
    $(BUILD_DIR)/obj/userinit/initcode.o \
    $(BUILD_DIR)/obj/userinit/initcalls.o \
    $(BUILD_DIR)/obj/userinit/userstart.o \
    $(BUILD_DIR)/obj/userlib/syscall.o
INIT_USER_ELF := $(BUILD_DIR)/userinit/initcode.elf
INIT_USER_BIN := $(BUILD_DIR)/userinit/initcode.bin
INIT_USER_HEADER := $(BUILD_DIR)/generated/initcode_amd64.h

$(BUILD_DIR)/obj/userinit/initcode.o: sys/src/9/port/initcode.c
	@mkdir -p "$(dir $@)"
	$(CLANG) $(INIT_USER_CFLAGS) -c "$<" -o "$@"

$(BUILD_DIR)/obj/userinit/initcalls.o: sys/src/libc/amd64/initcalls.c
	@mkdir -p "$(dir $@)"
	$(CLANG) $(INIT_USER_CFLAGS) -c "$<" -o "$@"

$(BUILD_DIR)/obj/userinit/userstart.o: sys/src/9/pc64/userstart.S
	@mkdir -p "$(dir $@)"
	$(CLANG) --target=$(TARGET_TRIPLE) -mno-red-zone -c "$<" -o "$@"

$(INIT_USER_ELF): $(INIT_USER_OBJ) sys/src/9/pc64/initcode.ld
	@mkdir -p "$(dir $@)"
	$(LLD) -m elf_x86_64 -nostdlib -static --gc-sections \
	    --build-id=none -T sys/src/9/pc64/initcode.ld \
	    -o "$@" $(INIT_USER_OBJ)

$(INIT_USER_BIN): $(INIT_USER_ELF)
	@mkdir -p "$(dir $@)"
	$(OBJCOPY) -O binary "$<" "$@"

$(INIT_USER_HEADER): $(INIT_USER_BIN) sys/src/9/pc64/geninitcode.py
	$(PYTHON) sys/src/9/pc64/geninitcode.py "$<" "$@"

$(BUILD_DIR)/obj/pc64/userinit.o: $(INIT_USER_HEADER)

.PHONY: user-initcode-check
user-initcode-check: $(INIT_USER_HEADER)
	$(PYTHON) tests/amd64/user_initcode.py "$(INIT_USER_ELF)" "$(INIT_USER_BIN)"
	@echo "Native original Plan 9 initcode image: PASS (static link/address test)"

check: user-initcode-check

MACHINE_C_NAMES := \
    dat \
    config \
    main \
    console \
    clock \
    portglobals \
    bootmem \
    mmu \
    mmuproc \
    segments \
    trap64 \
    syscall64 \
    userentry \
	userinit \
    traputil \
    procmach \
    power \
    errstr \
    kentry \
    atomics

MACHINE_ASM_NAMES := \
    lowlevel \
    trapvec \
    processentry \
    useriret \
    entry \
    atomic

MACHINE_C_OBJS := \
    $(addprefix $(BUILD_DIR)/obj/pc64/,$(addsuffix .o,$(MACHINE_C_NAMES)))

MACHINE_ASM_OBJS := \
    $(addprefix $(BUILD_DIR)/obj/pc64/,$(addsuffix .o,$(MACHINE_ASM_NAMES)))

MACHINE_OBJS := $(MACHINE_C_OBJS) $(MACHINE_ASM_OBJS)
MACHINE_RELOC := $(BUILD_DIR)/obj/pc64-machine.o

-include $(MACHINE_C_OBJS:.o=.d)

$(BUILD_DIR)/obj/pc64/%.o: sys/src/9/pc64/%.c
	@mkdir -p "$(dir $@)"
	$(CLANG) $(ARCH_CFLAGS) $(ARCH_CPPFLAGS) -c "$<" -o "$@"

$(BUILD_DIR)/obj/pc64/%.o: sys/src/9/pc64/%.S
	@mkdir -p "$(dir $@)"
	$(CLANG) --target=$(TARGET_TRIPLE) -mno-red-zone -c "$<" -o "$@"

$(MACHINE_RELOC): $(MACHINE_OBJS)
	@mkdir -p "$(dir $@)"
	$(LLD) -m elf_x86_64 -r $(MACHINE_OBJS) -o "$@"

.PHONY: machine-smoke machine-unit-tests boot-entry-check

machine-smoke: $(MACHINE_RELOC)
	@echo "Plan 9 AMD64 machine-layer relocatable link: PASS"

# Host tests use the same implementation files with a substitute physical
# page allocator; they never run privileged CPU instructions from userspace.
HOST_TEST_FLAGS := \
    -std=c11 \
    -O2 \
    -ffreestanding \
    -fno-builtin \
    -fno-stack-protector \
    -fno-pic \
    -mno-red-zone \
    -Wno-comment \
    -no-pie \
    $(ARCH_INCLUDES)

$(BUILD_DIR)/tests/mmu: tests/amd64/mmu.c sys/src/9/pc64/mmu.c
	@mkdir -p "$(dir $@)"
	$(CLANG) $(HOST_TEST_FLAGS) $^ -o "$@"

$(BUILD_DIR)/tests/label: tests/amd64/label.c sys/src/9/pc64/lowlevel.S
	@mkdir -p "$(dir $@)"
	$(CLANG) $(HOST_TEST_FLAGS) $^ -o "$@"

$(BUILD_DIR)/tests/atomic: tests/amd64/atomic.c sys/src/9/pc64/atomic.S
	@mkdir -p "$(dir $@)"
	$(CLANG) -std=c11 -O2 -no-pie $^ -o "$@"

boot-entry-check: $(BUILD_DIR)/obj/pc64/entry.o
	$(PYTHON) tests/amd64/boot_entry.py "$<"

machine-unit-tests: $(BUILD_DIR)/tests/mmu $(BUILD_DIR)/tests/label $(BUILD_DIR)/tests/atomic
	$(BUILD_DIR)/tests/mmu
	$(BUILD_DIR)/tests/label
	$(BUILD_DIR)/tests/atomic
	@echo "MMU, Label and atomic CPU ABI tests: PASS"


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

KERNEL_LDSCRIPT := sys/src/9/pc64/kernel.ld

# Preserve the original pc/mkfile core /port selection.  Device selection
# lists only drivers whose machine dependencies are actually linked.
PORT_KERNEL_NAMES := \
    alarm \
    alloc \
    allocb \
    auth \
    cache \
    chan \
    dev \
    edf \
    fault \
    latin1 \
    page \
    parse \
    pgrp \
    portclock \
    print \
    proc \
    qio \
    qlock \
    rdb \
    rebootcmd \
    segment \
    swap \
    sysfile \
    sysproc \
    taslock \
    tod \
    watermarks \
    xalloc \
    devcons \
    devenv \
    devmnt \
    devpipe \
    devsrv \
    devdup \
    devroot \
    devwd \
    devkprof \
    devproc \
    mul64fract \
    random

PORT_KERNEL_OBJS := \
    $(addprefix $(BUILD_DIR)/obj/port/,$(addsuffix .o,$(PORT_KERNEL_NAMES)))

# ---------------------------------------------------------------------------
# Original Plan 9 libc/port routines required by the kernel
#
# Compile the historical implementation rather than substitute host libc.
# Formatting and kernel console routines are deliberately not replaced by
# host-side shims.  Their proper /port wiring remains part of kernel linking.
# ---------------------------------------------------------------------------

KERNEL_LIBC_NAMES := \
    cleanname \
    memccpy \
    memchr \
    memcmp \
    memmove \
    memset \
    qsort \
    rune \
    strchr \
    strcmp \
    strcpy \
    strlen \
    strncmp \
    strncpy \
    strrchr \
    strtoul \
    tokenize \
    utflen \
    utfnlen \
    utfrune \
    strecpy \
    utfecpy \
    strtol \
    atol \
    pool

KERNEL_LIBC_OBJS := \
    $(addprefix $(BUILD_DIR)/obj/libc/,$(addsuffix .o,$(KERNEL_LIBC_NAMES)))

-include $(KERNEL_LIBC_OBJS:.o=.d)

$(BUILD_DIR)/obj/libc/%.o: sys/src/libc/port/%.c
	@mkdir -p "$(dir $@)"
	$(CLANG) $(ARCH_CFLAGS) $(ARCH_CPPFLAGS) -c "$<" -o "$@"

.PHONY: kernel-libc
kernel-libc: $(KERNEL_LIBC_OBJS)
	@echo "Original Plan 9 libc port objects: $(words $(KERNEL_LIBC_OBJS)) compiled"


# Genuine Plan 9 9P wire protocol: Fcall/Dir marshaling for /dev/mnt.
KERNEL_9SYS_NAMES := \
    convD2M \
    convM2D \
    convM2S \
    convS2M \
    fcallfmt

KERNEL_9SYS_OBJS := \
    $(addprefix $(BUILD_DIR)/obj/9sys/,$(addsuffix .o,$(KERNEL_9SYS_NAMES)))

-include $(KERNEL_9SYS_OBJS:.o=.d)

$(BUILD_DIR)/obj/9sys/%.o: sys/src/libc/9sys/%.c
	@mkdir -p "$(dir $@)"
	$(CLANG) $(ARCH_CFLAGS) $(ARCH_CPPFLAGS) -c "$<" -o "$@"

# Original Plan 9 formatting implementation (not the host printf family).
# fmtvprint/vseprint/vsnprint use va_copy for Clang's AMD64 va_list ABI.
KERNEL_FMT_NAMES := \
    dofmt \
    dorfmt \
    errfmt \
    fmt \
    fmtprint \
    fmtquote \
    fmtrune \
    sprint \
    fmtstr \
    fmtvprint \
    runefmtstr \
    seprint \
    snprint \
    vseprint \
    vsnprint

KERNEL_FMT_OBJS := \
    $(addprefix $(BUILD_DIR)/obj/fmt/,$(addsuffix .o,$(KERNEL_FMT_NAMES)))

-include $(KERNEL_FMT_OBJS:.o=.d)

$(BUILD_DIR)/obj/fmt/%.o: sys/src/libc/fmt/%.c
	@mkdir -p "$(dir $@)"
	$(CLANG) $(ARCH_CFLAGS) $(ARCH_CPPFLAGS) -c "$<" -o "$@"

.PHONY: kernel-fmt
kernel-fmt: $(KERNEL_FMT_OBJS)
	@echo "Original Plan 9 formatter objects: $(words $(KERNEL_FMT_OBJS)) compiled"


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
$(KERNEL_ELF): $(MACHINE_OBJS) $(PORT_KERNEL_OBJS) $(KERNEL_LIBC_OBJS) $(KERNEL_9SYS_OBJS) $(KERNEL_FMT_OBJS) $(KERNEL_LDSCRIPT)
	@mkdir -p "$(dir $@)"
	$(LLD) \
		-m elf_x86_64 \
		-nostdlib \
		-z max-page-size=0x1000 \
		-T "$(KERNEL_LDSCRIPT)" \
		-o "$@" \
		$(MACHINE_OBJS) \
		$(PORT_KERNEL_OBJS) \
		$(KERNEL_LIBC_OBJS) \
		$(KERNEL_9SYS_OBJS) \
		$(KERNEL_FMT_OBJS)

# The flat image contains the *same* native kernel image laid out by its
# physical load addresses; it is not a distinct executable or kernel.
# objcopy drops ELF metadata, but the ELF remains available for GDB.
$(KERNEL_MB): $(KERNEL_ELF)
	@mkdir -p "$(dir $@)"
	$(OBJCOPY) -O binary "$<" "$@"

# Linked image layout and original Plan 9 kernel symbol validation.
.PHONY: kernel-verify
kernel-verify: $(KERNEL_ELF) $(KERNEL_MB)
	$(PYTHON) tests/amd64/kernel_elf.py "$(KERNEL_ELF)" "$(KERNEL_MB)"

.PHONY: kernel qemu qemu-gdb qemu-preflight

kernel: $(KERNEL_ELF) $(KERNEL_MB)
	@echo "Native Plan 9/pc64 kernel linked: $(KERNEL_ELF)"
	@echo "QEMU Multiboot physical image: $(KERNEL_MB)"

qemu-preflight:
	@command -v "$(QEMU)" >/dev/null || { \
		printf '%s\n' 'ERROR: qemu-system-x86_64 not found in PATH.' \
			'Install QEMU (for Debian/Ubuntu: sudo apt install qemu-system-x86).' >&2; \
		exit 1; \
	}
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
		$(QEMU_EXTRA_ARGS)

# With the target-specific variable, all QEMU options stay in one recipe.
qemu-gdb: QEMU_EXTRA_ARGS += -S -gdb tcp::$(QEMU_GDB_PORT)
qemu-gdb: qemu

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

USER_SYSCALL_OBJ := $(BUILD_DIR)/obj/userlib/syscall.o
$(USER_SYSCALL_OBJ): sys/src/libc/amd64/syscall.S
	@mkdir -p "$(dir $@)"
	$(CLANG) --target=$(TARGET_TRIPLE) -mno-red-zone -c "$<" -o "$@"

.PHONY: user-abi-check
user-abi-check: $(USER_SYSCALL_OBJ) $(BUILD_DIR)/obj/pc64/useriret.o $(BUILD_DIR)/obj/pc64/syscall64.o $(BUILD_DIR)/obj/abi.o
	$(PYTHON) tests/amd64/user_abi.py \
		"$(USER_SYSCALL_OBJ)" \
		"$(BUILD_DIR)/obj/pc64/useriret.o" \
		"$(BUILD_DIR)/obj/pc64/syscall64.o"