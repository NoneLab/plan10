#ifndef P9_LIB9C_NATIVE_H
#define P9_LIB9C_NATIVE_H

#include "../../libc/9syscall/sys.h"
#include "../../libc/amd64/syscall.h"

#define P9C_CALL(n, a, b, c, d, e) \
    p9_syscall5((n), (unsigned long)(a), (unsigned long)(b), \
        (unsigned long)(c), (unsigned long)(d), (unsigned long)(e))

/* Reuse lib10c's allocation-free native error-to-errno adapter.
 * This is a private static-link dependency, not a public lib9c API. */
long __lib10c_sysresult(long result) __attribute__((visibility("hidden")));

static inline int
p9IntResult(long result)
{
    return (int)__lib10c_sysresult(result);
}

#endif
