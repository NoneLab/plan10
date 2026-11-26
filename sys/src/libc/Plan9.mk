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
LOCAL_TARGET := kernel
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
LOCAL_TARGET := kernel
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
LOCAL_TARGET := kernel
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
LOCAL_TARGET := userspace
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
LOCAL_TARGET := userspace
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
LOCAL_TARGET := userspace
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
LOCAL_TARGET := userspace
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
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/init/libc-port
LOCAL_SRC_DIR := sys/src/libc/port
LOCAL_SRC_FILES := atnotify.c strpbrk.c netmkaddr.c
LOCAL_CFLAGS := $(BOOT64_USER_CFLAGS)
include $(BUILD_OBJECTS)
INIT_PORT_EXTRAS := $(LOCAL_BUILT_OBJECTS)

include $(CLEAR_VARS)
LOCAL_MODULE := init-libc-9sys
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/init/libc-9sys
LOCAL_SRC_DIR := sys/src/libc/9sys
LOCAL_SRC_FILES := postnote.c getwd.c
LOCAL_CFLAGS := $(BOOT64_USER_CFLAGS)
include $(BUILD_OBJECTS)
INIT_9SYS_EXTRAS := $(LOCAL_BUILT_OBJECTS)

# ---------------------------------------------------------------------------
# Native AMD64 Plan 9 libc: a single userspace archive for boot, kfs, init,
# rc and future commands. The manifest includes functions historically
# compiled under each executable's directory; the archive linker extracts
# only the required members.
#
# KFS defines its own print() and rc defines its own floating-point formatter.
# Static archive extraction retains those original overrides without passing
# every libc object to the linker unconditionally.
# ---------------------------------------------------------------------------

# KFS requires additional libc functions beyond the original boot subset.
include $(CLEAR_VARS)
LOCAL_MODULE := kfs-libc-port
LOCAL_TARGET := userspace
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
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/kfs/libc-9sys
LOCAL_SRC_DIR := sys/src/libc/9sys
LOCAL_SRC_FILES := nsec.c time.c announce.c truerand.c fcallfmt.c
LOCAL_CFLAGS := $(BOOT64_USER_CFLAGS)
include $(BUILD_OBJECTS)
KFS_9SYS_EXTRAS := $(LOCAL_BUILT_OBJECTS)

include $(CLEAR_VARS)
LOCAL_MODULE := kfs-libc-fmt
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/kfs/libc-fmt
LOCAL_SRC_DIR := sys/src/libc/fmt
LOCAL_SRC_FILES := fmtlock.c fltfmt.c
LOCAL_CFLAGS := $(BOOT64_USER_CFLAGS)
include $(BUILD_OBJECTS)
KFS_FMT_EXTRAS := $(LOCAL_BUILT_OBJECTS)


include $(CLEAR_VARS)
LOCAL_MODULE := rc-libc-port
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/rc/libc-port
LOCAL_SRC_DIR := sys/src/libc/port
LOCAL_SRC_FILES := utfutf.c needsrcquote.c
LOCAL_CFLAGS := $(BOOT64_USER_CFLAGS)
include $(BUILD_OBJECTS)
RC_PORT_OBJS := $(LOCAL_BUILT_OBJECTS)


# All Plan 9 userspace libc objects share one ABI and use the same original
# compilation flags. Keep their original output paths during the transition.
PLAN9_USER_LIBC_OBJS := \
    $(BOOT64_PORT_OBJS) \
    $(BOOT64_9SYS_OBJS) \
    $(BOOT64_FMT_OBJS) \
    $(BOOT64_RUNTIME_OBJS) \
    $(KFS_PORT_EXTRAS) \
    $(KFS_9SYS_EXTRAS) \
    $(KFS_FMT_EXTRAS) \
    $(INIT_PORT_EXTRAS) \
    $(INIT_9SYS_EXTRAS) \
    $(RC_PORT_OBJS)

# Archive already-registered object modules: BUILD_STATIC_LIBRARY does not
# compile these sources again or merge transitive dependencies into the .a.
include $(CLEAR_VARS)
LOCAL_MODULE := libc-plan9
LOCAL_TARGET := userspace
# Public native Plan 9 ABI headers; selected via system-library dependencies.
LOCAL_EXPORT_C_INCLUDE_DIRS := amd64/include sys/include
LOCAL_BUILT_OBJECTS := $(PLAN9_USER_LIBC_OBJS)
include $(BUILD_STATIC_LIBRARY)
PLAN9_LIBC_ARCHIVE := $(LOCAL_BUILT_ARCHIVE)
