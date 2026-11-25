# Unit/integration check of each isolated backend against the real GNU Make rules.
.DEFAULT_GOAL := check
BUILD_DIR ?= out/amd64/backend-selftest
CLANG ?= clang
CLANGPP ?= clang++
LLD ?= ld.lld
OBJCOPY ?= objcopy
PYTHON ?= python3
AR ?= ar
include mk/plan9.mk
LOCAL_PATH := tests/mk-backends/src
# Select the user runtime through the same public variable as production.
# Bootstrap keeps libc-plan9 regardless of this selection.
TEST_USER_LIBC_VARIANT := $(patsubst libc-%,%,$(USERSPACE_SYSTEM_STATIC_LIBRARIES))

include $(CLEAR_VARS)
LOCAL_MODULE := libc-plan9
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/libc-plan9
LOCAL_SRC_FILES := plan9.c
LOCAL_CFLAGS := --target=x86_64-unknown-none-elf -ffreestanding -fno-builtin -O1 -MMD -MP
include $(BUILD_STATIC_LIBRARY)
include $(CLEAR_VARS)
LOCAL_MODULE := libc-experimental
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/libc-experimental
LOCAL_SRC_FILES := experimental.c
LOCAL_CFLAGS := --target=x86_64-unknown-none-elf -ffreestanding -fno-builtin -O1 -MMD -MP
include $(BUILD_STATIC_LIBRARY)
include $(CLEAR_VARS)
LOCAL_MODULE := p9-math
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/math
LOCAL_SRC_FILES := math.c
LOCAL_CFLAGS := --target=x86_64-unknown-none-elf -ffreestanding -fno-builtin -O1 -MMD -MP
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := p9-extra
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/extra
LOCAL_SRC_FILES := extra.c
LOCAL_CFLAGS := --target=x86_64-unknown-none-elf -ffreestanding -fno-builtin -O1 -MMD -MP
LOCAL_STATIC_LIBRARIES := p9-math
include $(BUILD_STATIC_LIBRARY)

# Register the production default CRT as a normal userspace object module.
include $(CLEAR_VARS)
LOCAL_MODULE := crt0-amd64
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/crt0
LOCAL_SRC_DIR := sys/src/libc/amd64
LOCAL_SRC_FILES := crt0.S
LOCAL_ASFLAGS := --target=x86_64-unknown-none-elf -mno-red-zone
include $(BUILD_OBJECTS)

# Keep an alternate entry to test an explicit LOCAL_CRT override.
include $(CLEAR_VARS)
LOCAL_MODULE := backend-test-crt
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/test-crt
LOCAL_SRC_FILES := crt.S
LOCAL_ASFLAGS := --target=x86_64-unknown-none-elf -mno-red-zone
include $(BUILD_OBJECTS)

include $(CLEAR_VARS)
LOCAL_MODULE := backend-test-aux
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/aux
LOCAL_SRC_FILES := aux.S
LOCAL_ASFLAGS := --target=x86_64-unknown-none-elf -mno-red-zone
include $(BUILD_OBJECTS)

include $(CLEAR_VARS)
LOCAL_MODULE := backend-test-user
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/user/$(TEST_USER_LIBC_VARIANT)
LOCAL_SRC_FILES := user.S
LOCAL_ASFLAGS := --target=x86_64-unknown-none-elf -mno-red-zone
LOCAL_SYSTEM_STATIC_LIBRARIES := default
LOCAL_STATIC_LIBRARIES := p9-extra
LOCAL_OBJECT_LIBRARIES := backend-test-aux
LOCAL_MODULE_PATH := $(BUILD_DIR)/userspace/$(TEST_USER_LIBC_VARIANT)
LOCAL_MODULE_FILENAME := hello
include $(BUILD_USER_EXECUTABLE)
TEST_USER := $(LOCAL_BUILT_MODULE)
TEST_USER_ELF := $(LOCAL_BUILT_ELF)

include $(CLEAR_VARS)
LOCAL_MODULE := backend-test-bootstrap
LOCAL_TARGET := userspace
LOCAL_SYSTEM_STATIC_LIBRARIES := libc-plan9
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/bootstrap
LOCAL_MODULE_PATH := $(BUILD_DIR)/bootstrap
LOCAL_MODULE_FILENAME := early
LOCAL_SRC_FILES := user.S
LOCAL_ASFLAGS := --target=x86_64-unknown-none-elf -mno-red-zone
LOCAL_STATIC_LIBRARIES := p9-extra
include $(BUILD_BOOTSTRAP_EXECUTABLE)
TEST_BOOTSTRAP := $(LOCAL_BUILT_MODULE)

# A specialized bootstrap CRT can still be selected explicitly.
include $(CLEAR_VARS)
LOCAL_MODULE := backend-test-override
LOCAL_TARGET := userspace
LOCAL_MODULE_PATH := $(BUILD_DIR)/bootstrap
LOCAL_MODULE_FILENAME := override
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/override
LOCAL_SRC_FILES := user.S
LOCAL_ASFLAGS := --target=x86_64-unknown-none-elf -mno-red-zone
LOCAL_CRT := backend-test-crt
LOCAL_SYSTEM_STATIC_LIBRARIES := libc-plan9
LOCAL_STATIC_LIBRARIES := p9-extra
include $(BUILD_BOOTSTRAP_EXECUTABLE)
TEST_OVERRIDE := $(LOCAL_BUILT_MODULE)

include $(CLEAR_VARS)
LOCAL_MODULE := backend-test-kernel
LOCAL_TARGET := kernel
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/kernel
LOCAL_SRC_FILES := kernel.S
LOCAL_ASFLAGS := --target=x86_64-unknown-none-elf -mno-red-zone
include $(BUILD_OBJECTS)
# BUILD_KERNEL consumes existing objects and never performs userspace conversion.
include $(CLEAR_VARS)
LOCAL_MODULE := backend-test-kernel-lib
LOCAL_TARGET := kernel
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/kernel-lib
LOCAL_SRC_FILES := kernellib.S
LOCAL_ASFLAGS := --target=x86_64-unknown-none-elf -mno-red-zone
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := backend-kernel-image
LOCAL_TARGET := kernel
LOCAL_KERNEL_OBJECTS := $(BUILD_DIR)/obj/kernel/kernel.o
LOCAL_KERNEL_LDSCRIPT := tests/mk-backends/kernel.ld
LOCAL_STATIC_LIBRARIES := backend-test-kernel-lib
LOCAL_KERNEL_ELF := $(BUILD_DIR)/kernel/kernel.elf
LOCAL_KERNEL_MB := $(BUILD_DIR)/kernel/kernel.mb
include $(BUILD_KERNEL)
TEST_KERNEL := $(LOCAL_BUILT_MODULE)

include $(CLEAR_VARS)
LOCAL_MODULE := backend-test-host-lib
LOCAL_TARGET := host
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/host-lib
LOCAL_SRC_FILES := hostlib.cpp
LOCAL_CXXFLAGS := -std=c++17 -O2 -MMD -MP
include $(BUILD_STATIC_LIBRARY)
TEST_HOST_ARCHIVE := $(LOCAL_BUILT_ARCHIVE)

include $(CLEAR_VARS)
LOCAL_MODULE := backend-test-host
LOCAL_TARGET := host
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/host
LOCAL_MODULE_PATH := $(BUILD_DIR)/host/bin
LOCAL_MODULE_FILENAME := host-test
LOCAL_STATIC_LIBRARIES := backend-test-host-lib
LOCAL_SRC_FILES := host.cpp
LOCAL_CXXFLAGS := -std=c++17 -O2 -MMD -MP
include $(BUILD_HOST_EXECUTABLE)
TEST_HOST := $(LOCAL_BUILT_MODULE)

.PHONY: check check-user check-host check-kernel check-invalid
check: check-user check-bootstrap check-override check-host check-kernel check-invalid check-crt-entry

check-user: $(TEST_USER)
	@$(PYTHON) -c 'from pathlib import Path; p=Path("$(TEST_USER)"); assert p.read_bytes()[:4] == bytes.fromhex("00008a97"), "not native 6.out"; print("PASS userspace 6.out",p)'

check-bootstrap: $(TEST_BOOTSTRAP)
	@$(PYTHON) -c 'from pathlib import Path; p=Path("$(TEST_BOOTSTRAP)"); assert p.read_bytes()[:4] == bytes.fromhex("00008a97"), "not native 6.out"; print("PASS bootstrap 6.out",p)'

check-override: $(TEST_OVERRIDE)
	@$(PYTHON) -c 'from pathlib import Path; p=Path("$(TEST_OVERRIDE)"); assert p.read_bytes()[:4] == bytes.fromhex("00008a97"), "not native 6.out"; print("PASS explicit CRT override",p)'

check-crt-entry:
	@sh tests/mk-backends/test-crt-entry.sh

check-host: $(TEST_HOST)
	@$(TEST_HOST)

check-kernel: $(TEST_KERNEL)
	@test -s "$(TEST_KERNEL)"
	@echo "PASS kernel ELF/MB outputs"

check-invalid:
	@sh tests/mk-backends/test-invalid.sh
