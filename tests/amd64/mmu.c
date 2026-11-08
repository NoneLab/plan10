/*
 * Host-executed test of the genuine pc64/mmu.c page-table implementation.
 * Only the low-level physical-page allocator and physical/virtual address
 * translations are mocked; the MMU walker itself is the kernel's code.
 * No privileged instructions or substitute kernels are executed.
 */
#include "u.h"
#include "../../sys/src/9/port/lib.h"
#include "../../sys/src/9/pc64/mem.h"
#include "../../sys/src/9/pc64/dat.h"
#include "../../sys/src/9/pc64/fns.h"

#define NPAGES 16
#define PHYSBASE 0x200000UL
static uchar physical[NPAGES * BY2PG] __attribute__((aligned(BY2PG)));
static uint pages;

void *
rampage(void)
{
    if(pages == NPAGES)
        return nil;
    return &physical[pages++ * BY2PG];
}

ulong
paddr(void *va)
{
    uintptr start = (uintptr)physical;
    uintptr addr = (uintptr)va;
    if(addr < start || addr >= start + sizeof(physical))
        return ~0UL;
    return PHYSBASE + addr - start;
}

void *
kaddr(ulong pa)
{
    if(pa < PHYSBASE || pa >= PHYSBASE + sizeof(physical))
        return nil;
    return physical + (pa - PHYSBASE);
}

extern int mmumap2m(ulong*, ulong, ulong, ulong);
extern int mmumap4k(ulong*, ulong, ulong, ulong);

static int
check(int ok, int failcode)
{
    return ok ? 0 : failcode;
}

int
main(void)
{
    ulong *pml4, *pte, *pde;
    ulong high, low;
    int result;

    high = KZERO + 0x00400000UL;
    low = 0x00400000UL;
    pml4 = rampage();

    if((result = check(pml4 != nil, 1)) != 0)
        return result;
    if((result = check(mmuwalk(pml4, high, 2, 0) == nil, 2)) != 0)
        return result;
    if((result = check(mmumap4k(pml4, high + 0x1000UL,
                              0x3000UL, PTEWRITE | PTEGLOBAL) == 0, 3)) != 0)
        return result;

    pte = mmuwalk(pml4, high + 0x1000UL, 2, 0);
    if((result = check(pte && (*pte & PTEVALID) &&
                       (*pte & PTEWRITE) && (*pte & PTEGLOBAL) &&
                       PPN(*pte) == 0x3000UL, 4)) != 0)
        return result;
    if((result = check(mmumap4k(pml4, high + 0x1000UL,
                              0x3000UL, PTEWRITE) == -1, 5)) != 0)
        return result;
    if((result = check(mmumap2m(pml4, high + 0x200000UL,
                              0x600000UL, PTEWRITE) == 0, 6)) != 0)
        return result;

    pde = mmuwalk(pml4, high + 0x200000UL, 1, 0);
    if((result = check(pde && (*pde & PTESIZE), 7)) != 0)
        return result;
    if((result = check(mmuwalk(pml4, high + 0x200000UL, 2, 0) == nil, 8)) != 0)
        return result;
    if((result = check(mmumap4k(pml4, low, 0x4000, PTEUSER) == 0, 9)) != 0)
        return result;
    if((result = check((pml4[PML4X(low)] & PTEUSER) != 0, 10)) != 0)
        return result;
    if((result = check(mmumap4k(pml4, 0x0000800000000000ULL,
                              0x5000, PTEUSER) == -1, 11)) != 0)
        return result;
    if((result = check(mmumap4k(pml4, high + 7, 0x6000, 0) == -1, 12)) != 0)
        return result;
    if((result = check(mmuwalk(pml4, high, 5, 0) == nil, 13)) != 0)
        return result;

    return 0;
}
