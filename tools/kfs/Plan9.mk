# U3.5 — Host-only C++17 KFS image tool; not built for Plan 9 userspace.
# /boot/boot and /boot/kfs remain embedded to bootstrap the KFS root.
HOST_CXX ?= clang++
HOST_CXXFLAGS ?= -std=c++17 -O2 -g -Wall -Wextra -Wpedantic

KFS_HOST_DIR := $(BUILD_DIR)/host/obj/kfs
KFS_HOST_BIN := $(BUILD_DIR)/host/bin/mkbootfs.kfs
KFS_HOST_OBJECTS := $(KFS_HOST_DIR)/mkbootfs.o $(KFS_HOST_DIR)/kfs_image.o
KFS_HOST_TEST_BIN := $(BUILD_DIR)/host/bin/test_mkbootfs.kfs
KFS_BOOTFS_IMAGE ?= $(BUILD_DIR)/images/kfs-root.raw
KFS_BOOTFS_SIZE ?= 64M

$(KFS_HOST_DIR)/%.o: tools/kfs/%.cpp tools/kfs/kfs_image.hpp
	@mkdir -p "$(dir $@)"
	$(HOST_CXX) $(HOST_CXXFLAGS) -MMD -MP -c "$<" -o "$@"
-include $(KFS_HOST_OBJECTS:.o=.d)

$(KFS_HOST_BIN): $(KFS_HOST_OBJECTS)
	@mkdir -p "$(dir $@)"
	$(HOST_CXX) $(HOST_CXXFLAGS) $^ -o "$@"

$(KFS_HOST_TEST_BIN): $(KFS_HOST_DIR)/test_mkbootfs.o $(KFS_HOST_DIR)/kfs_image.o
	@mkdir -p "$(dir $@)"
	$(HOST_CXX) $(HOST_CXXFLAGS) $^ -o "$@"

# Build from real native 6.out programs, then verify byte-for-byte read-back.
# Do not point KFS_BOOTFS_IMAGE to a user's persistent, modified disk.
$(KFS_BOOTFS_IMAGE): $(KFS_HOST_BIN) $(INIT_IMAGE) $(RC_IMAGE) rc/lib/rcmain
	@mkdir -p "$(dir $@)"
	$(KFS_HOST_BIN) create --force --output "$@" --size "$(KFS_BOOTFS_SIZE)" \
	    --dir /tmp --dir /adm/timezone --dir /lib \
	    --file "$(INIT_IMAGE):/amd64/init" --executable /amd64/init \
	    --file "$(RC_IMAGE):/bin/rc" --executable /bin/rc \
	    --file "rc/lib/rcmain:/rc/lib/rcmain"

.PHONY: kfs-bootfs-image kfs-bootfs-check kfs-bootfs-selftest
kfs-bootfs-image: $(KFS_BOOTFS_IMAGE)

kfs-bootfs-check: $(KFS_BOOTFS_IMAGE)
	$(KFS_HOST_BIN) verify --image "$(KFS_BOOTFS_IMAGE)" \
	    --file "$(INIT_IMAGE):/amd64/init" \
	    --file "$(RC_IMAGE):/bin/rc" \
	    --file "rc/lib/rcmain:/rc/lib/rcmain" --list

kfs-bootfs-selftest: $(KFS_HOST_TEST_BIN)
	$(KFS_HOST_TEST_BIN)
