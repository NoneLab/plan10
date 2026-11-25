# Original Plan 9 KFS user-level file server, AMD64 native build.
# The source list comes from the historical mkfile in this directory.
# This descriptor is included by the pc64 root build, not run recursively.
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := kfs-amd64
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/kfs
LOCAL_SRC_FILES := \
    9p1.c 9p1lib.c 9p2.c 9p12.c \
    auth.c chk.c con.c console.c dat.c dentry.c \
    ialloc.c iobuf.c main.c misc.c porttime.c sub.c \
    uid.c ofcallfmt.c devwren.c
LOCAL_CFLAGS := $(BOOT64_USER_CFLAGS) -I$(LOCAL_PATH) \
    -Wno-unknown-pragmas
include $(BUILD_OBJECTS)
KFS_SOURCE_OBJS := $(LOCAL_BUILT_OBJECTS)

include $(CLEAR_VARS)
LOCAL_MODULE := kfs-libauthsrv
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/kfs/libauthsrv
LOCAL_SRC_DIR := sys/src/libauthsrv
LOCAL_SRC_FILES := readnvram.c nvcsum.c convM2T.c convM2A.c convA2M.c passtokey.c
LOCAL_CFLAGS := $(BOOT64_USER_CFLAGS)
include $(BUILD_OBJECTS)
KFS_AUTHSRV_OBJS := $(LOCAL_BUILT_OBJECTS)

include $(CLEAR_VARS)
LOCAL_MODULE := kfs-libsec
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/kfs/libsec
LOCAL_SRC_DIR := sys/src/libsec/port
LOCAL_SRC_FILES := des.c
LOCAL_CFLAGS := $(BOOT64_USER_CFLAGS) -Isys/src/libmp/port
include $(BUILD_OBJECTS)
KFS_SEC_OBJS := $(LOCAL_BUILT_OBJECTS)

# KFS contributes authentication and security code outside libc.
KFS_SUPPORT_OBJS := \
    $(BOOT64_SUPPORT_OBJS) \
    $(KFS_AUTHSRV_OBJS) \
    $(KFS_SEC_OBJS)

# Bootstrap executable: common CRT and KFS-specific print(2).
include $(CLEAR_VARS)
LOCAL_MODULE := kfs-executable
LOCAL_TARGET := userspace
LOCAL_MODULE_PATH := $(BUILD_DIR)/kfs
LOCAL_MODULE_FILENAME := kfs
LOCAL_SYSTEM_STATIC_LIBRARIES := libc-plan9
LOCAL_EXTRA_OBJECTS := \
    $(KFS_SOURCE_OBJS) \
    $(KFS_SUPPORT_OBJS)
include $(BUILD_BOOTSTRAP_EXECUTABLE)
KFS_ELF := $(LOCAL_BUILT_ELF)
KFS_IMAGE := $(LOCAL_BUILT_MODULE)

.PHONY: kfs-amd64 kfs-source-check
kfs-amd64: $(KFS_IMAGE)
	@echo "Original Plan 9 KFS packaged: $(KFS_IMAGE)"

kfs-source-check: $(KFS_SOURCE_OBJS)
	@echo "Original Plan 9 KFS sources: $(words $(KFS_SOURCE_OBJS)) objects compiled"
