#include <9c/segment.h>
#include "../internal/native.h"

void *
p9Segattach(int attr, const char *name, void *addr, unsigned long len)
{
    return (void *)(__UINTPTR_TYPE__)__lib10c_sysresult(
        P9C_CALL(SEGATTACH, attr, name, addr, len, 0));
}

void *
p9Segbrk(void *addr, void *newtop)
{
    return (void *)(__UINTPTR_TYPE__)__lib10c_sysresult(
        P9C_CALL(SEGBRK, addr, newtop, 0, 0, 0));
}

int
p9Segdetach(void *addr)
{
    return p9IntResult(P9C_CALL(SEGDETACH, addr, 0, 0, 0, 0));
}

int
p9Segfree(void *addr, unsigned long len)
{
    return p9IntResult(P9C_CALL(SEGFREE, addr, len, 0, 0, 0));
}

int
p9Segflush(void *addr, unsigned long len)
{
    return p9IntResult(P9C_CALL(SEGFLUSH, addr, len, 0, 0, 0));
}
