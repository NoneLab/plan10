# Android.mk-style, ABI-scoped public header registry.
# All providers are registered before consumers (the same policy as archives).
# Direct dependencies import their providers' exported paths. Transitive paths
# are visible only through explicit LOCAL_EXPORT_*_LIBRARY_HEADERS re-exports.

# Make's compiler depfiles are emitted for all C/C++/preprocessed assembly.
P9_DEPFLAGS ?= -MMD -MP
# Safely serialize flags into a shell-quoted string (including embedded quotes).
p9-sq := '
p9-shell-escape = $(subst $(p9-sq),'"'"',$(1))

# Stable order is significant: LOCAL_C_INCLUDES precedes public library headers.
p9-unique = $(if $(strip $(1)),$(firstword $(1)) $(call p9-unique,$(filter-out $(firstword $(1)),$(1))))

# $(call p9-export-dirs-one,module,target,visited)
define p9-export-dirs-one
$(if $(filter $(1),$(3)),$(error Header export cycle: $(strip $(3) $(1))))
$(if $(filter $(1),$(P9_EXPORTED_MODULES)),,$(error Unknown header provider: $(1)))
$(if $(filter $(2),$(P9_EXPORT_TARGET.$(1))),,$(error Header provider $(1) target mismatch: $(P9_EXPORT_TARGET.$(1)) vs $(2)))
$(P9_EXPORT_DIRS.$(1)) \
$(foreach dep,$(P9_EXPORT_REEXPORT.$(1)),\
    $(call p9-export-dirs-one,$(dep),$(2),$(strip $(3) $(1))))
endef

p9-import-dirs = $(strip $(call p9-unique,\
    $(foreach dep,$(1),$(call p9-export-dirs-one,$(dep),$(2),))))

# Header libraries are a distinct class: do not mistake a static archive for one.
define p9-validate-header-one
$(if $(filter header,$(P9_EXPORT_KIND.$(1))),,\
    $(error $(1) is not a BUILD_HEADER_LIBRARY module))
$(if $(filter $(2),$(P9_EXPORT_TARGET.$(1))),,\
    $(error Header library $(1) target mismatch: $(P9_EXPORT_TARGET.$(1)) vs $(2)))
endef
p9-validate-header-libs = $(strip $(foreach dep,$(1),$(call p9-validate-header-one,$(dep),$(2))))

# Generated headers: re-export their build dependencies along the same edges.
define p9-export-deps-one
$(if $(filter $(1),$(3)),$(error Header export cycle: $(strip $(3) $(1))))
$(P9_EXPORT_INCLUDE_DEPS.$(1)) \
$(foreach dep,$(P9_EXPORT_REEXPORT.$(1)),\
    $(call p9-export-deps-one,$(dep),$(2),$(strip $(3) $(1))))
endef
p9-import-deps = $(strip $(call p9-unique,\
    $(foreach dep,$(1),$(call p9-export-deps-one,$(dep),$(2),))))

# A phony prerequisite makes the flags signature recipe run for each invocation;
# the signature timestamp changes only when the effective options change.
.PHONY: p9-force-compile-flags
p9-force-compile-flags:
