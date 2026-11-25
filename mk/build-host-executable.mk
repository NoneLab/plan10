# BUILD_HOST_EXECUTABLE
# Build a native tool for the development host, never a Plan 9 6.out.

# 1. Validate declaration and compile any local sources with host tools.
ifeq ($(strip $(LOCAL_MODULE)),)
$(error BUILD_HOST_EXECUTABLE requires LOCAL_MODULE)
endif
ifneq ($(strip $(LOCAL_TARGET)),host)
$(error $(LOCAL_MODULE): BUILD_HOST_EXECUTABLE requires LOCAL_TARGET := host)
endif

ifneq ($(strip $(LOCAL_SRC_FILES)),)
# Do not accidentally inherit Plan 9's cross-compiler target flags.
ifeq ($(strip $(LOCAL_C_COMPILER)),)
LOCAL_C_COMPILER := $(if $(HOST_CC),$(HOST_CC),$(CLANG))
endif
ifeq ($(strip $(LOCAL_CXX_COMPILER)),)
LOCAL_CXX_COMPILER := $(if $(HOST_CXX),$(HOST_CXX),$(CLANGPP))
endif
include $(BUILD_OBJECTS)
endif

# 2. Resolve host-only objects, archives and output path.
_p9_host_objects := $(strip \
    $(LOCAL_BUILT_OBJECTS) \
    $(LOCAL_EXTRA_OBJECTS) \
    $(call p9-resolve-object-libs,$(LOCAL_OBJECT_LIBRARIES),host))
ifeq ($(strip $(_p9_host_objects)),)
$(error $(LOCAL_MODULE): host executable has no objects)
endif

_p9_host_archives := $(call p9-resolve-static-libs,$(LOCAL_STATIC_LIBRARIES),host)
_p9_host_dir := $(if $(LOCAL_MODULE_PATH),$(LOCAL_MODULE_PATH),$(BUILD_DIR)/host/bin)
_p9_host_name := $(if $(LOCAL_MODULE_FILENAME),$(LOCAL_MODULE_FILENAME),$(LOCAL_MODULE))
_p9_host_output := $(_p9_host_dir)/$(_p9_host_name)

# 3. Freeze link configuration to protect against subsequent CLEAR_VARS.
$(_p9_host_output): private _P9_HOST_LINKER := $(if $(LOCAL_LINKER),$(LOCAL_LINKER),$(if $(HOST_CXX),$(HOST_CXX),$(CLANGPP)))
$(_p9_host_output): private _P9_HOST_LDFLAGS := $(LOCAL_LDFLAGS)
$(_p9_host_output): private _P9_HOST_OBJECTS := $(_p9_host_objects)
$(_p9_host_output): private _P9_HOST_ARCHIVES := $(_p9_host_archives)

$(_p9_host_output): $(_p9_host_objects) $(_p9_host_archives)
	@mkdir -p "$(dir $@)"
	$(_P9_HOST_LINKER) $(_P9_HOST_LDFLAGS) \
	    $(_P9_HOST_OBJECTS) $(_P9_HOST_ARCHIVES) -o "$@"

LOCAL_BUILT_MODULE := $(_p9_host_output)
