# Native Plan 9 init: compile the historical sys/src/cmd/init.c unchanged.
# Included by pc64/Plan9.mk after the userspace libraries and KFS objects.
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := init-amd64
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/init
LOCAL_SRC_FILES := init.c
LOCAL_CFLAGS := $(BOOT64_USER_CFLAGS)
include $(BUILD_OBJECTS)
INIT_SOURCE_OBJS := $(LOCAL_BUILT_OBJECTS)

# The common CRT supplies _start. libc comes from libc-plan9.a, while
# authentication and other support objects are still linked explicitly.
INIT_SUPPORT_OBJS := \
    $(KFS_SUPPORT_OBJS) \
    $(INIT_AUTH_OBJS)

# Bootstrap executable: init loads from disk-backed KFS by default.
# Both bootstrap and regular userspace use the same _start.
include $(CLEAR_VARS)
LOCAL_MODULE := init-executable
LOCAL_TARGET := userspace
LOCAL_MODULE_PATH := $(BUILD_DIR)/init
LOCAL_MODULE_FILENAME := init
LOCAL_INSTALL_IMAGE := kfs
LOCAL_INSTALL_PATH := /amd64
LOCAL_SYSTEM_STATIC_LIBRARIES := libc-plan9
LOCAL_EXTRA_OBJECTS := \
    $(INIT_SOURCE_OBJS) \
    $(INIT_SUPPORT_OBJS)
include $(BUILD_BOOTSTRAP_EXECUTABLE)
INIT_ELF := $(LOCAL_BUILT_ELF)
INIT_NATIVE_IMAGE := $(LOCAL_BUILT_MODULE)

.PHONY: init-amd64 init-source-check
init-amd64: $(INIT_NATIVE_IMAGE)
	@echo "Original Plan 9 init packaged: $(INIT_NATIVE_IMAGE)"
init-source-check: $(INIT_SOURCE_OBJS)
	@echo "Original Plan 9 init source compiled"

.PHONY: init-native-check
init-native-check: $(INIT_NATIVE_IMAGE) tests/amd64/init64_native.py
	$(PYTHON) tests/amd64/init64_native.py "$(INIT_ELF)" "$(INIT_NATIVE_IMAGE)"

check: init-native-check