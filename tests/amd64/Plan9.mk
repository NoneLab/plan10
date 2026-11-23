# Plan 9 module: tests/amd64
# Loaded by the root build; paths are relative to the repository root.
# Module declarations, object lists, and owned targets live here.

# Track ABI test headers, including generated dependencies.
# Compile-time native ABI test uses the same declarative object rules.
# Plan 9 source-owned module definitions (Plan9.mk-style, GNU Make backend).
# Parsed by GNU Make; not compatible with AOSP build/core or Kati.
LOCAL_PATH := $(call my-dir)

# pc64-abi-test
include $(CLEAR_VARS)
LOCAL_MODULE := pc64-abi-test
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj
LOCAL_SRC_FILES := \
    abi.c
LOCAL_CFLAGS := $(ARCH_CFLAGS) $(ARCH_CPPFLAGS)
include $(BUILD_OBJECTS)

abi-check: $(BUILD_DIR)/obj/abi.o
	@echo "AMD64 ABI compile-time checks: PASS"

# Native pc64 execution/format tests
# On-disk 6.out decoding is checked against native and malformed headers.
$(BUILD_DIR)/tests/exec64: tests/amd64/exec64.c sys/src/9/pc64/exec64.c
	@mkdir -p "$(dir $@)"
	$(CLANG) -std=c11 -O2 -Isys/include -Iamd64/include -Isys/src/9/pc64 $^ -o "$@"

.PHONY: exec64-check
exec64-check: $(BUILD_DIR)/tests/exec64
	$(BUILD_DIR)/tests/exec64

check: exec64-check

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

# Host-only format tests do not embed a placeholder executable.
.PHONY: boot6-format-check
boot6-format-check:
	$(PYTHON) tests/amd64/boot6_pipeline.py

check: boot6-format-check

# User syscall entry and return ABI
.PHONY: user-abi-check
user-abi-check: $(USER_SYSCALL_OBJ) $(BUILD_DIR)/obj/pc64/useriret.o $(BUILD_DIR)/obj/pc64/syscall64.o $(BUILD_DIR)/obj/abi.o
	$(PYTHON) tests/amd64/user_abi.py \
		"$(USER_SYSCALL_OBJ)" \
		"$(BUILD_DIR)/obj/pc64/useriret.o" \
		"$(BUILD_DIR)/obj/pc64/syscall64.o"
