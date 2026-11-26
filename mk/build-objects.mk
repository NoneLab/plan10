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

# Object modules are reusable, including CRTs. Consumers must match ABI.
P9_OBJECT_MODULE.$(LOCAL_MODULE) := $(LOCAL_BUILT_OBJECTS)
P9_OBJECT_TARGET.$(LOCAL_MODULE) := $(if $(LOCAL_TARGET),$(LOCAL_TARGET),legacy)

# Make rules are emitted per-source to support sources in sibling folders
# and to allow mixed .c/.cc/.S lists in one module.
define _p9_emit_c
$(2): private _P9_LOCAL_CFLAGS := $(3)
$(2): private _P9_CC := $(4)
$(2): $(1) $(5) $(6)
	@mkdir -p "$$(dir $$@)"
	$$(_P9_CC) $$(_P9_LOCAL_CFLAGS) -c "$$<" -o "$$@"
endef

define _p9_emit_cxx
$(2): private _P9_LOCAL_CXXFLAGS := $(3)
$(2): private _P9_CXX := $(4)
$(2): $(1) $(5) $(6)
	@mkdir -p "$$(dir $$@)"
	$$(_P9_CXX) $$(_P9_LOCAL_CXXFLAGS) -c "$$<" -o "$$@"
endef

define _p9_emit_asm
$(2): private _P9_LOCAL_ASFLAGS := $(3)
$(2): private _P9_AS := $(4)
$(2): $(1) $(5) $(6)
	@mkdir -p "$$(dir $$@)"
	$$(_P9_AS) $$(_P9_LOCAL_ASFLAGS) -c "$$<" -o "$$@"
endef

# Local and exported paths have higher priority than legacy system -I flags.
# The executable backend supplies its resolved system libc only while compiling
# that executable; regular object/static modules do not implicitly select libc.
_p9_header_validation := $(call p9-validate-header-libs,$(LOCAL_HEADER_LIBRARIES),$(if $(LOCAL_TARGET),$(LOCAL_TARGET),legacy))
_p9_imports := $(strip $(LOCAL_STATIC_LIBRARIES) $(LOCAL_HEADER_LIBRARIES) \
    $(LOCAL_OBJECT_LIBRARIES) $(P9_COMPILE_EXTRA_LIBRARIES))
_p9_header_dirs := $(call p9-unique,$(strip \
    $(LOCAL_C_INCLUDES) $(LOCAL_EXPORT_C_INCLUDE_DIRS) \
    $(call p9-import-dirs,$(_p9_imports),$(if $(LOCAL_TARGET),$(LOCAL_TARGET),legacy))))
_p9_header_flags := $(addprefix -I,$(_p9_header_dirs))
_p9_header_deps := $(strip $(LOCAL_EXPORT_C_INCLUDE_DEPS) $(call p9-import-deps,$(_p9_imports),$(if $(LOCAL_TARGET),$(LOCAL_TARGET),legacy)))

# Keep an option signature so changing a module's -I list recompiles its .o's.
_p9_flags_stamp := $(LOCAL_OBJ_DIR)/.p9-flags-$(LOCAL_MODULE)
$(_p9_flags_stamp): private _P9_OPTIONS := $(strip \
    CC=$(if $(LOCAL_C_COMPILER),$(LOCAL_C_COMPILER),$(CLANG)) \
    CXX=$(if $(LOCAL_CXX_COMPILER),$(LOCAL_CXX_COMPILER),$(CLANGPP)) \
    AS=$(if $(LOCAL_AS_COMPILER),$(LOCAL_AS_COMPILER),$(CLANG)) \
    INCLUDES=$(_p9_header_flags) CFLAGS=$(LOCAL_CFLAGS) \
    CXXFLAGS=$(LOCAL_CXXFLAGS) ASFLAGS=$(LOCAL_ASFLAGS) DEPFLAGS=$(P9_DEPFLAGS))
$(_p9_flags_stamp): p9-force-compile-flags
	@mkdir -p "$(dir $@)"
	@printf '%s\n' '$(call p9-shell-escape,$(_P9_OPTIONS))' > "$@.tmp"
	@cmp -s "$@.tmp" "$@" && rm -f "$@.tmp" || mv -f "$@.tmp" "$@"

# Snapshot options at declaration time: successive modules use CLEAR_VARS.
# Choose a compiler per module; the emitting macros capture it per object.
_p9_cc  := $(if $(LOCAL_C_COMPILER),$(LOCAL_C_COMPILER),$(CLANG))
_p9_cxx := $(if $(LOCAL_CXX_COMPILER),$(LOCAL_CXX_COMPILER),$(CLANGPP))
_p9_as  := $(if $(LOCAL_AS_COMPILER),$(LOCAL_AS_COMPILER),$(CLANG))

$(foreach source,$(filter %.c,$(LOCAL_SRC_FILES)),\
  $(eval $(call _p9_emit_c,$(_p9_src_dir)/$(source),\
    $(LOCAL_OBJ_DIR)/$(basename $(notdir $(source))).o,\
    $(strip $(_p9_header_flags) $(LOCAL_CFLAGS) $(P9_DEPFLAGS)),$(_p9_cc),\
    $(_p9_flags_stamp),$(_p9_header_deps))))

$(foreach source,$(filter %.cc %.cpp %.cxx,$(LOCAL_SRC_FILES)),\
  $(eval $(call _p9_emit_cxx,$(_p9_src_dir)/$(source),\
    $(LOCAL_OBJ_DIR)/$(basename $(notdir $(source))).o,\
    $(strip $(_p9_header_flags) $(LOCAL_CXXFLAGS) $(P9_DEPFLAGS)),$(_p9_cxx),\
    $(_p9_flags_stamp),$(_p9_header_deps))))

$(foreach source,$(filter %.S,$(LOCAL_SRC_FILES)),\
  $(eval $(call _p9_emit_asm,$(_p9_src_dir)/$(source),\
    $(LOCAL_OBJ_DIR)/$(basename $(notdir $(source))).o,\
    $(strip $(_p9_header_flags) $(LOCAL_ASFLAGS) $(P9_DEPFLAGS)),$(_p9_as),\
    $(_p9_flags_stamp),$(_p9_header_deps))))

_p9_unknown_ext := $(filter-out %.c %.cc %.cpp %.cxx %.S,$(LOCAL_SRC_FILES))
ifneq ($(strip $(_p9_unknown_ext)),)
$(error Unsupported source extension in $(LOCAL_MODULE): $(_p9_unknown_ext))
endif

-include $(LOCAL_BUILT_OBJECTS:.o=.d)

# Plain BUILD_OBJECTS modules may export header metadata as providers. Static
# archives register once, after BUILD_OBJECTS is included by their wrapper.
ifneq ($(_p9_archive_compile),1)
_p9_export_kind := objects
include mk/register-exports.mk
endif
