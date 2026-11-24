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

# The native 6.out entry and Plan 9 libc are shared with KFS, not reimplemented.
INIT_ENTRY_OBJS := $(KFS_ENTRY_OBJS)

INIT_RUNTIME_OBJS := $(BOOT64_LIB_OBJS) $(KFS_PORT_EXTRAS) \
    $(KFS_9SYS_EXTRAS) $(KFS_FMT_EXTRAS) $(KFS_AUTHSRV_OBJS) \
    $(KFS_SEC_OBJS) $(INIT_AUTH_OBJS) $(INIT_PORT_EXTRAS) $(INIT_9SYS_EXTRAS)

INIT_ELF := $(BUILD_DIR)/init/init.elf
INIT_NATIVE_IMAGE := $(BUILD_DIR)/init/init.6.out

$(INIT_ELF): $(INIT_ENTRY_OBJS) $(INIT_SOURCE_OBJS) $(INIT_RUNTIME_OBJS) sys/src/9/pc64/boot6.ld
	@mkdir -p "$(dir $@)"
	$(LLD) -m elf_x86_64 -nostdlib --gc-sections \
	    -z max-page-size=0x1000 --build-id=none \
	    -T sys/src/9/pc64/boot6.ld -o "$@" \
	    $(INIT_ENTRY_OBJS) $(INIT_SOURCE_OBJS) $(INIT_RUNTIME_OBJS)

$(INIT_NATIVE_IMAGE): $(INIT_ELF) tools/pc64/elf26out.py
	$(PYTHON) tools/pc64/elf26out.py "$<" "$@"

.PHONY: init-amd64 init-source-check
init-amd64: $(INIT_NATIVE_IMAGE)
	@echo "Original Plan 9 init packaged: $(INIT_NATIVE_IMAGE)"
init-source-check: $(INIT_SOURCE_OBJS)
	@echo "Original Plan 9 init source compiled"

.PHONY: init-native-check
init-native-check: $(INIT_NATIVE_IMAGE) tests/amd64/init64_native.py
	$(PYTHON) tests/amd64/init64_native.py "$(INIT_ELF)" "$(INIT_NATIVE_IMAGE)"

check: init-native-check