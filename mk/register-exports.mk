# Register current module's public headers and optional re-export edges.
# _p9_export_kind is set by BUILD_OBJECTS, BUILD_STATIC_LIBRARY or
# BUILD_HEADER_LIBRARY. This file must be included exactly once per module.
ifeq ($(strip $(LOCAL_MODULE)),)
$(error Public header registration requires LOCAL_MODULE)
endif
ifneq ($(filter $(LOCAL_MODULE),$(P9_EXPORTED_MODULES)),)
$(error Duplicate header provider $(LOCAL_MODULE))
endif
ifneq ($(filter-out $(LOCAL_STATIC_LIBRARIES),$(LOCAL_EXPORT_STATIC_LIBRARY_HEADERS)),)
$(error $(LOCAL_MODULE): re-exported static libraries must be direct LOCAL_STATIC_LIBRARIES)
endif
ifneq ($(filter-out $(LOCAL_HEADER_LIBRARIES),$(LOCAL_EXPORT_HEADER_LIBRARY_HEADERS)),)
$(error $(LOCAL_MODULE): re-exported header libraries must be direct LOCAL_HEADER_LIBRARIES)
endif

_p9_header_validation := $(call p9-validate-header-libs,$(LOCAL_HEADER_LIBRARIES),$(if $(LOCAL_TARGET),$(LOCAL_TARGET),legacy))
P9_EXPORTED_MODULES := $(strip $(P9_EXPORTED_MODULES) $(LOCAL_MODULE))
P9_EXPORT_KIND.$(LOCAL_MODULE) := $(_p9_export_kind)
P9_EXPORT_TARGET.$(LOCAL_MODULE) := $(if $(strip $(LOCAL_TARGET)),$(LOCAL_TARGET),legacy)
P9_EXPORT_DIRS.$(LOCAL_MODULE) := $(strip $(LOCAL_EXPORT_C_INCLUDE_DIRS))
P9_EXPORT_INCLUDE_DEPS.$(LOCAL_MODULE) := $(strip $(LOCAL_EXPORT_C_INCLUDE_DEPS))
P9_EXPORT_REEXPORT.$(LOCAL_MODULE) := $(strip $(LOCAL_EXPORT_STATIC_LIBRARY_HEADERS) $(LOCAL_EXPORT_HEADER_LIBRARY_HEADERS))

# Validate the re-export graph when each provider is registered. The automatic
# import checks the same ABI again when a later consumer is compiled.
_p9_export_check := $(call p9-import-dirs,$(P9_EXPORT_REEXPORT.$(LOCAL_MODULE)),$(P9_EXPORT_TARGET.$(LOCAL_MODULE)))
_p9_export_check := $(call p9-import-deps,$(P9_EXPORT_REEXPORT.$(LOCAL_MODULE)),$(P9_EXPORT_TARGET.$(LOCAL_MODULE)))
