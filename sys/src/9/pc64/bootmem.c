/*
 * Initial Plan 9/pc64 physical RAM handoff from the Multiboot-v1 loader.
 *
 * This supplies the RAM page allocator and PA<->KZERO conversions required
 * by the real /port MMU and memory initialisation. It is not a bootstrap
 * kernel: pages are only supplied to genuine Plan 9 kernel components.
 *
 * Limitations: until the final MMU takes over, only the low 1GiB is mapped.
 * Pages occupied by the kernel are reserved, and the Multiboot E820 map is
 * checked before returning a physical page. Kernel page-table ownership and
 * process MMUs still need integration with the /port Page structures.
 */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

#define EARLY_MAP_LIMIT (1UL << 30)
#define MB_MEMORY_INFO  (1U << 0)
#define MB_MEMORY_MAP   (1U << 6)
#define MB_RAM          1U

typedef struct __attribute__((packed)) Multiboot1 Multiboot1;
typedef struct __attribute__((packed)) MbootMemmap MbootMemmap;

struct Multiboot1 {
    u32int flags;
    u32int mem_lower;
    u32int mem_upper;
    u32int boot_device;
    u32int command_line;
    u32int modules_count;
    u32int modules_address;
    u32int symbols[4];
    u32int mmap_length;
    u32int mmap_address;
};

struct MbootMemmap {
    u32int size;
    u64int address;
    u64int length;
    u32int type;
};

_Static_assert(__builtin_offsetof(Multiboot1, mmap_length) == 44,
               "Multiboot-v1 mmap_length offset");
_Static_assert(__builtin_offsetof(Multiboot1, mmap_address) == 48,
               "Multiboot-v1 mmap_address offset");
_Static_assert(sizeof(MbootMemmap) == 24,
               "Multiboot-v1 E820 entry minimum size");

extern uchar __kernel_end[];
extern uchar p9_boot_pml4[];

static ulong nextphysical;
static ulong earlylimit;
static ulong tablelimit;
static Multiboot1 *mb;

/* The higher-half address map is KZERO plus physical address for 0..1GiB. */
void *
kaddr(ulong pa)
{
    if(pa >= EARLY_MAP_LIMIT)
        return nil;
    return (void *)(uintptr)(KZERO + pa);
}

ulong
paddr(void *p)
{
    ulong va;

    va = (ulong)(uintptr)p;
    if(va >= KZERO && va < KZERO + EARLY_MAP_LIMIT)
        return va - KZERO;
    if(va < EARLY_MAP_LIMIT)
        return va;
    return (ulong)-1;
}

ulong
cankaddr(ulong pa)
{
    if(pa >= EARLY_MAP_LIMIT)
        return 0;
    return EARLY_MAP_LIMIT - pa;
}

static int
isavailableram(ulong pa)
{
    ulong addr, end, next;
    MbootMemmap *e;

    if(mb != nil && (mb->flags & MB_MEMORY_MAP) != 0){
        addr = mb->mmap_address;
        end = addr + mb->mmap_length;
        if(end < addr || end > EARLY_MAP_LIMIT)
            return 0;
        while(addr < end){
            e = kaddr(addr);
            if(e == nil || end - addr < sizeof(u32int) || e->size < 20)
                return 0;
            next = addr + sizeof(u32int) + e->size;
            if(next <= addr || next > end)
                return 0;
            if(e->type == MB_RAM && (u64int)pa >= e->address
            && (u64int)(pa + BY2PG) <= e->address + e->length)
                return 1;
            addr = next;
        }
        return 0;
    }
    return pa + BY2PG <= earlylimit;
}

/* Called from the entry trampoline before the genuine Plan 9 main(). */
void
p9_amd64_memsetup(ulong mbi)
{
    ulong pa, reported;

    mb = kaddr(mbi);
    pa = ((ulong)(uintptr)__kernel_end - KZERO + BY2PG - 1)
       & ~(BY2PG - 1);
    if(pa < 0x200000 || pa >= EARLY_MAP_LIMIT)
        for(;;)
            __asm__ volatile ("cli; hlt");
    nextphysical = pa;
    /* Reserve 16 MiB exclusively for early page-table allocations. */
    tablelimit = pa + 16 * MiB;
    if(tablelimit > EARLY_MAP_LIMIT)
        tablelimit = EARLY_MAP_LIMIT;

    if(mb != nil && (mb->flags & MB_MEMORY_INFO) != 0){
        reported = ((ulong)mb->mem_upper << 10) + 0x100000;
        earlylimit = reported < EARLY_MAP_LIMIT ? reported : EARLY_MAP_LIMIT;
    }else
        earlylimit = 0;
}

/* Early page-table allocation for mmuwalk(), not a general kernel heap. */
void *
rampage(void)
{
    ulong pa;
    u64int *page;
    usize i;

    while(nextphysical < tablelimit - BY2PG){
        pa = nextphysical;
        nextphysical += BY2PG;
        if(!isavailableram(pa))
            continue;
        page = kaddr(pa);
        if(page == nil)
            return nil;
        for(i = 0; i < BY2PG / sizeof(*page); i++)
            page[i] = 0;
        return page;
    }
    return nil;
}

/* Temporary bootstrap page table; the real per-process tables replace it. */
ulong *
p9_amd64_bootpml4(void)
{
    return kaddr((ulong)(uintptr)p9_boot_pml4);
}

/*
 * Expose the largest RAM interval disjoint from the image and reserved
 * early paging pool.  The /port page allocator and xalloc own that region
 * after meminit(), never the early boot allocator.
 *
 * At this stage memory above the direct-mapped 1 GiB window cannot be
 * handed to the /port allocator; it is deliberately not misrepresented.
 */
int
p9_amd64_memregion(ulong *start, ulong *limit)
{
    ulong base, end, p, stop, candidate, bestbase, bestend;
    MbootMemmap *e;
    u64int top;

    if(start == nil || limit == nil)
        return -1;
    bestbase = bestend = 0;

    if(mb != nil && (mb->flags & MB_MEMORY_MAP) != 0){
        p = mb->mmap_address;
        stop = p + mb->mmap_length;
        if(stop < p || stop > EARLY_MAP_LIMIT)
            return -1;
        while(p < stop){
            if(stop - p < sizeof(u32int))
                return -1;
            e = kaddr(p);
            if(e == nil || e->size < 20)
                return -1;
            candidate = p + sizeof(u32int) + e->size;
            if(candidate <= p || candidate > stop)
                return -1;
            if(e->type == MB_RAM && e->address < EARLY_MAP_LIMIT){
                top = e->address + e->length;
                if(top < e->address || top > EARLY_MAP_LIMIT)
                    top = EARLY_MAP_LIMIT;
                base = e->address > tablelimit ? e->address : tablelimit;
                base = (base + BY2PG - 1) & ~(BY2PG - 1);
                end = (ulong)top & ~(BY2PG - 1);
                if(end > base && end - base > bestend - bestbase){
                    bestbase = base;
                    bestend = end;
                }
            }
            p = candidate;
        }
    }else{
        bestbase = (tablelimit + BY2PG - 1) & ~(BY2PG - 1);
        bestend = earlylimit & ~(BY2PG - 1);
    }
    if(bestend <= bestbase)
        return -1;
    *start = bestbase;
    *limit = bestend;
    return 0;
}
