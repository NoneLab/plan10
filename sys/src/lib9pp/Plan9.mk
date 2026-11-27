LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := lib9pp
LOCAL_TARGET := userspace

LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/lib9pp

LOCAL_SRC_FILES := 


LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include

include $(BUILD_STATIC_LIBRARY)