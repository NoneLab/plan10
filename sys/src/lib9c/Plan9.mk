LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := lib9c
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/lib9c
LOCAL_SRC_FILES := \
    malloc/malloc.c \
    malloc/pool.c \
    malloc/sbrk.c \
    native/file.c \
    native/ipc.c \
    native/process.c \
    native/namespace.c \
    native/segment.c \
    native/error.c \
    helpers/io.c \
    helpers/dir.c \
    helpers/string.c \
    helpers/process_util.c \
    helpers/env.c \
    helpers/rune.c \
    sync/lock.c \
    sync/qlock.c
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include
LOCAL_STATIC_LIBRARIES := lib10c
LOCAL_CFLAGS := $(BOOT64_USER_COMMON_FLAGS) -std=gnu11 -fno-strict-aliasing
include $(BUILD_STATIC_LIBRARY)

.PHONY: lib9c
lib9c: $(LOCAL_BUILT_ARCHIVE)
