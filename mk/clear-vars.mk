# Reset per-module declarations between includes. LOCAL_PATH survives because
# a source-local Plan9.mk normally declares several modules in one directory.

# Module identity and sources
LOCAL_MODULE :=
LOCAL_TARGET :=
LOCAL_SRC_FILES :=
LOCAL_SRC_DIR :=
LOCAL_OBJ_DIR :=

# Compiler configuration
LOCAL_CFLAGS :=
LOCAL_CXXFLAGS :=
LOCAL_ASFLAGS :=
LOCAL_C_COMPILER :=
LOCAL_CXX_COMPILER :=
LOCAL_AS_COMPILER :=

# Dependency and runtime selection
LOCAL_OBJECT_LIBRARIES :=
LOCAL_STATIC_LIBRARIES :=
# 'default' selects the appropriate executable backend's runtime archives.
# LOCAL_SYSTEM_STATIC_LIBRARIES := (empty) disables automatic runtime linking.
LOCAL_SYSTEM_STATIC_LIBRARIES := default
LOCAL_EXTRA_OBJECTS :=
LOCAL_CRT :=
LOCAL_CRT_OBJECTS :=

# Optional root filesystem installation (KFS staging).
LOCAL_INSTALL_IMAGE :=
LOCAL_INSTALL_PATH :=
LOCAL_INSTALL_FILENAME :=
LOCAL_INSTALL_MODE :=

# Linker and output paths
LOCAL_LINKER :=
LOCAL_LDFLAGS :=
LOCAL_MODULE_PATH :=
LOCAL_MODULE_FILENAME :=

# Kernel-only options
LOCAL_KERNEL_ELF :=
LOCAL_KERNEL_MB :=
LOCAL_KERNEL_LDSCRIPT :=
LOCAL_KERNEL_OBJECTS :=

# Build results (snapshot before the next CLEAR_VARS)
LOCAL_BUILT_OBJECTS :=
LOCAL_BUILT_ARCHIVE :=
LOCAL_BUILT_ELF :=
LOCAL_BUILT_MODULE :=
