# U3.5 — Host-only C++17 KFS image tool; not built for Plan 9 userspace.
# /boot/boot and /boot/kfs remain embedded to bootstrap the KFS root.
HOST_CXX ?= clang++
HOST_CXXFLAGS ?= -std=c++17 -O2 -g -Wall -Wextra -Wpedantic

KFS_HOST_DIR := $(BUILD_DIR)/host/obj/kfs
KFS_HOST_BIN := $(BUILD_DIR)/host/bin/mkbootfs.kfs
KFS_HOST_OBJECTS := $(KFS_HOST_DIR)/mkbootfs.o $(KFS_HOST_DIR)/kfs_image.o
KFS_HOST_TEST_BIN := $(BUILD_DIR)/host/bin/test_mkbootfs.kfs
KFS_BOOTFS_IMAGE ?= $(BUILD_DIR)/images/kfs-root.raw
KFS_ROOTFS_DIR ?= $(BUILD_DIR)/rootfs
KFS_BOOTFS_SIZE ?= 64M

# Host object/ELF builds are completely separate from target ABI and 6.out.
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := kfs-host-mkbootfs
LOCAL_TARGET := host
LOCAL_OBJ_DIR := $(KFS_HOST_DIR)
LOCAL_MODULE_PATH := $(BUILD_DIR)/host/bin
LOCAL_MODULE_FILENAME := mkbootfs.kfs
LOCAL_SRC_FILES := mkbootfs.cpp kfs_image.cpp
LOCAL_CXXFLAGS := $(HOST_CXXFLAGS) -MMD -MP
LOCAL_CXX_COMPILER := $(HOST_CXX)
LOCAL_LINKER := $(HOST_CXX)
include $(BUILD_HOST_EXECUTABLE)
KFS_HOST_BIN := $(LOCAL_BUILT_MODULE)
KFS_HOST_OBJECTS := $(LOCAL_BUILT_OBJECTS)

include $(CLEAR_VARS)
LOCAL_MODULE := kfs-host-selftest
LOCAL_TARGET := host
LOCAL_OBJ_DIR := $(KFS_HOST_DIR)/tests
LOCAL_MODULE_PATH := $(BUILD_DIR)/host/bin
LOCAL_MODULE_FILENAME := test_mkbootfs.kfs
LOCAL_SRC_FILES := test_mkbootfs.cpp
LOCAL_EXTRA_OBJECTS := $(KFS_HOST_DIR)/kfs_image.o
LOCAL_CXXFLAGS := $(HOST_CXXFLAGS) -MMD -MP
LOCAL_CXX_COMPILER := $(HOST_CXX)
LOCAL_LINKER := $(HOST_CXX)
include $(BUILD_HOST_EXECUTABLE)
KFS_HOST_TEST_BIN := $(LOCAL_BUILT_MODULE)

# Staging is rebuilt from the current module registry on every invocation.
# This also removes programs that no longer declare LOCAL_INSTALL_IMAGE=kfs.
# The image is regenerated each time, intentionally avoiding separate file
# inventory/mtime bookkeeping. Keep the QEMU disk copy separate from this file.
.PHONY: kfs-rootfs-stage
kfs-rootfs-stage: $(P9_KFS_INSTALL_SOURCES) tools/kfs/stage_rootfs.py
	$(PYTHON) tools/kfs/stage_rootfs.py \
	    --build-dir "$(BUILD_DIR)" --root "$(KFS_ROOTFS_DIR)" \
	    --dir /tmp --dir /adm/timezone --dir /lib \
	    $(foreach file,$(P9_KFS_INSTALL_FILES),--file "$(file)")

$(KFS_BOOTFS_IMAGE): kfs-rootfs-stage $(KFS_HOST_BIN)
	@mkdir -p "$(dir $@)"
	$(KFS_HOST_BIN) create --force --root "$(KFS_ROOTFS_DIR)" \
	    --output "$@" --size "$(KFS_BOOTFS_SIZE)"

.PHONY: kfs-bootfs-image kfs-bootfs-check kfs-bootfs-selftest
kfs-bootfs-image: $(KFS_BOOTFS_IMAGE)

kfs-bootfs-check: $(KFS_BOOTFS_IMAGE)
	$(KFS_HOST_BIN) verify --image "$(KFS_BOOTFS_IMAGE)" --list

kfs-bootfs-selftest: $(KFS_HOST_TEST_BIN) $(KFS_HOST_BIN)
	$(KFS_HOST_TEST_BIN)
	$(PYTHON) tests/kfs/rootfs_stage.py "$(KFS_HOST_BIN)"
