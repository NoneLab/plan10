#include <u.h>
#include <libc.h>
#include <tos.h>

Tos *_tos = (Tos*)(uintptr)(0x00007fffffffe000ULL - sizeof(Tos));

long ainc(long *p) 
{
    return __atomic_add_fetch(p, 1, __ATOMIC_SEQ_CST);
}

long adec(long *p)
{
    return __atomic_sub_fetch(p, 1, __ATOMIC_SEQ_CST);
}

int _tas(int *p)
{
    return __atomic_exchange_n(p, 1, __ATOMIC_ACQUIRE);
}

void abort()
{
    _exits("abort");
}