# Plan 9 module: sys/src/9/port
# Loaded by the root build; paths are relative to the repository root.
# Module declarations, object lists, and owned targets live here.

# Kernel /port objects linked into pc64
# Preserve the original pc/mkfile core /port selection.  Device selection
# lists only drivers whose machine dependencies are actually linked.
PORT_KERNEL_SOURCES := \
    alarm.c alloc.c allocb.c auth.c \
    cache.c chan.c dev.c edf.c \
    fault.c latin1.c page.c parse.c \
    pgrp.c portclock.c print.c proc.c \
    qio.c qlock.c rdb.c rebootcmd.c \
    segment.c swap.c sysfile.c sysproc.c \
    taslock.c tod.c watermarks.c xalloc.c \
    devcons.c devenv.c devmnt.c devpipe.c \
    devsrv.c devdup.c devroot.c devwd.c \
    devkprof.c devproc.c devsd.c mul64fract.c random.c

PORT_KERNEL_OBJS := $(addprefix $(BUILD_DIR)/obj/port/,$(PORT_KERNEL_SOURCES:.c=.o))

# Additional /port compilation checks
# ---------------------------------------------------------------------------
# Machine-independent source smoke builds
#
# These are genuine Plan 9 /port source files, but they do not yet constitute
# an executable kernel.  More translation units will join this list as their
# anonymous embeddings and pointer conversions are fixed.
# ---------------------------------------------------------------------------

# The 28 mandatory original /port units are also compiled by check, not
# merely accepted by a syntax-only audit.  Additional port/device sources
# are covered as independent Clang smoke-build targets.
PORT_EXTRA_SOURCES := \
    devdup.c devkbin.c devkbmap.c devkprof.c \
    devroot.c devssl.c devtls.c fpi.c \
    fpimem.c initcode.c mul64fract.c syscallfmt.c \
    thwack.c unthwack.c

# Compile the union of kernel and additional smoke-test sources.
# Link only PORT_KERNEL_OBJS in the original machine-independent order.
LOCAL_PATH := $(call my-dir)

# port-kernel
include $(CLEAR_VARS)
LOCAL_MODULE := port-kernel
LOCAL_OBJ_DIR := $(BUILD_DIR)/obj/port
LOCAL_SRC_FILES := $(sort $(PORT_KERNEL_SOURCES) $(PORT_EXTRA_SOURCES))
LOCAL_CFLAGS := $(ARCH_CFLAGS) $(ARCH_CPPFLAGS)
LOCAL_CXXFLAGS := $(ARCH_CFLAGS) $(ARCH_CPPFLAGS)
include $(BUILD_OBJECTS)
PORT_SMOKE_OBJS := $(LOCAL_BUILT_OBJECTS)

# ---------------------------------------------------------------------------
# Object compilation and ABI validation
# ---------------------------------------------------------------------------

# Clang emits a .d file beside each .o.  Dependencies are optional during
# a fresh build, but header edits trigger recompilation afterward.
# Generated error strings must exist before compiling original /port units.
$(PORT_SMOKE_OBJS): | $(BUILD_DIR)/generated/errstr.h

.PHONY: port-smoke port-audit

port-smoke: $(PORT_SMOKE_OBJS)
	@echo "Plan 9 machine-independent smoke objects: $(words $(PORT_SMOKE_OBJS)) compiled"

port-audit: $(BUILD_DIR)/generated/errstr.h
	$(PYTHON) sys/src/9/pc64/audit.py \
		--clang "$(CLANG)" \
		--output "$(BUILD_DIR)/audit"
