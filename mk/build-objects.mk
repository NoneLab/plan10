# Declarative objects backend for Plan 9's Plan9.mk-style modules.
#
# Required: LOCAL_MODULE, LOCAL_PATH, LOCAL_SRC_FILES, LOCAL_OBJ_DIR
# Optional: LOCAL_SRC_DIR (defaults to LOCAL_PATH), LOCAL_CFLAGS,
#           LOCAL_CXXFLAGS, LOCAL_ASFLAGS
# Result:   LOCAL_BUILT_OBJECTS (snapshot it before next CLEAR_VARS)
#
# Sources stay in their original directories; .o/.d stay under out/amd64.
# This module class deliberately does NOT create .a archives or change
# the order of objects passed to the original Plan 9 kernel/boot linkers.

ifeq ($(strip $(LOCAL_MODULE)),)
$(error LOCAL_MODULE is required before include $(BUILD_OBJECTS))
endif
ifeq ($(strip $(LOCAL_PATH)),)
$(error LOCAL_PATH is required in $(LOCAL_MODULE))
endif
ifeq ($(strip $(LOCAL_SRC_FILES)),)
$(error LOCAL_SRC_FILES is required in $(LOCAL_MODULE))
endif
ifeq ($(strip $(LOCAL_OBJ_DIR)),)
$(error LOCAL_OBJ_DIR is required in $(LOCAL_MODULE))
endif
ifneq ($(filter $(LOCAL_MODULE),$(P9_REGISTERED_MODULES)),)
$(error Duplicate LOCAL_MODULE $(LOCAL_MODULE))
endif
P9_REGISTERED_MODULES := $(strip $(P9_REGISTERED_MODULES) $(LOCAL_MODULE))

_p9_src_dir := $(if $(strip $(LOCAL_SRC_DIR)),$(LOCAL_SRC_DIR),$(LOCAL_PATH))
_p9_obj_names := $(addsuffix .o,$(basename $(notdir $(LOCAL_SRC_FILES))))
ifneq ($(words $(sort $(_p9_obj_names))),$(words $(_p9_obj_names)))
$(error Duplicate output basename in $(LOCAL_MODULE): $(_p9_obj_names))
endif
LOCAL_BUILT_OBJECTS := $(addprefix $(LOCAL_OBJ_DIR)/,$(_p9_obj_names))

# Make rules are emitted per-source to support sources in sibling folders
# and to allow mixed .c/.cc/.S lists in one module.
define _p9_emit_c
$(2): private _P9_LOCAL_CFLAGS := $(3)
$(2): $(1)
	@mkdir -p "$$(dir $$@)"
	$$(CLANG) $$(_P9_LOCAL_CFLAGS) -c "$$<" -o "$$@"
endef

define _p9_emit_cxx
$(2): private _P9_LOCAL_CXXFLAGS := $(3)
$(2): $(1)
	@mkdir -p "$$(dir $$@)"
	$$(CLANGPP) $$(_P9_LOCAL_CXXFLAGS) -c "$$<" -o "$$@"
endef

define _p9_emit_asm
$(2): private _P9_LOCAL_ASFLAGS := $(3)
$(2): $(1)
	@mkdir -p "$$(dir $$@)"
	$$(CLANG) $$(_P9_LOCAL_ASFLAGS) -c "$$<" -o "$$@"
endef

# Snapshot options at declaration time: successive modules use CLEAR_VARS.
$(foreach _p9_source,$(filter %.c,$(LOCAL_SRC_FILES)),\
  $(eval $(call _p9_emit_c,$(_p9_src_dir)/$(_p9_source),\
    $(LOCAL_OBJ_DIR)/$(basename $(notdir $(_p9_source))).o,$(LOCAL_CFLAGS))))
$(foreach _p9_source,$(filter %.cc,$(LOCAL_SRC_FILES)),\
  $(eval $(call _p9_emit_cxx,$(_p9_src_dir)/$(_p9_source),\
    $(LOCAL_OBJ_DIR)/$(basename $(notdir $(_p9_source))).o,$(LOCAL_CXXFLAGS))))
$(foreach _p9_source,$(filter %.S,$(LOCAL_SRC_FILES)),\
  $(eval $(call _p9_emit_asm,$(_p9_src_dir)/$(_p9_source),\
    $(LOCAL_OBJ_DIR)/$(basename $(notdir $(_p9_source))).o,$(LOCAL_ASFLAGS))))

_p9_unknown_ext := $(filter-out %.c %.cc %.S,$(LOCAL_SRC_FILES))
ifneq ($(strip $(_p9_unknown_ext)),)
$(error Unsupported source extension in $(LOCAL_MODULE): $(_p9_unknown_ext))
endif

-include $(LOCAL_BUILT_OBJECTS:.o=.d)
