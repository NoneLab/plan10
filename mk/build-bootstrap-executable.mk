# BUILD_BOOTSTRAP_EXECUTABLE
# Early Plan 9 userspace programs: boot, KFS and init.
#
# Bootstrap programs use the common CRT unless LOCAL_CRT overrides it.
# They do not inherit USERSPACE_LIBC=experimental globally.
# Link and ELF -> 6.out conversion are shared with BUILD_USER_EXECUTABLE.

ifeq ($(strip $(LOCAL_TARGET)),)
LOCAL_TARGET := userspace
endif
ifneq ($(strip $(LOCAL_TARGET)),userspace)
$(error BUILD_BOOTSTRAP_EXECUTABLE requires LOCAL_TARGET := userspace)
endif

ifeq ($(strip $(LOCAL_CRT)),)
LOCAL_CRT := $(USERSPACE_DEFAULT_CRT)
endif
ifeq ($(strip $(LOCAL_CRT)),)
$(error $(LOCAL_MODULE): no default CRT configured)
endif
ifneq ($(strip $(LOCAL_CRT_OBJECTS)),)
$(error $(LOCAL_MODULE): bootstrap executable cannot use LOCAL_CRT_OBJECTS)
endif
# Bootstrap stays on the known-good Plan 9 runtime unless explicitly
# overridden by LOCAL_SYSTEM_STATIC_LIBRARIES in the module declaration.
_p9_default_system_libs := $(BOOTSTRAP_SYSTEM_STATIC_LIBRARIES)

include mk/link-plan9-executable.mk
