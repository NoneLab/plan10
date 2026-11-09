#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "ureg.h"

extern void p9_amd64_iretuser(Ureg *) __attribute__((noreturn));

void
touser(void * user_sp)
{
    Ureg ur;

    if (up == nil || user_sp == nil)
        panic("pc64: touser without a mapped user process");

    if ((ulong)(uintptr)user_sp >= USTKTOP ||
        (ulong)(uintptr)user_sp < USTKTOP - USTKSIZE)
        panic("pc64: invalid user stack for touser");

    memset(&ur, 0, sizeof(ur));
    ur.ip = UTZERO + 32;
    ur.sp = (ulong)(uintptr)user_sp;
    ur.cs = UESEL;
    ur.ss = UDSEL;
    ur.flags = 0x202;

    p9_amd64_iretuser(&ur);
}