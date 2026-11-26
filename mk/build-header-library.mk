# BUILD_HEADER_LIBRARY: dependency-only module, no compilation or archive.
ifeq ($(strip $(LOCAL_MODULE)),)
$(error BUILD_HEADER_LIBRARY requires LOCAL_MODULE)
endif
ifeq ($(filter $(LOCAL_TARGET),kernel userspace host),)
$(error $(LOCAL_MODULE): header library requires kernel, userspace or host target)
endif
ifneq ($(strip $(LOCAL_SRC_FILES) $(LOCAL_STATIC_LIBRARIES) $(LOCAL_OBJECT_LIBRARIES) $(LOCAL_EXTRA_OBJECTS) $(LOCAL_CRT) $(LOCAL_CRT_OBJECTS)),)
$(error $(LOCAL_MODULE): header library cannot compile sources or link archives)
endif
ifneq ($(strip $(LOCAL_SYSTEM_STATIC_LIBRARIES)),default)
$(error $(LOCAL_MODULE): header library cannot select system libraries)
endif
ifneq ($(strip $(LOCAL_EXPORT_STATIC_LIBRARY_HEADERS)),)
$(error $(LOCAL_MODULE): header library cannot re-export static libraries)
endif
ifeq ($(strip $(LOCAL_EXPORT_C_INCLUDE_DIRS) $(LOCAL_EXPORT_HEADER_LIBRARY_HEADERS)),)
$(error $(LOCAL_MODULE): header library has no public headers)
endif
ifneq ($(filter $(LOCAL_MODULE),$(P9_REGISTERED_MODULES)),)
$(error Duplicate LOCAL_MODULE $(LOCAL_MODULE))
endif
P9_REGISTERED_MODULES := $(strip $(P9_REGISTERED_MODULES) $(LOCAL_MODULE))
_p9_export_kind := header
include mk/register-exports.mk
