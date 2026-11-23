# Plan 9 module: sys/src/libip
# Loaded by the root build; paths are relative to the repository root.
# Module declarations, object lists, and owned targets live here.

# Plan 9 source-owned module definitions (Plan9.mk-style, GNU Make backend).
# Parsed by GNU Make; not compatible with AOSP build/core or Kati.
LOCAL_PATH := $(call my-dir)

# boot64-libip
include $(CLEAR_VARS)
LOCAL_MODULE := boot64-libip
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/boot64-lib/ip
LOCAL_SRC_FILES := \
    bo.c parseip.c eipfmt.c ipaux.c
LOCAL_CFLAGS := $(BOOT64_USER_CFLAGS)
include $(BUILD_OBJECTS)
BOOT64_IP_OBJS := $(LOCAL_BUILT_OBJECTS)


.PHONY: boot64-libip
boot64-libip: $(BOOT64_IP_OBJS)
	@echo "Original Plan 9 libip boot objects: $(words $^) compiled"
