# Original Plan 9 rc shell for the native AMD64 userspace ABI.
# Preserve syn.y and the original interpreter; generated parser files stay in out/.
LOCAL_PATH := $(call my-dir)
RC_SRC_DIR := $(LOCAL_PATH)
BISON ?= bison

RC_GEN_DIR := $(BUILD_DIR)/generated/rc
RC_PARSER_C := $(RC_GEN_DIR)/y.tab.c
RC_PARSER_H := $(RC_GEN_DIR)/x.tab.h

# Plan 9 yacc calls terminal declarations %term; Bison uses %token.
# Generate a compatibility copy rather than modifying the historical syn.y.
$(RC_PARSER_C) $(RC_PARSER_H) &: $(LOCAL_PATH)/syn.y
	@mkdir -p "$(RC_GEN_DIR)"
	sed -E 's/^%term[[:space:]]+/%token /' "$<" > "$(RC_GEN_DIR)/syn.y"
	$(BISON) -y -d -o "$(RC_PARSER_C)" "$(RC_GEN_DIR)/syn.y"
	cp "$(RC_GEN_DIR)/y.tab.h" "$(RC_PARSER_H)"

include $(CLEAR_VARS)
LOCAL_MODULE := rc-amd64
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/rc
LOCAL_SRC_FILES := code.c exec.c getflags.c glob.c havefork.c here.c io.c \
    lex.c pcmd.c pfnc.c plan9.c simple.c subr.c trap.c tree.c var.c
# Original rc.h defines common globals; preserve the historical common ABI.
LOCAL_CFLAGS := $(BOOT64_USER_CFLAGS) -fcommon -I$(RC_GEN_DIR) -I$(LOCAL_PATH)
include $(BUILD_OBJECTS)
RC_SOURCE_OBJS := $(LOCAL_BUILT_OBJECTS)

# All rc objects include rc.h, which includes the generated x.tab.h.
$(RC_SOURCE_OBJS): $(RC_PARSER_H)

# rc owns argv0 in exec.c, so it requires an entry that does not
# declare the KFS/boot start.S argv0 symbol a second time.
include $(CLEAR_VARS)
LOCAL_MODULE := rc-amd64-entry
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/rc
LOCAL_SRC_DIR := sys/src/cmd/rc/amd64
LOCAL_SRC_FILES := start.S
LOCAL_ASFLAGS := --target=$(TARGET_TRIPLE) -mno-red-zone
include $(BUILD_OBJECTS)
RC_ENTRY_OBJS := $(LOCAL_BUILT_OBJECTS)

RC_PARSER_OBJ := $(BUILD_DIR)/obj/rc/y.tab.o
$(RC_PARSER_OBJ): $(RC_PARSER_C) $(RC_PARSER_H)
	@mkdir -p "$(dir $@)"
	$(CLANG) $(BOOT64_USER_CFLAGS) -fcommon -I$(RC_GEN_DIR) -I$(RC_SRC_DIR) -c "$<" -o "$@"

# rc/plan9.c provides _efgfmt itself; the KFS floating formatter would
# otherwise introduce a duplicate definition.
include sys/src/libString/Plan9.mk
include $(CLEAR_VARS)
LOCAL_MODULE := rc-libc-port
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/rc/libc-port
LOCAL_SRC_DIR := sys/src/libc/port
LOCAL_SRC_FILES := utfutf.c needsrcquote.c
LOCAL_CFLAGS := $(BOOT64_USER_CFLAGS)
include $(BUILD_OBJECTS)
RC_PORT_OBJS := $(LOCAL_BUILT_OBJECTS)

RC_RUNTIME_OBJS := $(filter-out $(BUILD_DIR)/obj/kfs/libc-fmt/fltfmt.o,$(INIT_RUNTIME_OBJS)) $(RC_PORT_OBJS) $(RC_STRING_OBJS)
RC_ELF := $(BUILD_DIR)/rc/rc.elf
RC_NATIVE_IMAGE := $(BUILD_DIR)/rc/rc.6.out

$(RC_ELF): $(RC_ENTRY_OBJS) $(RC_SOURCE_OBJS) $(RC_PARSER_OBJ) $(RC_RUNTIME_OBJS) sys/src/9/pc64/boot6.ld
	@mkdir -p "$(dir $@)"
	$(LLD) -m elf_x86_64 -nostdlib --gc-sections \
	    -z max-page-size=0x1000 --build-id=none \
	    -T sys/src/9/pc64/boot6.ld -o "$@" \
	    $(RC_ENTRY_OBJS) $(RC_SOURCE_OBJS) $(RC_PARSER_OBJ) $(RC_RUNTIME_OBJS)

$(RC_NATIVE_IMAGE): $(RC_ELF) tools/pc64/elf26out.py
	$(PYTHON) tools/pc64/elf26out.py "$<" "$@"

.PHONY: rc-amd64 rc-source-check
rc-amd64: $(RC_NATIVE_IMAGE)
	@echo "Original Plan 9 rc packaged: $(RC_NATIVE_IMAGE)"
rc-source-check: $(RC_SOURCE_OBJS) $(RC_PARSER_OBJ)
	@echo "Original Plan 9 rc source and yacc parser compiled"

.PHONY: rc-native-check
rc-native-check: $(RC_NATIVE_IMAGE) tests/amd64/rc64_native.py
	$(PYTHON) tests/amd64/rc64_native.py "$(RC_ELF)" "$(RC_NATIVE_IMAGE)"

check: rc-native-check