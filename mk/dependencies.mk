# Module dependency resolver shared by the executable backends.
#
# Registries are populated at parse time. Declare providers before consumers.
# Archive link order is consumer-first, provider-second.

# $(call p9-resolve-static-libs,names,target)
#   names  : space-separated static-library module names
#   target : kernel, userspace, or host
#
# The third argument of the recursive helper is the visited-module chain.
# It is private to the resolver and used to reject dependency cycles.
define p9-resolve-static-one
$(if $(filter $(1),$(3)),\
    $(error Static library cycle: $(strip $(3) $(1))))
$(if $(P9_ARCHIVE.$(1)),,\
    $(error Unknown static library: $(1)))
$(if $(filter $(2),$(P9_ARCHIVE_TARGET.$(1))),,\
    $(error Static library $(1) target mismatch: $(P9_ARCHIVE_TARGET.$(1)) vs $(2)))
$(P9_ARCHIVE.$(1)) \
$(foreach dependency,$(P9_ARCHIVE_DEPS.$(1)),\
    $(call p9-resolve-static-one,$(dependency),$(2),$(strip $(3) $(1))))
endef

p9-resolve-static-libs = $(strip \
    $(foreach library,$(1),\
        $(call p9-resolve-static-one,$(library),$(2),)))

# $(call p9-resolve-object-libs,names,target)
# A CRT is a registered object module. Its _start object is linked directly.
define p9-resolve-object-one
$(if $(P9_OBJECT_MODULE.$(1)),,\
    $(error Unknown object module $(1)))
$(if $(filter $(2),$(P9_OBJECT_TARGET.$(1))),,\
    $(error Object module $(1) target mismatch: $(P9_OBJECT_TARGET.$(1)) vs $(2)))
$(P9_OBJECT_MODULE.$(1))
endef

p9-resolve-object-libs = $(strip \
    $(foreach object,$(1),\
        $(call p9-resolve-object-one,$(object),$(2))))

# Semantic alias used by the userspace linker.
p9-resolve-crt = $(call p9-resolve-object-one,$(1),userspace)
