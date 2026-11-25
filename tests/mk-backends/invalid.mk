# Each case intentionally fails when the backend rejects a bad declaration.
BUILD_DIR ?= out/amd64/backend-invalid
include mk/plan9.mk
LOCAL_PATH := tests/mk-backends/src
include $(CLEAR_VARS)
LOCAL_MODULE := bad-declaration
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/bad
LOCAL_MODULE_PATH := $(BUILD_DIR)/bin
LOCAL_SRC_FILES := host.cpp
LOCAL_CXXFLAGS := -std=c++17

ifeq ($(CASE),host-as-user)
LOCAL_TARGET := host
include $(BUILD_USER_EXECUTABLE)
else ifeq ($(CASE),kernel-as-host)
LOCAL_TARGET := kernel
include $(BUILD_HOST_EXECUTABLE)
else ifeq ($(CASE),user-as-kernel)
LOCAL_TARGET := userspace
LOCAL_KERNEL_OBJECTS := test.o
LOCAL_KERNEL_LDSCRIPT := tests/mk-backends/kernel.ld
include $(BUILD_KERNEL)
else ifeq ($(CASE),unknown-default-crt)
LOCAL_TARGET := userspace
USERSPACE_DEFAULT_CRT := non-existent-crt
include $(BUILD_BOOTSTRAP_EXECUTABLE)
else ifeq ($(CASE),unknown-bootstrap-crt)
LOCAL_TARGET := userspace
LOCAL_CRT := non-existent-crt
include $(BUILD_BOOTSTRAP_EXECUTABLE)
else ifeq ($(CASE),non-user-crt)
include $(CLEAR_VARS)
LOCAL_TARGET := host
LOCAL_MODULE := host-crt
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/host-crt
LOCAL_SRC_FILES := host.cpp
include $(BUILD_OBJECTS)
include $(CLEAR_VARS)
LOCAL_MODULE := bad-crt-app
LOCAL_TARGET := userspace
LOCAL_CRT := host-crt
LOCAL_EXTRA_OBJECTS := tests/mk-backends/src/user.S
include $(BUILD_BOOTSTRAP_EXECUTABLE)
else ifeq ($(CASE),host-library-as-user)
include $(CLEAR_VARS)
LOCAL_MODULE := host-lib
LOCAL_TARGET := host
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/host-lib
LOCAL_SRC_FILES := host.cpp
include $(BUILD_STATIC_LIBRARY)
include $(CLEAR_VARS)
LOCAL_MODULE := bad-user-app
LOCAL_TARGET := userspace
LOCAL_STATIC_LIBRARIES := host-lib
LOCAL_EXTRA_OBJECTS := tests/mk-backends/src/user.S
include $(BUILD_USER_EXECUTABLE)
else ifeq ($(CASE),unknown-user-library)
LOCAL_TARGET := userspace
LOCAL_STATIC_LIBRARIES := nonexistent-library
include $(BUILD_USER_EXECUTABLE)
else ifeq ($(CASE),unknown-system-library)
LOCAL_TARGET := userspace
LOCAL_SYSTEM_STATIC_LIBRARIES := nonexistent-library
include $(BUILD_USER_EXECUTABLE)
else ifeq ($(CASE),mixed-system-default)
LOCAL_TARGET := userspace
LOCAL_SYSTEM_STATIC_LIBRARIES := default libc-plan9
include $(BUILD_USER_EXECUTABLE)
else ifeq ($(CASE),explicit-libc-in-app-list)
LOCAL_TARGET := userspace
LOCAL_STATIC_LIBRARIES := libc-plan9
LOCAL_SYSTEM_STATIC_LIBRARIES :=
include $(BUILD_USER_EXECUTABLE)
else
$(error Unknown CASE $(CASE))
endif
