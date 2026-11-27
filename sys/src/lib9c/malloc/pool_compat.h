#ifndef P9_LIB9C_POOL_COMPAT_H
#define P9_LIB9C_POOL_COMPAT_H

/* Private adapter for the original Plan 9 pool implementation.  Do not
 * include u.h or libc.h: those headers describe a different libc ABI. */
#include <stddef.h>
#include <errno.h>
#include <string.h>

typedef unsigned char uchar;
typedef unsigned long ulong;
typedef __UINTPTR_TYPE__ uintptr;

#define nil NULL
#define nelem(a) (sizeof(a) / sizeof((a)[0]))

/* Rename every externally defined pool entry before reading pool.h. */
#define poolalloc       p9PoolAlloc
#define poolallocalign  p9PoolAllocAlign
#define poolfree        p9PoolFree
#define poolmsize       p9PoolMsize
#define poolrealloc     p9PoolRealloc
#define poolcheck       p9PoolCheck
#define poolcompact     p9PoolCompact
#define poolblockcheck  p9PoolBlockCheck
#define pooldump        p9PoolDump
#include "../../../include/pool.h"

void p9PoolDump(Pool *);
void p9PoolFail(const char *) __attribute__((noreturn));
void *p9HeapGrow(size_t size);

/* The shared source uses these three werrstr messages.  Keep errno and
 * avoid importing the Plan 9 formatter or calling a libc allocator. */
static inline void
p9PoolError(const char *msg, ...)
{
    errno = strcmp(msg, "memory pool too large") == 0 ? ENOMEM : EINVAL;
}

#define werrstr p9PoolError
#define assert(expr) ((expr) ? (void)0 : p9PoolFail("assertion: " #expr))

#endif
