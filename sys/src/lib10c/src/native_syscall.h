#ifndef LIB10C_NATIVE_SYSCALL_H
#define LIB10C_NATIVE_SYSCALL_H

/* The existing Plan 9 AMD64 syscall ABI, not the OpenBSD syscall ABI. */
#include "../../libc/9syscall/sys.h"
#include "../../libc/amd64/syscall.h"


#define P9_CALL(n, a, b, c, d, e) \
    p9_syscall5((n), (unsigned long)(a), (unsigned long)(b), \
        (unsigned long)(c), (unsigned long)(d), (unsigned long)(e))

/* Translate the kernel's string error to a best-effort POSIX errno. */
__dso_hidden long __lib10c_sysresult(long);

#endif
