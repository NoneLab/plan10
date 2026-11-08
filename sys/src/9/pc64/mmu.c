/*
 * Plan 9 AMD64 four-level page-table walker.
 *
 * Retain the meaning of the original pc/mmu.c interface:
 *   level 1: PDE (2 MiB mapping / pointer to a page table)
 *   level 2: PTE (4 KiB mapping)
 * The upper PML4 and PDPT levels are traversed internally.
 *
 * This is not a separate bootstrap kernel.  It is the page-table part of
 * the original Plan 9 machine abstraction, using its rampage(), kaddr()
 * and paddr() allocator/address-translation contracts.  The early
 * E820 memory allocator now supplies page table pages on real boot.
 */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

#define AMD64_PTEADDR  0x000ffffffffff000ULL
#define P9_BIGPG       (2UL * 1024 * 1024)

/* Explicitly reject non-canonical addresses, including the LA57 range. */
static int
canonical(ulong va)
{
    ulong upper;

    upper = va >> 48;
    return upper == ((va & (1ULL << 47)) ? 0xffffULL : 0);
}

/*
 * Return an existing table or insert a zeroed page-table page.
 * Every table allocation is a physical page obtained from Plan 9's
 * low-level RAM allocator, never the normal heap.
 */
static ulong *
nexttable(ulong *table, int index, ulong va, int create)
{
    ulong *entry;
    void *page;
    ulong pa, flags;

    entry = &table[index];
    if((*entry & PTEVALID) == 0){
        if(!create)
            return nil;

        page = rampage();
        if(page == nil)
            return nil;

        pa = paddr(page);
        if((pa & (BY2PG - 1)) != 0 || (pa & ~AMD64_PTEADDR) != 0)
            return nil;

        memset(page, 0, BY2PG);
        flags = PTEVALID | PTEWRITE;
        if((va >> 47) == 0)
            flags |= PTEUSER;
        *entry = (pa & AMD64_PTEADDR) | flags;
    }

    if((*entry & PTESIZE) != 0)
        return nil;

    return kaddr(*entry & AMD64_PTEADDR);
}

ulong *
mmuwalk(ulong *pml4, ulong va, int level, int create)
{
    ulong *table, *pde;

    if(pml4 == nil || !canonical(va) || (level != 1 && level != 2))
        return nil;

    table = nexttable(pml4, PML4X(va), va, create);
    if(table == nil)
        return nil;

    table = nexttable(table, PDPX(va), va, create);
    if(table == nil)
        return nil;

    pde = &table[PDX(va)];
    if(level == 1)
        return pde;

    table = nexttable(table, PDX(va), va, create);
    if(table == nil)
        return nil;

    return &table[PTX(va)];
}

/*
 * Map one 2 MiB physical frame in the kernel address space.
 * Returns -1 on conflicting mappings, noncanonical addresses, or when
 * rampage() cannot provide a page table.  No silent remapping.
 */
int
mmumap2m(ulong *pml4, ulong va, ulong pa, ulong flags)
{
    ulong *pde;

    if(((va | pa) & (P9_BIGPG - 1)) != 0)
        return -1;
    if((pa & ~AMD64_PTEADDR) != 0)
        return -1;
    if(!canonical(va))
        return -1;

    pde = mmuwalk(pml4, va, 1, 1);
    if(pde == nil || (*pde & PTEVALID))
        return -1;

    *pde = (pa & AMD64_PTEADDR)
        | (flags & (PTEWRITE | PTEUSER | PTEWT | PTEUNCACHED | PTEGLOBAL))
        | PTEVALID | PTESIZE;
    return 0;
}

int
mmumap4k(ulong *pml4, ulong va, ulong pa, ulong flags)
{
    ulong *pte;

    if(((va | pa) & (BY2PG - 1)) != 0)
        return -1;
    if((pa & ~AMD64_PTEADDR) != 0 || !canonical(va))
        return -1;

    pte = mmuwalk(pml4, va, 2, 1);
    if(pte == nil || (*pte & PTEVALID))
        return -1;

    *pte = (pa & AMD64_PTEADDR)
        | (flags & (PTEWRITE | PTEUSER | PTEWT | PTEUNCACHED | PTEGLOBAL))
        | PTEVALID;
    return 0;
}
