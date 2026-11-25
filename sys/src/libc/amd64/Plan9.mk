# Plan 9 module: sys/src/libc/amd64
# Loaded by the root build; paths are relative to the repository root.
# Module declarations, object lists, and owned targets live here.

# Plan 9 source-owned module definitions (Plan9.mk-style, GNU Make backend).
# Parsed by GNU Make; not compatible with AOSP build/core or Kati.
LOCAL_PATH := $(call my-dir)

# amd64-syscall
include $(CLEAR_VARS)
LOCAL_MODULE := amd64-syscall
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/userlib
LOCAL_SRC_FILES := \
    syscall.S
LOCAL_ASFLAGS := --target=$(TARGET_TRIPLE) -mno-red-zone
include $(BUILD_OBJECTS)
USER_SYSCALL_OBJ := $(LOCAL_BUILT_OBJECTS)

# One _start for bootstrap and regular native 6.out programs.
# crt0.S handles only argc/argv, legacy argv0 and main -> exits(nil).
include $(CLEAR_VARS)
LOCAL_MODULE := crt0-amd64
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/userlib/crt0
LOCAL_SRC_FILES := crt0.S
LOCAL_ASFLAGS := --target=$(TARGET_TRIPLE) -mno-red-zone
include $(BUILD_OBJECTS)
CRT0_AMD64_OBJS := $(LOCAL_BUILT_OBJECTS)
