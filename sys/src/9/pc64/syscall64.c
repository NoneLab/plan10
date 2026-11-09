#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "ureg.h"
#include "../port/error.h"
#include "systab_amd64.h"

void 
p9_amd64_syscall(Ureg* ur)
{
    ulong scallnr, sp;
    long ret;
    char* err;
    int s;

    if (up == nil || !userureg(ur))
        panic("pc64: syscall without user process");

    scallnr = ur->ax;
    sp = ur->sp;
    up->dbgreg = ur;
    up->pc  = ur->ip;
    up->scallnr = scallnr;
    up->insyscall = 1;
    up->nerrlab = 0;
    m->syscall++;
    ret = -1;

    s = spllo();
    if (!waserror())
    {
        if (scallnr >= (ulong)nsyscall)
            error(Ebadarg);

        if (scallnr == NOTED || scallnr == NOTIFY)
            error(Ebadarg);

        if (sp < UTZERO || sp > USTKTOP - sizeof(Sargs) - BY2WD)
            error(Ebadarg);

        validaddr(sp + BY2WD, sizeof(Sargs), 0);
        memmove(&up->s, (void*)(uintptr)(sp + BY2WD), sizeof(Sargs));
        up->psstate = sysctab[scallnr];
        ret = systab[scallnr](up->s.args);
        poperror();
    }
    else 
    {
        err = up->syserrstr;
        up->syserrstr = up->errstr;
        up->errstr = err;
    }

    if(up->nerrlab != 0)
        panic("pc64: syscall unbalanced error stack");

    ur->ax = ret;
    up->psstate = nil;
    up->insyscall = 0;
    splx(s);

    if (up->procctl || up->nnote)
        panic("pc64: syscall requires unprorted user note delivery");

    if (up->delaysched)
        sched();
}