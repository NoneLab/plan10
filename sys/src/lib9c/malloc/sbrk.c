/* Private, grow-only adaptation of libc/9sys/sbrk.c. */
#include <stddef.h>
#include <errno.h>
#include "../internal/native.h"
#include "pool_compat.h"

extern char end[]; /* Provided by pc64/boot6.ld. */
static char *heapend = end;

enum { HeapAlign = 16 };

/* Only called while the pool lock is held.  A single allocator must own
 * BRK_: another sbrk/brk user could shrink or overwrite this heap. */
void *
p9HeapGrow(size_t size)
{
    uintptr base = (uintptr)heapend;
    uintptr next;

    if (base > (uintptr)-1 - (HeapAlign - 1))
        goto nomem;
    base = (base + HeapAlign - 1) & ~(uintptr)(HeapAlign - 1);
    if (size > (uintptr)-1 - base || size == 0)
        goto nomem;
    next = base + size;

    if (P9C_CALL(BRK_, next, 0, 0, 0, 0) < 0)
        goto nomem;
    heapend = (char *)next;
    return (void *)base;

nomem:
    errno = ENOMEM;
    return NULL;
}
