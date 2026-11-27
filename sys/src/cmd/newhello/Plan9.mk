LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := hello
LOCAL_TARGET := userspace

LOCAL_SRC_FILES := hello.cc
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/hello

LOCAL_CFLAGS := \
    $(BOOT64_USER_CFLAGS) \
    -nostdlibinc

LOCAL_CRT := crt0-lib10c-amd64
LOCAL_SYSTEM_STATIC_LIBRARIES := lib10c

LOCAL_INSTALL_IMAGE := kfs
LOCAL_INSTALL_PATH := /bin

include $(BUILD_USER_EXECUTABLE)

.PHONY: hello
hello: $(LOCAL_BUILT_MODULE)