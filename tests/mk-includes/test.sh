#!/bin/sh
set -eu
out=out/amd64/include-selftest
rm -rf "$out"
make -f tests/mk-includes/Makefile all extra EXPECT_EXPERIMENTAL=0 BUILD_DIR="$out" >/dev/null
app_obj="$out/obj/app/app.o"
test -s "$out/bin/test-app.6.out"
test -s "$app_obj"
test -s "$out/obj/cxx/cxx.o"
test -s "$out/obj/asm/asm.o"
# Stable configuration must not rebuild objects after their .flags checks.
first=$(stat -c %Y "$app_obj")
sleep 1
make -f tests/mk-includes/Makefile all EXPECT_EXPERIMENTAL=0 BUILD_DIR="$out" >/dev/null
test "$(stat -c %Y "$app_obj")" = "$first"
# Switching default libc must switch the selected public header and compile.
make -f tests/mk-includes/Makefile all EXPECT_EXPERIMENTAL=1 \
  USERSPACE_SYSTEM_STATIC_LIBRARIES=libc-experimental BUILD_DIR="$out" >/dev/null
test "$(stat -c %Y "$app_obj")" != "$first"
# Changes in include paths should also force recompilation.
before=$(stat -c %Y "$app_obj")
sleep 1
make -f tests/mk-includes/Makefile all EXPECT_EXPERIMENTAL=1 \
  USERSPACE_SYSTEM_STATIC_LIBRARIES=libc-experimental \
  EXTRA_APP_INCLUDE=tests/mk-includes/include/extra BUILD_DIR="$out" >/dev/null
test "$(stat -c %Y "$app_obj")" != "$before"
# Re-exported header-only library is visible to an indirect consumer.
make -f tests/mk-includes/Makefile all REEXPORT_PRIVATE=1 EXPECT_EXPERIMENTAL=0 \
  EXTRA_APP_CFLAGS=-DEXPECT_PRIVATE BUILD_DIR=out/amd64/include-reexport >/dev/null
# Without explicit re-export, the consumer must not see math-priv's headers.
if make -f tests/mk-includes/Makefile all EXPECT_EXPERIMENTAL=0 \
  EXTRA_APP_CFLAGS=-DEXPECT_PRIVATE \
  BUILD_DIR=out/amd64/include-not-exported >/dev/null 2>&1; then
  echo 'transitive private headers were leaked' >&2; exit 1
fi
# Missing providers, ABI mismatch, illegal re-export and malformed headers fail.
for case in unknown mismatch reexport header-kind wrong-provider-type; do
  if make -f tests/mk-includes/invalid.mk CASE="$case" all >/dev/null 2>&1; then
    echo "unexpectedly accepted invalid header configuration: $case" >&2; exit 1
  fi
done
echo 'U3.10 header registry: PASS'
