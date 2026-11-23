# Plan 9 GNU Make implementation of Plan9.mk-style declarations.
# Deliberately independent of Android build/core and Soong; uses the existing Clang ABI.
# Include once from the root Makefile, before any component Plan9.mk.

# Plan 9 module directory convention: use before including other files.
my-dir = $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))

CLEAR_VARS := mk/clear-vars.mk
BUILD_OBJECTS := mk/build-objects.mk
