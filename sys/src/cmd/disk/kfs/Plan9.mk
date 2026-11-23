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

# _start has the same argc/argv/stack ABI as native /boot/boot.
include $(CLEAR_VARS)
LOCAL_MODULE := kfs-amd64-start
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/kfs
LOCAL_SRC_DIR := sys/src/cmd/disk/kfs/amd64
LOCAL_SRC_FILES := start.S
LOCAL_ASFLAGS := --target=$(TARGET_TRIPLE) -mno-red-zone
include $(BUILD_OBJECTS)
KFS_ENTRY_OBJS := $(LOCAL_BUILT_OBJECTS)

# Link the real Plan 9 user libraries needed by the historical KFS sources.
# These are separate objects from boot, preserving the original boot binary.
include $(CLEAR_VARS)
LOCAL_MODULE := kfs-libc-port
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/kfs/libc-port
LOCAL_SRC_DIR := sys/src/libc/port
LOCAL_SRC_FILES := \
    readn.c lrand.c nrand.c \
    nan.c toupper.c frexp.c pow10.c strtod.c fabs.c crypt.c muldiv.c
LOCAL_CFLAGS := $(BOOT64_USER_CFLAGS)
include $(BUILD_OBJECTS)
KFS_PORT_EXTRAS := $(LOCAL_BUILT_OBJECTS)

include $(CLEAR_VARS)
LOCAL_MODULE := kfs-libc-9sys
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/kfs/libc-9sys
LOCAL_SRC_DIR := sys/src/libc/9sys
LOCAL_SRC_FILES := nsec.c time.c announce.c truerand.c fcallfmt.c
LOCAL_CFLAGS := $(BOOT64_USER_CFLAGS)
include $(BUILD_OBJECTS)
KFS_9SYS_EXTRAS := $(LOCAL_BUILT_OBJECTS)

include $(CLEAR_VARS)
LOCAL_MODULE := kfs-libc-fmt
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/kfs/libc-fmt
LOCAL_SRC_DIR := sys/src/libc/fmt
LOCAL_SRC_FILES := fmtlock.c fltfmt.c
LOCAL_CFLAGS := $(BOOT64_USER_CFLAGS)
include $(BUILD_OBJECTS)
KFS_FMT_EXTRAS := $(LOCAL_BUILT_OBJECTS)

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

# KFS supplies its own print(2) implementation to keep the 9P pipe clean.
KFS_RUNTIME_OBJS := $(filter-out $(BUILD_DIR)/obj/boot64-lib/fmt/print.o,$(BOOT64_LIB_OBJS)) \
    $(KFS_PORT_EXTRAS) $(KFS_9SYS_EXTRAS) $(KFS_FMT_EXTRAS) \
    $(KFS_AUTHSRV_OBJS) $(KFS_SEC_OBJS)

KFS_ELF := $(BUILD_DIR)/kfs/kfs.elf
KFS_IMAGE := $(BUILD_DIR)/kfs/kfs.6.out

$(KFS_ELF): $(KFS_SOURCE_OBJS) $(KFS_ENTRY_OBJS) $(KFS_RUNTIME_OBJS) sys/src/9/pc64/boot6.ld
	@mkdir -p "$(dir $@)"
	$(LLD) -m elf_x86_64 -nostdlib --gc-sections \
	    -z max-page-size=0x1000 --build-id=none \
	    -T sys/src/9/pc64/boot6.ld -o "$@" \
	    $(KFS_ENTRY_OBJS) $(KFS_SOURCE_OBJS) $(KFS_RUNTIME_OBJS)

$(KFS_IMAGE): $(KFS_ELF) tools/pc64/elf26out.py
	$(PYTHON) tools/pc64/elf26out.py "$<" "$@"

.PHONY: kfs-amd64 kfs-source-check
kfs-amd64: $(KFS_IMAGE)
	@echo "Original Plan 9 KFS packaged: $(KFS_IMAGE)"

kfs-source-check: $(KFS_SOURCE_OBJS)
	@echo "Original Plan 9 KFS sources: $(words $(KFS_SOURCE_OBJS)) objects compiled"
