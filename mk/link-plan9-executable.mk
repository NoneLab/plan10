# Shared link/packaging implementation for Plan 9 userspace.
# Internal: use BUILD_BOOTSTRAP_EXECUTABLE or BUILD_USER_EXECUTABLE instead.
#
# Both variants produce two files:
#   <module>.elf    - intermediate ELF64 for GDB and link diagnostics
#   <module>.6.out  - REQUIRED native Plan 9 executable

# 1. Validate the module declaration.
ifeq ($(strip $(LOCAL_MODULE)),)
$(error Plan 9 executable requires LOCAL_MODULE)
endif
ifneq ($(strip $(LOCAL_TARGET)),userspace)
$(error $(LOCAL_MODULE): Plan 9 executable requires LOCAL_TARGET := userspace)
endif
ifneq ($(strip $(LOCAL_CRT)),)
ifneq ($(strip $(LOCAL_CRT_OBJECTS)),)
$(error $(LOCAL_MODULE): choose LOCAL_CRT or LOCAL_CRT_OBJECTS, not both)
endif
endif

# Resolve runtime selection before compilation, not only before linking.
# This allows a change in LOCAL_SYSTEM_STATIC_LIBRARIES to switch libc headers.
ifneq ($(filter default,$(LOCAL_SYSTEM_STATIC_LIBRARIES)),)
ifneq ($(words $(LOCAL_SYSTEM_STATIC_LIBRARIES)),1)
$(error $(LOCAL_MODULE): 'default' must be the only system library selector)
endif
endif
ifneq ($(filter libc-%,$(LOCAL_STATIC_LIBRARIES)),)
$(error $(LOCAL_MODULE): put libc modules in LOCAL_SYSTEM_STATIC_LIBRARIES)
endif
_p9_user_sys_libs := $(strip \
    $(if $(filter default,$(LOCAL_SYSTEM_STATIC_LIBRARIES)), \
        $(_p9_default_system_libs), \
        $(LOCAL_SYSTEM_STATIC_LIBRARIES)))

# 2. Compile any sources declared directly on this executable.
#    Existing modules may instead supply LOCAL_EXTRA_OBJECTS.
ifneq ($(strip $(LOCAL_SRC_FILES)),)
ifeq ($(strip $(LOCAL_CFLAGS)),)
LOCAL_CFLAGS := $(BOOT64_USER_CFLAGS)
endif
ifeq ($(strip $(LOCAL_ASFLAGS)),)
LOCAL_ASFLAGS := --target=$(TARGET_TRIPLE) -mno-red-zone
endif
P9_COMPILE_EXTRA_LIBRARIES := $(_p9_user_sys_libs)
include $(BUILD_OBJECTS)
P9_COMPILE_EXTRA_LIBRARIES :=
endif

# 3. Resolve CRT, application objects and dependent libraries.
#    CRT must come first. Archives follow ordinary objects in link order.
_p9_user_crt := $(strip \
    $(if $(LOCAL_CRT), \
        $(call p9-resolve-crt,$(LOCAL_CRT)), \
        $(LOCAL_CRT_OBJECTS)))
_p9_user_objects := $(strip \
    $(_p9_user_crt) \
    $(LOCAL_BUILT_OBJECTS) \
    $(LOCAL_EXTRA_OBJECTS) \
    $(call p9-resolve-object-libs,$(LOCAL_OBJECT_LIBRARIES),userspace))

ifeq ($(strip $(_p9_user_objects)),)
$(error $(LOCAL_MODULE): no objects to link)
endif

# Application archives and system runtime archives have separate contracts.
# Explicit LOCAL_SYSTEM_STATIC_LIBRARIES replaces the backend default, rather
# than appending another libc. Empty means no implicit runtime archives.
_p9_user_app_libs := $(strip $(LOCAL_STATIC_LIBRARIES))
_p9_user_app_archives := $(call p9-resolve-static-libs,$(_p9_user_app_libs),userspace)
_p9_user_sys_archives := $(call p9-resolve-static-libs,$(_p9_user_sys_libs),userspace)
_p9_user_archives := $(strip $(_p9_user_app_archives) $(_p9_user_sys_archives))

# 4. Keep existing paths stable: KFS packaging and embedding depend on them.
_p9_user_dir  := $(if $(LOCAL_MODULE_PATH),$(LOCAL_MODULE_PATH),$(BUILD_DIR)/userspace/bin)
_p9_user_name := $(if $(LOCAL_MODULE_FILENAME),$(LOCAL_MODULE_FILENAME),$(LOCAL_MODULE))
_p9_user_elf  := $(_p9_user_dir)/$(_p9_user_name).elf
_p9_user_6out := $(_p9_user_dir)/$(_p9_user_name).6.out
_p9_user_ldscript := sys/src/9/pc64/boot6.ld

# 5. Capture options per output: later CLEAR_VARS calls must not affect us.
$(_p9_user_elf): private _P9_USER_LINKER := $(if $(LOCAL_LINKER),$(LOCAL_LINKER),$(LLD))
$(_p9_user_elf): private _P9_USER_LDFLAGS := $(if $(LOCAL_LDFLAGS),$(LOCAL_LDFLAGS),-m elf_x86_64 -nostdlib --gc-sections -z max-page-size=0x1000 --build-id=none)
$(_p9_user_elf): private _P9_USER_LDSCRIPT := $(_p9_user_ldscript)
$(_p9_user_elf): private _P9_USER_OBJECTS := $(_p9_user_objects)
$(_p9_user_elf): private _P9_USER_ARCHIVES := $(_p9_user_archives)

$(_p9_user_elf): $(_p9_user_objects) $(_p9_user_archives) $(_p9_user_ldscript)
	@mkdir -p "$(dir $@)"
	$(_P9_USER_LINKER) $(_P9_USER_LDFLAGS) \
	    -T "$(_P9_USER_LDSCRIPT)" -o "$@" \
	    $(_P9_USER_OBJECTS) $(_P9_USER_ARCHIVES)

$(_p9_user_6out): $(_p9_user_elf) tools/pc64/elf26out.py
	$(PYTHON) tools/pc64/elf26out.py "$<" "$@"

# Consumers obtain both outputs from the public backend, not these internals.
LOCAL_BUILT_ELF := $(_p9_user_elf)
LOCAL_BUILT_MODULE := $(_p9_user_6out)

# Packaging metadata is opt-in: native boot and KFS remain kernel-embedded.
# Regular userspace and selected bootstrap executables install into staging.
_p9_install_default_name := $(_p9_user_name)
_p9_install_default_mode := 0755
include mk/register-install.mk
