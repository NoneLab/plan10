# BUILD_INSTALL_DATA
# Register one existing source file in an install image without compiling it.
# Data installation uses the same KFS staging registry as native executables.

ifeq ($(strip $(LOCAL_MODULE)),)
$(error BUILD_INSTALL_DATA requires LOCAL_MODULE)
endif
ifneq ($(strip $(LOCAL_SRC_FILES)),)
ifneq ($(words $(LOCAL_SRC_FILES)),1)
$(error $(LOCAL_MODULE): BUILD_INSTALL_DATA accepts exactly one source file)
endif
else
$(error $(LOCAL_MODULE): BUILD_INSTALL_DATA requires LOCAL_SRC_FILES)
endif
ifneq ($(strip $(LOCAL_INSTALL_IMAGE)),kfs)
$(error $(LOCAL_MODULE): BUILD_INSTALL_DATA requires LOCAL_INSTALL_IMAGE := kfs)
endif

_p9_data_srcdir := $(if $(LOCAL_SRC_DIR),$(LOCAL_SRC_DIR),$(LOCAL_PATH))
LOCAL_BUILT_MODULE := $(_p9_data_srcdir)/$(LOCAL_SRC_FILES)
_p9_install_default_name := $(notdir $(LOCAL_SRC_FILES))
_p9_install_default_mode := 0644
include mk/register-install.mk
