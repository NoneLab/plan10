#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"
#include <tos.h>
#include "initcode_amd64.h"

enum 
{
    Ustkheadroom = sizeof(Sargs) + sizeof(uintptr) + sizeof(Tos),
};

static uchar *initsp;

static uchar*
pusharg(uchar **stack, char *arg)
{
    ulong n;

    n = strlen(arg) + 1;
    *stack -= n;
    memmove(*stack, arg, n);
    return *stack;
}

/* base is the kernel alias of the initial (topmost) user stack page. */
void
bootargs(void *base)
{
    uchar *stack, *arg0;
    ulong *argv;
    ulong delta;

    stack = (uchar*)base + BY2PG - Ustkheadroom;
    arg0 = pusharg(&stack, "/amd64/9pc64");

    /* Provide the user ABI's argc and argv vector as native 64-bit words. */
    stack = (uchar*)((uintptr)stack & ~(uintptr)15);
    stack -= 3 * BY2WD; /* argc, argv[0], trailing nil */
    argv = (ulong*)stack;
    delta = (USTKTOP - BY2PG) - (ulong)(uintptr)base;
    argv[0] = 1;
    argv[1] = (ulong)(uintptr)arg0 + delta;
    argv[2] = 0;

    initsp = (uchar*)((uintptr)stack + delta);
}

void
init0(void)
{
    char buf[2*KNAMELEN];

    up->nerrlab = 0;
    spllo();

    /* Preserve the original pc/main.c namespace and device initialisation. */
    up->slash = namec("#/", Atodir, 0, 0);
    pathclose(up->slash->path);
    up->slash->path = newpath("/");
    up->dot = cclone(up->slash);
    chandevinit();

    if(!waserror()){
        snprint(buf, sizeof buf, "pc64 %s", conffile);
        ksetenv("terminal", buf, 0);
        ksetenv("cputype", "amd64", 0);
        ksetenv("service", cpuserver ? "cpu" : "terminal", 0);
        poperror();
    }

    /* Keep the real Plan 9 alarm process used by the original init0(). */
    kproc("alarm", alarmkproc, nil);
    print("pc64: init0 pid=%lud entering native user mode\n", up->pid);
    touser((void*)(uintptr)initsp);
    panic("pc64: touser returned");
}

void
userinit(void)
{
    Proc *p;
    Segment *s;
    Page *pg;
    void *v;
    ulong stacktop;

    if(sizeof(initcode_amd64) > BY2PG)
        panic("pc64: native initcode does not fit on one page");

    p = newproc();
    p->pgrp = newpgrp();
    p->egrp = smalloc(sizeof(Egrp));
    p->egrp->ref = 1;
    p->fgrp = dupfgrp(nil);
    p->rgrp = newrgrp();
    p->procmode = 0640;
    kstrdup(&eve, "");
    kstrdup(&p->text, "*init*");
    kstrdup(&p->user, eve);
    p->fpstate = FPinit;

    /* gotolabel() jumps to a SysV function entry, requiring a return slot. */
    p->sched.pc = (ulong)(uintptr)init0;
    p->sched.sp = (ulong)(uintptr)p->kstack + KSTACK - BY2WD;
    *(ulong*)(uintptr)p->sched.sp = 0;

    s = newseg(SG_STACK, USTKTOP-USTKSIZE, USTKSIZE/BY2PG);
    p->seg[SSEG] = s;
    pg = newpage(0, nil, USTKTOP-BY2PG);
    v = tmpmap(pg);
    memset(v, 0, BY2PG);
    segpage(s, pg);
    bootargs(v);
    stacktop = (ulong)(uintptr)initsp;
    tmpunmap(v);

    s = newseg(SG_TEXT, UTZERO, 1);
    s->flushme++;
    p->seg[TSEG] = s;
    pg = newpage(0, nil, UTZERO);
    memset(pg->cachectl, PG_TXTFLUSH, sizeof(pg->cachectl));
    segpage(s, pg);
    v = tmpmap(pg);
    memset(v, 0, BY2PG);
    memmove(v, initcode_amd64, sizeof(initcode_amd64));
    tmpunmap(v);

    print("pc64: userinit pid=%lud entry=%#lux stack=%#lux image=%lud bytes\n",
          p->pid, (ulong)(UTZERO+32), stacktop, (ulong)sizeof(initcode_amd64));
    ready(p);
}