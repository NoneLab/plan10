# U3.10 integration fixture using the actual production backends.
.DEFAULT_GOAL := all
BUILD_DIR ?= out/amd64/include-selftest
CLANG ?= clang
CLANGPP ?= clang++
LLD ?= ld.lld
PYTHON ?= python3
AR ?= ar
TEST_CFLAGS := --target=x86_64-unknown-none-elf -ffreestanding -fno-builtin -O1 -MMD -MP
include mk/plan9.mk
LOCAL_PATH := tests/mk-includes/src

include $(CLEAR_VARS)
LOCAL_MODULE := abi-headers
LOCAL_TARGET := userspace
LOCAL_EXPORT_C_INCLUDE_DIRS := tests/mk-includes/include/abi $(BUILD_DIR)/gen
LOCAL_EXPORT_C_INCLUDE_DEPS := $(BUILD_DIR)/gen/generated.h
include $(BUILD_HEADER_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := math-priv
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/math
LOCAL_SRC_FILES := math.c
LOCAL_CFLAGS := $(TEST_CFLAGS)
LOCAL_EXPORT_C_INCLUDE_DIRS := tests/mk-includes/include/math
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libcore
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/core
LOCAL_SRC_FILES := core.c
LOCAL_CFLAGS := $(TEST_CFLAGS)
LOCAL_STATIC_LIBRARIES := math-priv
LOCAL_HEADER_LIBRARIES := abi-headers
LOCAL_EXPORT_C_INCLUDE_DIRS := tests/mk-includes/include/core
LOCAL_EXPORT_HEADER_LIBRARY_HEADERS := abi-headers
ifeq ($(REEXPORT_PRIVATE),1)
LOCAL_EXPORT_STATIC_LIBRARY_HEADERS := math-priv
endif
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libc-plan9
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/plan9
LOCAL_SRC_FILES := runtime.c
LOCAL_CFLAGS := $(TEST_CFLAGS)
LOCAL_EXPORT_C_INCLUDE_DIRS := tests/mk-includes/include/plan9
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libc-experimental
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/experimental
LOCAL_SRC_FILES := runtime.c
LOCAL_CFLAGS := $(TEST_CFLAGS)
LOCAL_EXPORT_C_INCLUDE_DIRS := tests/mk-includes/include/experimental
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := crt0-amd64
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/crt
LOCAL_SRC_FILES := crt.S
LOCAL_ASFLAGS := --target=x86_64-unknown-none-elf -mno-red-zone
include $(BUILD_OBJECTS)

include $(CLEAR_VARS)
LOCAL_MODULE := test-app
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/app
LOCAL_MODULE_PATH := $(BUILD_DIR)/bin
LOCAL_SRC_FILES := app.c
LOCAL_CFLAGS := $(TEST_CFLAGS) -DEXPECT_EXPERIMENTAL=$(EXPECT_EXPERIMENTAL) $(EXTRA_APP_CFLAGS)
LOCAL_C_INCLUDES := $(EXTRA_APP_INCLUDE)
LOCAL_STATIC_LIBRARIES := libcore
LOCAL_SYSTEM_STATIC_LIBRARIES := default
include $(BUILD_USER_EXECUTABLE)
TEST_APP := $(LOCAL_BUILT_MODULE)

.PHONY: all
all: $(TEST_APP)

# Generate a header before any consumer compiles it.
$(BUILD_DIR)/gen/generated.h:
	@mkdir -p "$(dir $@)"
	@printf '%s\n' '#define GENERATED_VALUE 13' > "$@"

include $(CLEAR_VARS)
LOCAL_MODULE := test-cxx
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/cxx
LOCAL_SRC_FILES := cxx.cpp
LOCAL_CXXFLAGS := $(TEST_CFLAGS)
LOCAL_STATIC_LIBRARIES := libcore
LOCAL_HEADER_LIBRARIES := abi-headers
include $(BUILD_OBJECTS)
TEST_CXX_OBJECTS := $(LOCAL_BUILT_OBJECTS)

include $(CLEAR_VARS)
LOCAL_MODULE := test-asm
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/asm
LOCAL_SRC_FILES := asm.S
LOCAL_ASFLAGS := --target=x86_64-unknown-none-elf -mno-red-zone
LOCAL_HEADER_LIBRARIES := abi-headers
include $(BUILD_OBJECTS)
TEST_ASM_OBJECTS := $(LOCAL_BUILT_OBJECTS)

.PHONY: extra
extra: $(TEST_CXX_OBJECTS) $(TEST_ASM_OBJECTS)
