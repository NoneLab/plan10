# lib9c: legacy Plan 9 C facilities

The userspace archive exposes a separately named allocator and Plan 9 native
interfaces for lib10c clients such as lib9pp. It reuses the original Plan 9 pool
algorithm and grows the BSS segment through the existing AMD64 BRK_ syscall.
No kernel VM changes, replacement of lib10c malloc, or legacy libc linkage
are required. The syscall entry is supplied by lib10c.

Include `<9c/malloc.h>` and link `lib9c` as a static module with `lib10c` as
the system runtime. Use `p9Free` for every allocation from `p9Malloc`,
`p9Mallocz`, `p9Calloc`, `p9Realloc`, and `p9Strdup`. Never pass such storage
to `free`, and never pass standard allocations to `p9Free` or `p9Realloc`.
`p9Msize` exposes the pool's usable block size; `p9Realloc(p, 0)` frees a
non-null p and returns NULL. A zero-sized allocation returns a minimum
allocation if memory is available. Failed allocation sets errno; failed
reallocation with a nonzero size preserves the original allocation.

The private pool has a 512 MiB arena budget and 16-byte payload alignment.
Its arena extents are kept for reuse until process exit; it does not shrink
BRK_. Only this allocator may control BRK_ in a lib10c address space using
lib9c. It is not a hardened OpenBSD allocator and does not implement guard
pages, secure erasure, fork handlers, or thread-local errno. Pool mutation
uses an atomic lock with a native scheduler yield; do not allocate in a note
handler or fork while another execution context holds that lock. Full
RFMEM/concurrent process lifecycle support is not verified. Pool failure
messages use raw native I/O, without interpreting Plan 9 format verbs or
calling stdio or malloc. Optional original pool debugging flags are not
exposed.

The standard libc allocator and its VM dependencies are unchanged. In
particular, `strdup`, `asprintf`, buffered stdio, and `atexit` can still
require OpenBSD malloc or VM support. Use explicit lib9c allocation and
release throughout the future lib9pp; do not use global renaming macros.

Build the archive with `make lib9c`.

## Native C API

All native API names use the p9Prefix convention, and native structures and
constants have P9 prefixes. Include `<9c.h>` or individual headers. Link lib9c
with lib10c; do not import the legacy `<u.h>` or `<libc.h>` in modern code.

| Header | Facilities |
| --- | --- |
| malloc.h | Existing pool allocator (unchanged) |
| file.h | Native open/create/remove, FD operations, seek, read/write, packed stat, authentication/version, access and coalesced vector I/O |
| dir.h | P9Qid/P9Dir, 9P2000 stat codec, dir stat/read/write helpers |
| ipc.h | Full-duplex pipe, rendezvous, native semaphores |
| namespace.h | Bind, mount, unmount |
| process.h | Rfork/fork/exec/execl, wait/await, sleep/alarm/nsec, native notes |
| segment.h | Attach/detach/resize/free/flush (existing segment ABI only) |
| error.h | Errstr exchange, read/format native errors, fatal exit |
| env.h | Native /env files, user and system names, parent PID |
| sync.h | Atomics, semaphore locks, QLock/RWLock/Rendez |
| string.h | Native tokenization, path normalization, quoting and string helpers |
| rune.h | UTF-8 and Rune string helpers |

### ABI and ownership

- Syscall numbers and arguments come from `libc/9syscall/sys.h`,
  `libc/amd64/bootcalls.c`, and the kernel dispatch table. The wrappers use
  lib10c's existing p9_syscall5 and __lib10c_sysresult, not libc-plan9.
  The latter maps known kernel error strings to errno and preserves them;
  unknown errors map to EIO. ERRSTR exchanges buffers and is not recursively
  translated. Rendezvous preserves all machine-word bits except its native
  interruption sentinel (void *)-1; do not use that value as payload.
- P9_* flags are native constants, not OpenBSD POSIX constants. Read/write/close
  can also use lib10c's standard interfaces. Mount/bind return a mount ID;
  successful mount consumes fd. A failed mount leaves fd with the caller.
- P9Dir is an in-memory object, not a packed wire struct. Stat fields are fixed
  width and little endian. p9ConvD2M may return 2 for a size probe. p9ConvM2D
  takes a separate caller-owned string buffer. All returned dir arrays, wait
  messages, environment values, Rune duplicates and quoted strings are one
  lib9c allocation and must be released with p9Free.
- p9Errstr exchanges the native error string, p9Rerrstr preserves it. The
  p9Werrstr/p9Sysfatal format is standard printf, not Plan 9 custom Fmt.
  Complex formatting can still depend on lib10c's unfinished allocator.
- Native semaphore words are long on AMD64. Lock objects must be zeroed and
  shared by cooperating processes in the same rendezvous group. Queued lock
  nodes use a bounded 1024-entry shared BSS pool (not private process stacks).
  No pthread/TLS integration, cancellation, process-death recovery or robust
  lock guarantees. RFMEM concurrency and note handling require native tests.
- p9Segfree does not unmap an address range. Native segment APIs are not
  replacements for POSIX mmap/munmap or mprotect. Never resize or detach the
  BSS segment containing lib9c's heap. Raw BRK_ and public sbrk/brk are
  intentionally not exposed because the allocator exclusively owns BRK_.
- p9Getuser/p9Sysname return static buffers; do not free them. p9Getenv uses
  native /env files, independent of lib10c environ. These and /proc/#d helpers
  require the respective device/namespace to be present.

### 9P server integration

p9Pipe creates two bidirectional endpoints. Create a registry file under /srv
with p9Create and P9_OWRITE | P9_ORCLOSE | P9_OCEXEC | P9_OEXCL. Write the
client endpoint FD as one decimal string, and keep the registry descriptor
open for the service lifetime. Close unused endpoint references and separate
FD groups. A different process can p9Open the registry with P9_ORDWR and
p9Mount the resulting channel. The server must already be handling version
and attach requests while the mount blocks. This library does not launch a
server or provide the Fcall codec; that belongs to lib9pp.

### Scope and remaining compatibility

This is a native ABI and core helper port, not a complete clone of every
function declared in the historical libc.h. Obsolete pre-9P2000 syscall
variants, SYSR1 and allocator-internal BRK_ are not public. ISO C/POSIX
functions remain in lib10c. The original custom Fmt engine, Unicode case/base
classification tables, full Rune quoting variants, allocation tags/aligned
pool APIs, old network/TLS/crypto libraries, profiling, floating-point/math,
TOS/private-data/atexit/notifier internals and other high-level compatibility
helpers are not part of this increment. 9P Fcall and server state belong to
lib9pp. No unsupported functionality is advertised via fake success stubs.

Build with `make lib9c`. No test target or test source is installed by this
increment. Real target linking, QEMU syscalls and shared-process locking must
be verified separately.
