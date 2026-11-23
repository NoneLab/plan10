# Shared GNU Make configuration. No component source lists or recipes here.
# Source directories declare LOCAL_* in Plan9.mk; mk/build-objects.mk builds objects.
# Generated, link and run targets remain in their owning Makefiles.
P9_DEPFLAGS ?= -MMD -MP
