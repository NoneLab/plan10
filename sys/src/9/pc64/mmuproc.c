/*
 * AMD64 Plan 9 process page tables.
 *
 * The 386 VPT trick is deliberately not carried over: IA-32e uses four
 * paging levels.  User mappings are owned by a Proc; upper-half mappings
 * are shared with the kernel and never released by the process.
 *
 * This is the kernel's native PMMU, not a separate test kernel.  On the
 * initial bring-up only RAM covered by the direct map can be used.
 */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

#define PAGEADDR 0x000ffffffffff000ULL
#define MAXUSERVA (1ULL << 47)

static void
recycleused(Proc *p)
{
    Page *q, *next;
    ulong *root;

    if(p->mmupdb == nil)
        return;

    root = kaddr(p->mmupdb->pa);
    if(root == nil)
        panic("pc64: process PML4 outside direct map");

    /* User PML4 entries; the kernel part remains byte-for-byte identical. */
    memset(root, 0, 256*sizeof(*root));
    for(q = p->mmuused; q != nil; q = next){
        next = q->next;
        q->next = p->mmufree;
        p->mmufree = q;
    }
    p->mmuused = nil;
}

static Page*
newptpage(Proc *p)
{
    Page *q;

    if(p->mmufree != nil){
        q = p->mmufree;
        p->mmufree = q->next;
    }else
        q = newpage(0, nil, 0);

    if(q == nil || kaddr(q->pa) == nil)
        panic("pc64: cannot allocate user page-table page");
    memset(kaddr(q->pa), 0, BY2PG);
    q->next = p->mmuused;
    p->mmuused = q;
    return q;
}

static void
allocroot(Proc *p)
{
    Page *q;
    ulong *table;

    if(p->mmupdb != nil)
        return;
    q = newpage(0, nil, 0);
    if(q == nil || kaddr(q->pa) == nil)
        panic("pc64: cannot allocate process PML4");

    table = kaddr(q->pa);
    memmove(table, m->pdb, BY2PG);
    memset(table, 0, 256*sizeof(*table));
    p->mmupdb = q;
}

static ulong*
walkuser(Proc *p, ulong va)
{
    int levels[4];
    int i;
    ulong *table, entry;
    Page *newpg;

    levels[0] = PML4X(va);
    levels[1] = PDPX(va);
    levels[2] = PDX(va);
    levels[3] = PTX(va);

    table = kaddr(p->mmupdb->pa);
    for(i = 0; i < 3; i++){
        entry = table[levels[i]];
        if((entry & PTEVALID) == 0){
            newpg = newptpage(p);
            entry = newpg->pa | PTEUSER | PTEWRITE | PTEVALID;
            table[levels[i]] = entry;
        }
        if(entry & PTESIZE)
            panic("pc64: unexpected huge user page");
        table = kaddr(entry & PAGEADDR);
        if(table == nil)
            panic("pc64: user page table outside direct map");
    }
    return &table[levels[3]];
}

void
mmuinit(void)
{
    /* The bootstrap PML4 is the initial kernel address space. */
    if(m->pdb == nil)
        panic("pc64: bootstrap PML4 missing");
    putcr3(paddr(m->pdb));
}

void
flushmmu(void)
{
    int s;

    if(up == nil)
        return;
    s = splhi();
    up->newtlb = 1;
    mmuswitch(up);
    splx(s);
}

void
mmuswitch(Proc *p)
{
    ulong rootpa;

    if(p == nil)
        panic("pc64: nil process in mmuswitch");
    if(p->newtlb){
        recycleused(p);
        p->newtlb = 0;
    }
    rootpa = p->mmupdb ? p->mmupdb->pa : paddr(m->pdb);
    p9_amd64_tss_rsp0((ulong)(uintptr)p->kstack + KSTACK);
    putcr3(rootpa);
}

void
putmmu(ulong va, ulong pa, Page *pg)
{
    ulong *pte;
    int s;

    USED(pg);
    if(up == nil || va >= MAXUSERVA || (va & (BY2PG - 1)))
        panic("pc64: invalid user putmmu address");
    if((pa & PAGEADDR) >= (1ULL << 30))
        panic("pc64: physical page outside direct map");

    if(up->mmupdb == nil)
        allocroot(up);

    /* Page allocation may sleep, so it cannot happen under splhi(). */
    pte = walkuser(up, va);
    s = splhi();
    *pte = (pa & (PAGEADDR | PTEWRITE | PTEUNCACHED | PTEWT))
         | PTEVALID | PTEUSER;
    if(getcr3() != up->mmupdb->pa)
        putcr3(up->mmupdb->pa);
    else
        invlpg(va);
    splx(s);
}

void
checkmmu(ulong va, ulong pa)
{
    ulong *pte;

    if(up == nil || up->mmupdb == nil || va >= MAXUSERVA)
        return;
    /* No new tables during a debugging check. */
    pte = mmuwalk(kaddr(up->mmupdb->pa), va, 2, 0);
    if(pte != nil && (*pte & PTEVALID) && ((*pte & PAGEADDR) != pa))
        print("pc64: page mismatch va %#lux physical %#lux expected %#lux\n",
              va, *pte & PAGEADDR, pa);
}

KMap*
kmap(Page *p)
{
    if(p == nil || kaddr(p->pa) == nil)
        panic("pc64: kmap page unavailable");
    return (KMap*)kaddr(p->pa);
}

void
kunmap(KMap *k)
{
    USED(k);   /* Mapping is permanent in the direct-map window. */
}

void*
tmpmap(Page *p)
{
    return (void*)kmap(p);
}

void
tmpunmap(void *p)
{
    USED(p);
}

void
mmurelease(Proc *p)
{
    Page *q, *next;

    /* Called from schedinit with palloc locked, as on 386. */
    putcr3(paddr(m->pdb));
    recycleused(p);
    if(p->mmupdb != nil){
        q = p->mmupdb;
        p->mmupdb = nil;
        if(--q->ref != 0)
            panic("pc64: process PML4 page still referenced");
        pagechainhead(q);
    }
    for(q = p->mmufree; q != nil; q = next){
        next = q->next;
        if(--q->ref != 0)
            panic("pc64: process PT page still referenced");
        pagechainhead(q);
    }
    p->mmufree = nil;
}
