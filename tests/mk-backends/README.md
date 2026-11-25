# Backend split regression tests

Run from project root:

```sh
make -f tests/mk-backends/Makefile check USERSPACE_LIBC=plan9
make -f tests/mk-backends/Makefile check USERSPACE_LIBC=experimental
```

Fixtures use the actual build backends and minimal cross-target code:
- kernel `ELF + Multiboot` (including an ABI-matched `.a`);
- bootstrap native `6.out` requiring an explicitly registered CRT module;
- regular userspace native `6.out`, with `plan9` or `experimental` test libc;
- host-native C++17 executable and host `.a`;
- archive transitive dependency, cross-target rejection and missing CRT checks.

This tests build mechanics only, not the real Plan 9 runtime or QEMU boot.

The production `sys/src/libc/amd64/crt0.S` is compiled as the default
userspace and bootstrap CRT, with an explicit alternate CRT override test.
`test-crt-entry.sh` also runs the real `_start` on the host with a minimal
syscall-only fixture to validate argc/argv and weak, strong-initialized and strong-null argv0 behavior.
