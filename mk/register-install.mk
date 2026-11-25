# Register a KFS installation without copying anything at parse time.
# Internal: executable/data backends include this after LOCAL_BUILT_MODULE.
# Only explicitly installed modules appear in the generated root filesystem.

ifneq ($(strip $(LOCAL_INSTALL_IMAGE)),)
ifneq ($(LOCAL_INSTALL_IMAGE),kfs)
$(error $(LOCAL_MODULE): unsupported LOCAL_INSTALL_IMAGE '$(LOCAL_INSTALL_IMAGE)')
endif
ifeq ($(strip $(LOCAL_INSTALL_PATH)),)
$(error $(LOCAL_MODULE): LOCAL_INSTALL_PATH required for KFS install)
endif
ifeq ($(filter /%,$(LOCAL_INSTALL_PATH)),)
$(error $(LOCAL_MODULE): LOCAL_INSTALL_PATH must be absolute)
endif
ifeq ($(strip $(LOCAL_BUILT_MODULE)),)
$(error $(LOCAL_MODULE): no source file to install)
endif

_p9_install_name := $(if $(LOCAL_INSTALL_FILENAME),$(LOCAL_INSTALL_FILENAME),$(_p9_install_default_name))
_p9_install_mode := $(if $(LOCAL_INSTALL_MODE),$(LOCAL_INSTALL_MODE),$(_p9_install_default_mode))
_p9_guest_path := $(patsubst %/,%,$(LOCAL_INSTALL_PATH))/$(_p9_install_name)

# Expand immediately: CLEAR_VARS and the following module must not change
# previously registered file paths or modes. RootFS staging consumes these.
P9_KFS_INSTALL_FILES := $(strip $(P9_KFS_INSTALL_FILES) \
    $(LOCAL_BUILT_MODULE):$(_p9_guest_path):$(_p9_install_mode))
P9_KFS_INSTALL_SOURCES := $(strip $(P9_KFS_INSTALL_SOURCES) $(LOCAL_BUILT_MODULE))
endif
