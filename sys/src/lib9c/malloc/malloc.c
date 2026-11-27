/* Plan 9 pool allocator adapted from libc/port/malloc.c. */
#include <9c/malloc.h>
#include <stdarg.h>
#include "pool_compat.h"
#include "../internal/native.h"

/* The original pool stores internal sizes in ulong and limits a single
 * request to less than 2 GiB.  Use a conservative total arena limit. */
#define HeapLimit (512UL * 1024 * 1024)
#define ArenaMagic 0xDeadBeefUL

_Static_assert(sizeof(ulong) == 8, "lib9c allocator requires the AMD64 ABI");
_Static_assert(__alignof__(max_align_t) <= 16, "heap alignment too small");

static unsigned int heaplock;

static void
poollock(Pool *p)
{
    (void)p;
    while (__atomic_exchange_n(&heaplock, 1, __ATOMIC_ACQUIRE)) {
        while (__atomic_load_n(&heaplock, __ATOMIC_RELAXED))
            (void)P9C_CALL(SLEEP, 0, 0, 0, 0, 0);
    }
}

static void
poolunlock(Pool *p)
{
    (void)p;
    __atomic_store_n(&heaplock, 0, __ATOMIC_RELEASE);
}

/* No stdio, abort(), or heap allocation on allocator failure paths. */
static void
rawprint(const char *s)
{
    size_t len = strlen(s);

    while (len != 0) {
        long n = P9C_CALL(PWRITE, 2, s, len, -1L, 0);
        if (n <= 0 || (size_t)n > len)
            break;
        s += n;
        len -= (size_t)n;
    }
}

void
p9PoolFail(const char *message)
{
    rawprint("lib9c: ");
    rawprint(message);
    rawprint("\n");
    (void)P9C_CALL(EXITS, "lib9c: corrupt pool", 0, 0, 0, 0);
    __builtin_trap();
}

static void
poolprint(Pool *p, char *fmt, ...)
{
    va_list ap;
    (void)p;

    /* Preserve the corruption reason.  Other original diagnostic formats
     * use Plan 9 verbs; emit their text without interpreting varargs. */
    if (strcmp(fmt, "%s\n") == 0) {
        va_start(ap, fmt);
        rawprint(va_arg(ap, char *));
        va_end(ap);
        rawprint("\n");
    } else {
        rawprint(fmt);
    }
}

static void
poolpanic(Pool *p, char *fmt, ...)
{
    (void)p;
    p9PoolFail(fmt);
}

/* Two words before each arena let adjacent sbrk extents be merged.
 * Both the header and the resulting arena are 16-byte aligned. */
static void *
allocarena(ulong size)
{
    ulong *header;

    if (size > HeapLimit || size > (ulong)-1 - 2 * sizeof(ulong)) {
        errno = ENOMEM;
        return NULL;
    }
    header = p9HeapGrow(size + 2 * sizeof(ulong));
    if (header == NULL)
        return NULL;
    header[0] = size + 2 * sizeof(ulong);
    header[1] = ArenaMagic;
    return header + 2;
}

static int
mergearena(void *left, void *right)
{
    ulong *a = left;
    ulong *b = right;

    if (a[-1] != ArenaMagic || b[-1] != ArenaMagic)
        p9PoolFail("invalid arena header");
    if ((uintptr)a + a[-2] != (uintptr)b)
        return 0;
    a[-2] += b[-2];
    return 1;
}

static Pool heap = {
    .name = "lib9c",
    .maxsize = HeapLimit,
    .minarena = 4096,
    .quantum = 32,
    .alloc = allocarena,
    .merge = mergearena,
    .lock = poollock,
    .unlock = poolunlock,
    .print = poolprint,
    .panic = poolpanic,
};

void *
p9Malloc(size_t size)
{
    void *ptr;

    if (size > HeapLimit) {
        errno = ENOMEM;
        return NULL;
    }
    ptr = poolalloc(&heap, size ? size : 1);
    if (ptr == NULL)
        errno = ENOMEM;
    return ptr;
}

void *
p9Mallocz(size_t size, int clear)
{
    void *ptr = p9Malloc(size);

    if (ptr != NULL && clear)
        memset(ptr, 0, size);
    return ptr;
}

void *
p9Calloc(size_t count, size_t size)
{
    if (size != 0 && count > (size_t)-1 / size) {
        errno = ENOMEM;
        return NULL;
    }
    return p9Mallocz(count * size, 1);
}

void
p9Free(void *ptr)
{
    if (ptr != NULL)
        poolfree(&heap, ptr);
}

void *
p9Realloc(void *ptr, size_t size)
{
    void *result;

    if (ptr == NULL)
        return p9Malloc(size);
    if (size == 0) {
        p9Free(ptr);
        return NULL;
    }
    if (size > HeapLimit) {
        errno = ENOMEM;
        return NULL;
    }
    result = poolrealloc(&heap, ptr, size);
    if (result == NULL)
        errno = ENOMEM;
    return result;
}

size_t
p9Msize(void *ptr)
{
    return ptr == NULL ? 0 : poolmsize(&heap, ptr);
}

char *
p9Strdup(const char *str)
{
    size_t size;
    char *copy;

    if (str == NULL) {
        errno = EINVAL;
        return NULL;
    }
    size = strlen(str);
    if (size >= HeapLimit) {
        errno = ENOMEM;
        return NULL;
    }
    copy = p9Malloc(size + 1);
    if (copy != NULL)
        memcpy(copy, str, size + 1);
    return copy;
}
