/*
 * Native long-mode GDT/TSS for the Plan 9 AMD64 machine port.
 * No BIOS-compatibility GDT or protected-mode bootstrap is fabricated.
 * The genuine kernel entry path must call mmuinit0() after enabling the
 * final higher-half address map and arranging a valid 64-bit CS.
 */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

typedef struct __attribute__((packed)) DescriptorRegister DescriptorRegister;
struct DescriptorRegister {
    u16int limit;
    u64int base;
};

_Static_assert(sizeof(DescriptorRegister) == 10, "lgdt descriptor must be 10 bytes");

extern void p9_amd64_lgdt(const DescriptorRegister*);
extern void p9_amd64_ltr(u16int);
extern void p9_amd64_reload_segments(void);
extern uchar p9_amd64_early_stack_end[];

/* A dedicated IST stack prevents double-fault handling on a blown stack. */
static uchar doublefaultstack[4 * BY2PG] __attribute__((aligned(16)));

static u64int gdt[NGDT] __attribute__((aligned(16)));
static Tss boot_tss __attribute__((aligned(16)));
static DescriptorRegister gdtr;

static void
settss(u64int base, u32int limit)
{
    /* System TSS occupies two consecutive 64-bit GDT slots. */
    gdt[TSSSEG] = (limit & 0xffffU)
        | ((base & 0xffffffULL) << 16)
        | (0x89ULL << 40)
        | (((u64int)(limit >> 16) & 0x0fULL) << 48)
        | (((base >> 24) & 0xffULL) << 56);
    gdt[TSSSEG + 1] = base >> 32;
}

void
mmuinit0(void)
{
    memset(gdt, 0, sizeof(gdt));
    memset(&boot_tss, 0, sizeof(boot_tss));

    /* IA-32e code segment has L=1, D=0. */
    gdt[KDSEG] = 0x00cf92000000ffffULL;
    gdt[KESEG] = 0x00af9a000000ffffULL;
    gdt[UDSEG] = 0x00cff2000000ffffULL;
    gdt[UESEG] = 0x00affa000000ffffULL;

    /* No I/O bitmap: userspace must not access hardware I/O ports. */
    boot_tss.iomap = sizeof(Tss);
    boot_tss.rsp0 = (u64int)(uintptr)p9_amd64_early_stack_end;
    boot_tss.ist[0] = (u64int)(uintptr)(doublefaultstack + sizeof doublefaultstack);
    settss((u64int)(uintptr)&boot_tss, sizeof(Tss) - 1);

    gdtr.limit = sizeof(gdt) - 1;
    gdtr.base = (u64int)(uintptr)gdt;
    p9_amd64_lgdt(&gdtr);
    p9_amd64_reload_segments();
    p9_amd64_ltr(TSSSEL);
}

/* Must be set to a valid ring-0 stack before permitting user transitions. */
void
p9_amd64_tss_rsp0(ulong top)
{
    boot_tss.rsp0 = top;
}
