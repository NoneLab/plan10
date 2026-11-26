# U3.10: Android.mk-style public header propagation

Run from the repository root:

```sh
bash tests/mk-includes/test.sh
make -f tests/mk-backends/Makefile check USERSPACE_SYSTEM_STATIC_LIBRARIES=libc-plan9
make -f tests/mk-backends/Makefile check USERSPACE_SYSTEM_STATIC_LIBRARIES=libc-experimental
```

This test uses the production `mk/` backends, Clang/LLD and the real ELF-to-6.out
converter. It verifies C, C++ and preprocessed assembly include resolution;
static and header-only libraries; explicit re-exports; native libc selection;
generated header prerequisites; stable incremental builds; options signature
changes; and parse-time rejection of invalid headers/ABI declarations.

The test is not a full Plan 9 kernel build or QEMU boot test.
