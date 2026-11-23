# Plan 9 module: sys/src/libbio
# Loaded by the root build; paths are relative to the repository root.
# Module declarations, object lists, and owned targets live here.

# Plan 9 source-owned module definitions (Plan9.mk-style, GNU Make backend).
# Parsed by GNU Make; not compatible with AOSP build/core or Kati.
LOCAL_PATH := $(call my-dir)

# boot64-libbio
include $(CLEAR_VARS)
LOCAL_MODULE := boot64-libbio
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/boot64-lib/bio
LOCAL_SRC_FILES := \
    binit.c brdline.c bgetc.c bflush.c \
    bseek.c bbuffered.c bread.c bwrite.c
LOCAL_CFLAGS := $(BOOT64_USER_CFLAGS)
include $(BUILD_OBJECTS)
BOOT64_BIO_OBJS := $(LOCAL_BUILT_OBJECTS)


.PHONY: boot64-libbio
boot64-libbio: $(BOOT64_BIO_OBJS)
	@echo "Original Plan 9 libbio boot objects: $(words $^) compiled"
