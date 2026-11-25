#!/bin/sh
# Validate the real common _start; no hosted C runtime or replacement CRT.
set -eu

CLANG=${CLANG:-clang}
LLD=${LLD:-ld.lld}
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT HUP INT TERM

"$CLANG" -c --target=x86_64-unknown-none-elf \
    sys/src/libc/amd64/crt0.S -o "$work/crt0.o"

for mode in weak strong strong-null; do
    define=
    case "$mode" in
        strong) define=-DTEST_STRONG_ARGV0 ;;
        strong-null) define=-DTEST_STRONG_NULL_ARGV0 ;;
    esac
    "$CLANG" -c --target=x86_64-unknown-none-elf $define \
        tests/mk-backends/src/crt_entry_test.S -o "$work/main.o"
    "$LLD" -m elf_x86_64 -nostdlib -static -e _start \
        -o "$work/test-$mode" "$work/crt0.o" "$work/main.o"
    "$work/test-$mode" sample-argument
    echo "PASS common crt0: argc/argv, exits(nil), argv0=$mode"
done
