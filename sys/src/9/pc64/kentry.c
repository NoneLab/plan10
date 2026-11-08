/*
 * Entry handoff from the AMD64 machine startup to the actual Plan 9 main().
 * No stand-alone bootstrap kernel or test substitute is linked here: if the
 * Plan 9 kernel's main() is not compiled, the final link MUST fail.
 */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"

extern void main(void);
extern void p9_amd64_memsetup(ulong);
extern ulong *p9_amd64_bootpml4(void);
extern Mach *m;
extern Mach *machp[MAXMACH];

/* Physical Multiboot information address, retained for the real early port. */
ulong p9_amd64_mbinfo;

__attribute__((noreturn)) void
p9_amd64_kentry(ulong mbinfo)
{
    p9_amd64_mbinfo = mbinfo;
    p9_amd64_memsetup(mbinfo);

    /* First CPU's identity and page root for the real Plan 9 scheduler. */
    m->machno = 0;
    m->pdb = p9_amd64_bootpml4();
    machp[0] = m;

    main();

    /* A real Plan 9 main() never returns after entering schedinit(). */
    for(;;)
        __asm__ volatile ("cli; hlt" ::: "memory");
}
