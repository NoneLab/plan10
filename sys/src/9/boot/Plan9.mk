# Plan 9 module: sys/src/9/boot
# Loaded by the root build; paths are relative to the repository root.
# Module declarations, object lists, and owned targets live here.

# ---------------------------------------------------------------------------
# U3.2 native boot user-program source builds (original Plan 9 boot source)
#
# This is deliberately NOT a fake linked boot executable. The original
# boot modules still require the real AMD64 user libc and authentication,
# storage and network libraries for a complete link.
# ---------------------------------------------------------------------------

# Native user ABI flags are shared with libc and declared by pc64.

# Plan 9 source-owned module definitions (Plan9.mk-style, GNU Make backend).
# Parsed by GNU Make; not compatible with AOSP build/core or Kati.
LOCAL_PATH := $(call my-dir)

# boot64-source
include $(CLEAR_VARS)
LOCAL_MODULE := boot64-source
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/boot64
LOCAL_SRC_FILES := \
    bootauth.c aux.c boot.c bootcache.c \
    bootip.c local.c embed.c settime.c \
    paq.c parts.c printstub.c usb.c
LOCAL_CFLAGS := $(BOOT64_USER_CFLAGS)
include $(BUILD_OBJECTS)
BOOT64_SOURCE_OBJS := $(LOCAL_BUILT_OBJECTS)

# boot64-config: bootconf.c defines main, platform config and argv0.
# _start is supplied by the default userspace CRT, not bootstart.S.
include $(CLEAR_VARS)
LOCAL_MODULE := boot64-config
LOCAL_TARGET := userspace
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/boot64
LOCAL_SRC_DIR := sys/src/9/pc64
LOCAL_SRC_FILES := bootconf.c
LOCAL_CFLAGS := $(BOOT64_USER_CFLAGS)
include $(BUILD_OBJECTS)
BOOT64_CONFIG_OBJS := $(LOCAL_BUILT_OBJECTS)

.PHONY: boot64-source-check boot64-deps boot64-package
boot64-source-check: $(BOOT64_SOURCE_OBJS) $(BOOT64_CONFIG_OBJS) $(CRT0_AMD64_OBJS)
	@echo "Original Plan 9 boot source compiled: $(words $(BOOT64_SOURCE_OBJS)) modules, plus boot config and shared CRT"
	@echo "This is NOT a complete linked boot program or a QEMU execution test"

check: boot64-source-check

boot64-deps: boot64-source-check
	@mkdir -p "$(BUILD_DIR)/audit"
	$(LLD) -m elf_x86_64 -r $(CRT0_AMD64_OBJS) $(BOOT64_SOURCE_OBJS) $(BOOT64_CONFIG_OBJS) \
	    -o "$(BUILD_DIR)/audit/boot64-objects.o"
	@nm -u "$(BUILD_DIR)/audit/boot64-objects.o" | sed -n 's/^ *U //p' | sort -u \
	    > "$(BUILD_DIR)/audit/boot64-unresolved.txt"
	@echo "Unresolved symbols requiring native user libc / system libraries:"
	@wc -l < "$(BUILD_DIR)/audit/boot64-unresolved.txt"
	@echo "See $(BUILD_DIR)/audit/boot64-unresolved.txt"

# Convert only an authentic *fully linked* user ELF, never a .o or a
# relocatable output from boot64-deps, into the Plan 9 6.out format.
BOOT64_ELF ?=
boot64-package:
ifeq ($(strip $(BOOT64_ELF)),)
	@echo "ERROR: set BOOT64_ELF to an actual fully linked AMD64 boot ELF" >&2; exit 1
else
	$(PYTHON) tools/pc64/elf26out.py "$(BOOT64_ELF)" "$(BUILD_DIR)/boot64/boot.6.out"
endif

# Authentication, networking and buffered I/O remain outside libc.
# libc objects are provided by the registered libc-plan9.a system archive.
BOOT64_SUPPORT_OBJS := \
    $(BOOT64_AUTH_OBJS) \
    $(BOOT64_IP_OBJS) \
    $(BOOT64_BIO_OBJS)


# Bootstrap executable: the kernel embeds the resulting /boot/boot 6.out.
# The common userspace linker keeps an ELF intermediate for diagnostics.
include $(CLEAR_VARS)
LOCAL_MODULE := boot64-executable
LOCAL_TARGET := userspace
LOCAL_MODULE_PATH := $(BUILD_DIR)/boot64
LOCAL_MODULE_FILENAME := boot
# Common crt0-amd64, plus the pinned Plan 9 libc system archive.
LOCAL_SYSTEM_STATIC_LIBRARIES := libc-plan9
LOCAL_EXTRA_OBJECTS := \
    $(BOOT64_CONFIG_OBJS) \
    $(BOOT64_SOURCE_OBJS) \
    $(BOOT64_SUPPORT_OBJS)
include $(BUILD_BOOTSTRAP_EXECUTABLE)
BOOT64_NATIVE_ELF := $(LOCAL_BUILT_ELF)
BOOT64_NATIVE_IMAGE := $(LOCAL_BUILT_MODULE)

.PHONY: boot64-native
boot64-native: $(BOOT64_NATIVE_IMAGE)
	@echo "Real Plan 9 boot.c linked and packaged: $(BOOT64_NATIVE_IMAGE)"

check: boot64-native-check

.PHONY: boot64-native-check
boot64-native-check: $(BOOT64_NATIVE_IMAGE) tests/amd64/boot64_native.py
	$(PYTHON) tests/amd64/boot64_native.py "$(BOOT64_NATIVE_ELF)" "$(BOOT64_NATIVE_IMAGE)"
