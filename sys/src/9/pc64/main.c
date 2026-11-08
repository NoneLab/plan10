/*
 * Native AMD64 adaptation of ../pc/main.c's Plan 9 kernel bring-up.
 *
 * This is the original Plan 9 kernel initialisation path: /port heap,
 * allocator, proc table, channels, devices and page pool.  It is not a
 * second-stage/bootstrap kernel or an emulation of the Plan 9 services.
 *
 * The user process creation and entry sequence is intentionally NOT
 * attempted until AMD64 syscall, traps and original initcode have been
 * completed.  The kernel remains in a well-defined idle loop after the
 * machine-independent core has been initialised.
 */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "pool.h"

Conf conf;

extern void mmuinit0(void);
extern void p9_amd64_serialinit(void);
extern void p9_amd64_clockinit(void);
extern int p9_amd64_memregion(ulong *, ulong *);
extern void p9_amd64_picinit(void);
extern void p9_amd64_irqstart(void);

char *
getconf(char *name)
{
    USED(name);
    /* Until plan9.ini/multiboot cmdline parsing is installed, no options. */
    return nil;
}

void
meminit(void)
{
    ulong start, limit;

    if(p9_amd64_memregion(&start, &limit) < 0 || limit <= start)
        panic("pc64: no usable RAM above reserved boot memory");

    conf.mem[0].base = start;
    conf.mem[0].npage = (limit - start) / BY2PG;
    conf.base0 = start;
    conf.npage = conf.mem[0].npage;
}

void
confinit(void)
{
    ulong available, kpages, reserved;

    conf.nmach = 1;
    available = conf.npage;
    if(available < 2048)
        panic("pc64: insufficient physical RAM (%lud pages)", available);

    conf.nproc = 100 + (available * BY2PG / MB) * 5;
    if(conf.nproc > 2000)
        conf.nproc = 2000;
    conf.nimage = 200;
    conf.nswap = conf.nproc * 80;
    conf.nswppo = 4096;

    /* Preserve original pc/main.c's terminal split: about 40% kernel. */
    kpages = (available * 40) / 100;
    conf.upages = available - kpages;
    conf.ialloc = (kpages / 2) * BY2PG;

    reserved = conf.upages * sizeof(Page)
             + conf.nproc * sizeof(Proc)
             + conf.nimage * sizeof(Image)
             + conf.nswap
             + conf.nswppo * sizeof(Page);
    if(reserved >= kpages * BY2PG)
        panic("pc64: kernel heap metadata exceeds kernel RAM");

    mainmem->maxsize = kpages * BY2PG - reserved;
    imagmem->maxsize = mainmem->maxsize;
}

/* The original Plan 9 scheduler/driver flow is not changed for this port. */
void
main(void)
{
    m->machno = 0;
    m->loopconst = 100000;
    m->perf.period = 1;
    machp[0] = m;
    active.machs = 1;

    p9_amd64_serialinit();
    print("\nPlan 9/amd64: kernel entry\n");

    mmuinit0();
    trapinit0();
    mmuinit();
    p9_amd64_clockinit();
    p9_amd64_picinit();

    meminit();
    confinit();
    xinit();
    printinit();
    timersinit();
    procinit0();
    initseg();
    chandevreset();
    pageinit();
    swapinit();
    p9_amd64_irqstart();

    print("Plan 9/amd64: core kernel initialised, userspace pending\n");
    /* Userspace is a later milestone; do not enter it via fake initcode. */
    for(;;)
        __asm__ volatile("sti; hlt" ::: "memory");
}
