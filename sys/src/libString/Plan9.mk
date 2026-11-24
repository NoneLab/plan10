# Native rc uses the historical Plan 9 libString for command substitution.
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := rc-libString
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/rc/libString
LOCAL_SRC_FILES := s_alloc.c s_nappend.c s_putc.c s_terminate.c \
    s_reset.c s_grow.c s_unique.c s_copy.c s_append.c s_memappend.c
LOCAL_CFLAGS := $(BOOT64_USER_CFLAGS)
include $(BUILD_OBJECTS)
RC_STRING_OBJS := $(LOCAL_BUILT_OBJECTS)