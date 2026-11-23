# Plan 9 module: sys/src/9/pc64/init
# Loaded by the root build; paths are relative to the repository root.
# Module declarations, object lists, and owned targets live here.

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

# The initial CPL3 image includes /port initcode and amd64 ABI stubs.
# Plan 9 source-owned module definitions (Plan9.mk-style, GNU Make backend).
# Parsed by GNU Make; not compatible with AOSP build/core or Kati.
LOCAL_PATH := $(call my-dir)

# pc64-initcode
include $(CLEAR_VARS)
LOCAL_MODULE := pc64-initcode
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/userinit
LOCAL_SRC_DIR := sys/src/9/port
LOCAL_SRC_FILES := \
    initcode.c
LOCAL_CFLAGS := $(INIT_USER_CFLAGS)
include $(BUILD_OBJECTS)
INITCODE_OBJ := $(LOCAL_BUILT_OBJECTS)

# pc64-initcalls
include $(CLEAR_VARS)
LOCAL_MODULE := pc64-initcalls
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/userinit
LOCAL_SRC_DIR := sys/src/libc/amd64
LOCAL_SRC_FILES := \
    initcalls.c
LOCAL_CFLAGS := $(INIT_USER_CFLAGS)
include $(BUILD_OBJECTS)
INITCALLS_OBJ := $(LOCAL_BUILT_OBJECTS)

# pc64-userstart
include $(CLEAR_VARS)
LOCAL_MODULE := pc64-userstart
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/userinit
LOCAL_SRC_DIR := sys/src/9/pc64
LOCAL_SRC_FILES := \
    userstart.S
LOCAL_CFLAGS := $(INIT_USER_CFLAGS)
LOCAL_ASFLAGS := --target=$(TARGET_TRIPLE) -mno-red-zone
include $(BUILD_OBJECTS)
USERSTART_OBJ := $(LOCAL_BUILT_OBJECTS)


INIT_USER_OBJ := $(INITCODE_OBJ) $(INITCALLS_OBJ) $(USERSTART_OBJ) $(USER_SYSCALL_OBJ)
INIT_USER_ELF := $(BUILD_DIR)/userinit/initcode.elf
INIT_USER_BIN := $(BUILD_DIR)/userinit/initcode.bin
INIT_USER_HEADER := $(BUILD_DIR)/generated/initcode_amd64.h

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
