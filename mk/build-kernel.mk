# BUILD_KERNEL
# Link the native pc64 kernel as ELF64 and export its Multiboot image.
# Kernel ELF remains available for GDB; no 6.out conversion is performed.

# 1. Validate declaration and compile optional local sources.
ifeq ($(strip $(LOCAL_MODULE)),)
$(error BUILD_KERNEL requires LOCAL_MODULE)
endif
ifneq ($(strip $(LOCAL_TARGET)),kernel)
$(error $(LOCAL_MODULE): BUILD_KERNEL requires LOCAL_TARGET := kernel)
endif
ifeq ($(strip $(LOCAL_KERNEL_LDSCRIPT)),)
$(error $(LOCAL_MODULE): BUILD_KERNEL requires LOCAL_KERNEL_LDSCRIPT)
endif

ifneq ($(strip $(LOCAL_SRC_FILES)),)
include $(BUILD_OBJECTS)
endif

# 2. Resolve kernel-only objects, archives, outputs and linker script.
_p9_kernel_objects := $(strip \
    $(LOCAL_BUILT_OBJECTS) \
    $(LOCAL_KERNEL_OBJECTS) \
    $(call p9-resolve-object-libs,$(LOCAL_OBJECT_LIBRARIES),kernel))
ifeq ($(strip $(_p9_kernel_objects)),)
$(error $(LOCAL_MODULE): kernel executable has no objects)
endif

_p9_kernel_archives := $(call p9-resolve-static-libs,$(LOCAL_STATIC_LIBRARIES),kernel)
_p9_kernel_elf := $(if $(LOCAL_KERNEL_ELF),$(LOCAL_KERNEL_ELF),$(BUILD_DIR)/$(LOCAL_MODULE).elf)
_p9_kernel_mb := $(if $(LOCAL_KERNEL_MB),$(LOCAL_KERNEL_MB),$(BUILD_DIR)/$(LOCAL_MODULE).mb)
_p9_kernel_script := $(LOCAL_KERNEL_LDSCRIPT)

# 3. Freeze the current module's link options and produce its two outputs.
$(_p9_kernel_elf): private _P9_KERNEL_LINKER := $(if $(LOCAL_LINKER),$(LOCAL_LINKER),$(LLD))
$(_p9_kernel_elf): private _P9_KERNEL_LDFLAGS := $(if $(LOCAL_LDFLAGS),$(LOCAL_LDFLAGS),-m elf_x86_64 -nostdlib -z max-page-size=0x1000)
$(_p9_kernel_elf): private _P9_KERNEL_SCRIPT := $(_p9_kernel_script)
$(_p9_kernel_elf): private _P9_KERNEL_OBJECTS := $(_p9_kernel_objects)
$(_p9_kernel_elf): private _P9_KERNEL_ARCHIVES := $(_p9_kernel_archives)

$(_p9_kernel_elf): $(_p9_kernel_objects) $(_p9_kernel_archives) $(_p9_kernel_script)
	@mkdir -p "$(dir $@)"
	$(_P9_KERNEL_LINKER) $(_P9_KERNEL_LDFLAGS) \
	    -T "$(_P9_KERNEL_SCRIPT)" -o "$@" \
	    $(_P9_KERNEL_OBJECTS) $(_P9_KERNEL_ARCHIVES)

$(_p9_kernel_mb): $(_p9_kernel_elf)
	@mkdir -p "$(dir $@)"
	$(OBJCOPY) -O binary "$<" "$@"

LOCAL_BUILT_ELF := $(_p9_kernel_elf)
LOCAL_BUILT_MODULE := $(_p9_kernel_mb)
