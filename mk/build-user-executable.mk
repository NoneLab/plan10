# BUILD_USER_EXECUTABLE
# Normal Plan 9 userspace program. Final output is always native 6.out.
#
# The common crt0-amd64 is the default entry. Libraries use two independent
# declarations: LOCAL_STATIC_LIBRARIES for application dependencies and
# LOCAL_SYSTEM_STATIC_LIBRARIES for libc/runtime archives.

ifeq ($(strip $(LOCAL_TARGET)),)
LOCAL_TARGET := userspace
endif
ifneq ($(strip $(LOCAL_TARGET)),userspace)
$(error BUILD_USER_EXECUTABLE requires LOCAL_TARGET := userspace)
endif

# A raw LOCAL_CRT_OBJECTS override remains valid for existing modules.
ifeq ($(strip $(LOCAL_CRT) $(LOCAL_CRT_OBJECTS)),)
LOCAL_CRT := $(USERSPACE_DEFAULT_CRT)
endif
ifeq ($(strip $(LOCAL_CRT) $(LOCAL_CRT_OBJECTS)),)
$(error $(LOCAL_MODULE): no default CRT configured)
endif

_p9_default_system_libs := $(USERSPACE_SYSTEM_STATIC_LIBRARIES)

include mk/link-plan9-executable.mk
