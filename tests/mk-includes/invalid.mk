# Negative declarations for registration/ABI/re-export diagnostics.
BUILD_DIR ?= out/amd64/include-invalid
CLANG ?= clang
include mk/plan9.mk
LOCAL_PATH := tests/mk-includes/src
include $(CLEAR_VARS)
LOCAL_MODULE := header-provider
LOCAL_TARGET := userspace
LOCAL_EXPORT_C_INCLUDE_DIRS := tests/mk-includes/include/abi
include $(BUILD_HEADER_LIBRARY)

ifeq ($(CASE),mismatch)
include $(CLEAR_VARS)
LOCAL_MODULE := bad
LOCAL_TARGET := host
LOCAL_OBJ_DIR := $(BUILD_DIR)/bad
LOCAL_SRC_FILES := math.c
LOCAL_CFLAGS := -MMD -MP
LOCAL_HEADER_LIBRARIES := header-provider
include $(BUILD_OBJECTS)
else ifeq ($(CASE),unknown)
include $(CLEAR_VARS)
LOCAL_MODULE := bad
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/bad
LOCAL_SRC_FILES := math.c
LOCAL_CFLAGS := -MMD -MP
LOCAL_HEADER_LIBRARIES := missing-header-module
include $(BUILD_OBJECTS)
else ifeq ($(CASE),reexport)
include $(CLEAR_VARS)
LOCAL_MODULE := bad
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/bad
LOCAL_SRC_FILES := math.c
LOCAL_CFLAGS := -MMD -MP
LOCAL_EXPORT_HEADER_LIBRARY_HEADERS := header-provider
include $(BUILD_OBJECTS)
else ifeq ($(CASE),wrong-provider-type)
include $(CLEAR_VARS)
LOCAL_MODULE := some-static
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/static
LOCAL_SRC_FILES := math.c
LOCAL_CFLAGS := -MMD -MP
include $(BUILD_STATIC_LIBRARY)
include $(CLEAR_VARS)
LOCAL_MODULE := bad
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/bad
LOCAL_SRC_FILES := math.c
LOCAL_HEADER_LIBRARIES := some-static
include $(BUILD_OBJECTS)
else ifeq ($(CASE),header-kind)
include $(CLEAR_VARS)
LOCAL_MODULE := bad
LOCAL_TARGET := userspace
LOCAL_SRC_FILES := math.c
LOCAL_EXPORT_C_INCLUDE_DIRS := tests/mk-includes/include/abi
include $(BUILD_HEADER_LIBRARY)
endif
all: ; @true
