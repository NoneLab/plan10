# Android.mk-style declarations for the Plan 9 GNU Make build.
# Included once by the root Makefile, before source-local Plan9.mk files.
#
# A source-local Plan9.mk describes WHAT to build using LOCAL_* variables.
# The BUILD_* include selects HOW that module is built.

my-dir = $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))

# Shared module plumbing.
CLEAR_VARS           := mk/clear-vars.mk
BUILD_OBJECTS        := mk/build-objects.mk
BUILD_STATIC_LIBRARY := mk/build-static-library.mk

# Executable policies; the object and archive machinery remains shared.
BUILD_KERNEL                := mk/build-kernel.mk
BUILD_BOOTSTRAP_EXECUTABLE  := mk/build-bootstrap-executable.mk
BUILD_USER_EXECUTABLE       := mk/build-user-executable.mk
BUILD_HOST_EXECUTABLE       := mk/build-host-executable.mk
BUILD_INSTALL_DATA          := mk/build-install-data.mk

# The common native _start is the default for both bootstrap and userspace.
# Specialized applications may override it with LOCAL_CRT.
USERSPACE_DEFAULT_CRT ?= crt0-amd64

# Default runtime archives are backend policy, not an implicit libc module type.
# LOCAL_SYSTEM_STATIC_LIBRARIES may replace these per executable. The explicit
# empty value disables system archives; the sentinel 'default' restores them.
USERSPACE_SYSTEM_STATIC_LIBRARIES ?= libc-plan9
BOOTSTRAP_SYSTEM_STATIC_LIBRARIES ?= libc-plan9

include mk/dependencies.mk
