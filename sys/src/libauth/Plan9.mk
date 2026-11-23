# Plan 9 module: sys/src/libauth
# Loaded by the root build; paths are relative to the repository root.
# Module declarations, object lists, and owned targets live here.

# Plan 9 source-owned module definitions (Plan9.mk-style, GNU Make backend).
# Parsed by GNU Make; not compatible with AOSP build/core or Kati.
LOCAL_PATH := $(call my-dir)

# boot64-libauth
include $(CLEAR_VARS)
LOCAL_MODULE := boot64-libauth
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/boot64-lib/auth
LOCAL_SRC_FILES := \
    auth_proxy.c auth_getkey.c auth_rpc.c auth_respond.c \
    attr.c
LOCAL_CFLAGS := $(BOOT64_USER_CFLAGS)
include $(BUILD_OBJECTS)
BOOT64_AUTH_OBJS := $(LOCAL_BUILT_OBJECTS)


.PHONY: boot64-libauth
boot64-libauth: $(BOOT64_AUTH_OBJS)
	@echo "Original Plan 9 libauth boot objects: $(words $^) compiled"
