# BUILD_STATIC_LIBRARY
# One archive rule for kernel, userspace and host modules.
# The ar operation is common; compiled objects and dependencies remain
# target-specific. Never mix objects built for different ABIs.

# 1. Validate declaration and set compilation defaults for this target.
ifeq ($(strip $(LOCAL_MODULE)),)
$(error BUILD_STATIC_LIBRARY requires LOCAL_MODULE)
endif
ifeq ($(filter $(LOCAL_TARGET),kernel userspace host),)
$(error $(LOCAL_MODULE): static library requires kernel, userspace or host target)
endif
ifneq ($(strip $(LOCAL_CRT) $(LOCAL_CRT_OBJECTS)),)
$(error $(LOCAL_MODULE): static library cannot select a CRT)
endif
ifneq ($(strip $(LOCAL_SYSTEM_STATIC_LIBRARIES)),default)
$(error $(LOCAL_MODULE): system static libraries are executable-only)
endif

ifneq ($(strip $(LOCAL_SRC_FILES)),)
ifeq ($(LOCAL_TARGET),userspace)
ifeq ($(strip $(LOCAL_CFLAGS)),)
LOCAL_CFLAGS := $(BOOT64_USER_CFLAGS)
endif
else ifeq ($(LOCAL_TARGET),kernel)
ifeq ($(strip $(LOCAL_CFLAGS)),)
LOCAL_CFLAGS := $(ARCH_CFLAGS) $(ARCH_CPPFLAGS)
endif
else ifeq ($(LOCAL_TARGET),host)
LOCAL_C_COMPILER := $(if $(LOCAL_C_COMPILER),$(LOCAL_C_COMPILER),$(if $(HOST_CC),$(HOST_CC),$(CLANG)))
LOCAL_CXX_COMPILER := $(if $(LOCAL_CXX_COMPILER),$(LOCAL_CXX_COMPILER),$(if $(HOST_CXX),$(HOST_CXX),$(CLANGPP)))
endif
include $(BUILD_OBJECTS)
endif

# 2. Collect only this module's objects (not its dependency archives).
ifeq ($(strip $(LOCAL_BUILT_OBJECTS)),)
$(error $(LOCAL_MODULE): static library needs sources or object files)
endif
_p9_archive_name := $(LOCAL_MODULE)
_p9_archive_target := $(LOCAL_TARGET)
_p9_archive_objects := $(strip $(LOCAL_BUILT_OBJECTS))
_p9_archive_dependencies := $(strip $(LOCAL_STATIC_LIBRARIES))
_p9_archive_dir := $(if $(LOCAL_MODULE_PATH),$(LOCAL_MODULE_PATH),$(BUILD_DIR)/$(_p9_archive_target)/lib)
_p9_archive_filename := $(if $(LOCAL_MODULE_FILENAME),$(LOCAL_MODULE_FILENAME),$(LOCAL_MODULE).a)
_p9_archive_output := $(_p9_archive_dir)/$(_p9_archive_filename)

# 3. Register the archive and verify its dependencies use the same target.
ifneq ($(strip $(P9_ARCHIVE.$(_p9_archive_name))),)
$(error Duplicate static library $(_p9_archive_name))
endif
_p9_archive_sanity := $(call p9-resolve-static-libs,$(_p9_archive_dependencies),$(_p9_archive_target))

P9_ARCHIVE.$(_p9_archive_name) := $(_p9_archive_output)
P9_ARCHIVE_TARGET.$(_p9_archive_name) := $(_p9_archive_target)
P9_ARCHIVE_DEPS.$(_p9_archive_name) := $(_p9_archive_dependencies)

# 4. Build .a from direct objects. Always recreate to drop removed members.
$(_p9_archive_output): $(_p9_archive_objects)
	@mkdir -p "$(dir $@)"
	@rm -f "$@"
	$(AR) rcs "$@" $^

LOCAL_BUILT_ARCHIVE := $(_p9_archive_output)
