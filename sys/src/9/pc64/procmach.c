/*
 * Process-specific AMD64 state for the original Plan 9 /port scheduler.
 * There is no userspace entry until the native syscall and init image are
 * connected; these routines nevertheless implement the real SysV ABI
 * coroutine and 64-bit trap-state contracts.
 */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "ureg.h"
#include <tos.h>

extern void p9_amd64_forkret(void);

void
procsetup(Proc *p)
{
    p->fpstate = FPinit;
    memset(&p->fpsave, 0, sizeof p->fpsave);
}

void
procsave(Proc *p)
{
    uvlong t;

    cycles(&t);
    p->pcycles += t;
    if(p->fpstate == FPactive){
        /* The Plan 9 FPsave layout has 16-byte alignment for fxsave64. */
        __asm__ volatile("fxsave64 %0" : "=m"(p->fpsave) :: "memory");
        p->fpstate = FPinactive;
    }
    /* Kernel root before the next process's user PML4 is selected. */
    putcr3(paddr(m->pdb));
}

void
procrestore(Proc *p)
{
    uvlong t;

    if(p->kp)
        return;
    cycles(&t);
    p->pcycles -= t;
    if(p->fpstate == FPinactive){
        __asm__ volatile("fxrstor64 %0" :: "m"(p->fpsave) : "memory");
        p->fpstate = FPactive;
    }
}

static void
linkproc(void)
{
    spllo();
    up->kpfun(up->kparg);
    pexit("kproc completed", 0);
}

void
kprocchild(Proc *p, void (*func)(void *), void *arg)
{
    /* A jmp through gotolabel() must mimic a SysV function-entry stack. */
    p->sched.pc = (ulong)(uintptr)linkproc;
    p->sched.sp = (ulong)(uintptr)p->kstack + KSTACK - BY2WD;
    *(ulong*)(uintptr)p->sched.sp = 0;
    p->kpfun = func;
    p->kparg = arg;
}

void
forkchild(Proc *p, Ureg *ur)
{
    Ureg *saved;

    p->sched.sp = (ulong)(uintptr)p->kstack + KSTACK - sizeof(Ureg);
    p->sched.pc = (ulong)(uintptr)p9_amd64_forkret;
    saved = (Ureg*)(uintptr)p->sched.sp;
    memmove(saved, ur, sizeof(*saved));
    saved->ax = 0;
    p->psstate = 0;
    p->insyscall = 0;
}

long
execregs(ulong entry, ulong ssize, ulong nargs)
{
    Ureg *ur;
    ulong *sp;

    if(up == nil || up->dbgreg == nil)
        panic("pc64: missing user trap frame in exec");
    up->fpstate = FPinit;
    sp = (ulong*)(uintptr)(USTKTOP - ssize);
    *--sp = nargs;
    ur = up->dbgreg;
    ur->sp = (ulong)(uintptr)sp;
    ur->pc = entry;
    return USTKTOP - sizeof(Tos);
}

void
idlehands(void)
{
    /* Enable interrupts before idle: an IRQ must wake sched(). */
    __asm__ volatile("sti; hlt" ::: "memory");
}
