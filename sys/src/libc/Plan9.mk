# Plan 9 module: sys/src/libc
# Loaded by the root build; paths are relative to the repository root.
# Module declarations, object lists, and owned targets live here.

# Module source manifests and ABI-specific object profiles live here.
# kernel and /boot/boot use different flags, hence separate object trees.
# Plan 9 source-owned module definitions (Plan9.mk-style, GNU Make backend).
# Parsed by GNU Make; not compatible with AOSP build/core or Kati.
LOCAL_PATH := $(call my-dir)

# kernel-libc
include $(CLEAR_VARS)
LOCAL_MODULE := kernel-libc
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/libc
LOCAL_SRC_DIR := sys/src/libc/port
LOCAL_SRC_FILES := \
    cleanname.c memccpy.c memchr.c memcmp.c \
    memmove.c memset.c qsort.c rune.c \
    strchr.c strcmp.c strcpy.c strlen.c \
    strncmp.c strncpy.c strrchr.c strtoul.c strtoull.c \
    tokenize.c utflen.c utfnlen.c utfrune.c \
    strecpy.c utfecpy.c strtol.c atol.c \
    pool.c
LOCAL_CFLAGS := $(ARCH_CFLAGS) $(ARCH_CPPFLAGS)
include $(BUILD_OBJECTS)
KERNEL_LIBC_OBJS := $(LOCAL_BUILT_OBJECTS)

# kernel-9sys
include $(CLEAR_VARS)
LOCAL_MODULE := kernel-9sys
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/9sys
LOCAL_SRC_DIR := sys/src/libc/9sys
LOCAL_SRC_FILES := \
    convD2M.c convM2D.c convM2S.c convS2M.c \
    fcallfmt.c
LOCAL_CFLAGS := $(ARCH_CFLAGS) $(ARCH_CPPFLAGS)
include $(BUILD_OBJECTS)
KERNEL_9SYS_OBJS := $(LOCAL_BUILT_OBJECTS)

# kernel-fmt
include $(CLEAR_VARS)
LOCAL_MODULE := kernel-fmt
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/fmt
LOCAL_SRC_DIR := sys/src/libc/fmt
LOCAL_SRC_FILES := \
    dofmt.c dorfmt.c errfmt.c fmt.c \
    fmtprint.c fmtquote.c fmtrune.c sprint.c \
    fmtstr.c fmtvprint.c runefmtstr.c seprint.c \
    snprint.c vseprint.c vsnprint.c
LOCAL_CFLAGS := $(ARCH_CFLAGS) $(ARCH_CPPFLAGS)
include $(BUILD_OBJECTS)
KERNEL_FMT_OBJS := $(LOCAL_BUILT_OBJECTS)

# boot64-libc-port
include $(CLEAR_VARS)
LOCAL_MODULE := boot64-libc-port
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/boot64-lib/port
LOCAL_SRC_DIR := sys/src/libc/port
LOCAL_SRC_FILES := \
    atol.c atoll.c strtoll.c ctype.c \
    atexit.c _assert.c getcallerpc.c getfields.c \
    lock.c malloc.c pool.c memccpy.c \
    memchr.c memcmp.c memmove.c memset.c \
    rune.c runebase.c strcat.c strchr.c \
    strcmp.c strcpy.c strlen.c strncmp.c \
    strncpy.c strrchr.c strstr.c strdup.c \
    strecpy.c strtoul.c strtoull.c tokenize.c \
    utflen.c utfnlen.c utfrune.c utfecpy.c \
    qsort.c
LOCAL_CFLAGS := $(BOOT64_USER_CFLAGS)
include $(BUILD_OBJECTS)
BOOT64_PORT_OBJS := $(LOCAL_BUILT_OBJECTS)

# boot64-libc-9sys
include $(CLEAR_VARS)
LOCAL_MODULE := boot64-libc-9sys
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/boot64-lib/9sys
LOCAL_SRC_DIR := sys/src/libc/9sys
LOCAL_SRC_FILES := \
    access.c getpid.c convM2D.c convD2M.c \
    convM2S.c convS2M.c dirstat.c dirfstat.c \
    dirwstat.c dirfwstat.c dirread.c getenv.c \
    fork.c wait.c waitpid.c rerrstr.c \
    werrstr.c sysfatal.c sbrk.c qlock.c \
    dial.c
LOCAL_CFLAGS := $(BOOT64_USER_CFLAGS)
include $(BUILD_OBJECTS)
BOOT64_9SYS_OBJS := $(LOCAL_BUILT_OBJECTS)

# boot64-libc-fmt
include $(CLEAR_VARS)
LOCAL_MODULE := boot64-libc-fmt
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/boot64-lib/fmt
LOCAL_SRC_DIR := sys/src/libc/fmt
LOCAL_SRC_FILES := \
    fmtfd.c dofmt.c dorfmt.c errfmt.c \
    fmt.c fmtprint.c fmtquote.c fmtrune.c \
    fmtstr.c fmtvprint.c runefmtstr.c seprint.c \
    snprint.c sprint.c vseprint.c vsnprint.c \
    print.c fprint.c vfprint.c smprint.c \
    vsmprint.c
LOCAL_CFLAGS := $(BOOT64_USER_CFLAGS)
include $(BUILD_OBJECTS)
BOOT64_FMT_OBJS := $(LOCAL_BUILT_OBJECTS)

# boot64-libc-runtime
include $(CLEAR_VARS)
LOCAL_MODULE := boot64-libc-runtime
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/boot64-lib/amd64
LOCAL_SRC_DIR := sys/src/libc/amd64
LOCAL_SRC_FILES := \
    bootcalls.c bootruntime.c execl.c
LOCAL_CFLAGS := $(BOOT64_USER_CFLAGS)
include $(BUILD_OBJECTS)
BOOT64_RUNTIME_OBJS := $(LOCAL_BUILT_OBJECTS) $(USER_SYSCALL_OBJ)

.PHONY: kernel-libc
kernel-libc: $(KERNEL_LIBC_OBJS)
	@echo "Original Plan 9 libc port objects: $(words $(KERNEL_LIBC_OBJS)) compiled"

.PHONY: kernel-fmt
kernel-fmt: $(KERNEL_FMT_OBJS)
	@echo "Original Plan 9 formatter objects: $(words $(KERNEL_FMT_OBJS)) compiled"

.PHONY: boot64-libc
boot64-libc: $(BOOT64_PORT_OBJS) $(BOOT64_9SYS_OBJS) $(BOOT64_FMT_OBJS) $(BOOT64_RUNTIME_OBJS)
	@echo "Original Plan 9 boot libc objects: $(words $^) compiled"

include $(CLEAR_VARS)
LOCAL_MODULE := init-libc-port
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/init/libc-port
LOCAL_SRC_DIR := sys/src/libc/port
LOCAL_SRC_FILES := atnotify.c strpbrk.c netmkaddr.c
LOCAL_CFLAGS := $(BOOT64_USER_CFLAGS)
include $(BUILD_OBJECTS)
INIT_PORT_EXTRAS := $(LOCAL_BUILT_OBJECTS)

include $(CLEAR_VARS)
LOCAL_MODULE := init-libc-9sys
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/init/libc-9sys
LOCAL_SRC_DIR := sys/src/libc/9sys
LOCAL_SRC_FILES := postnote.c getwd.c
LOCAL_CFLAGS := $(BOOT64_USER_CFLAGS)
include $(BUILD_OBJECTS)
INIT_9SYS_EXTRAS := $(LOCAL_BUILT_OBJECTS)
