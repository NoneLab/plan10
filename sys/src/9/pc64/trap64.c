/*
 * AMD64 interrupt descriptor table for the real Plan 9 kernel.
 *
 * This initializes real hardware IDT entries and catches CPU exceptions.
 * It is not yet a replacement for ../pc/trap.c: IRQ routing, page faults,
 * Plan 9 notes and syscall/notify/noted will follow in the next stage.
 */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "ureg.h"

typedef struct __attribute__((packed)) IdtGate IdtGate;
typedef struct __attribute__((packed)) IdtRegister IdtRegister;

struct IdtGate {
    u16int offset_low;
    u16int selector;
    u8int ist;
    u8int attr;
    u16int offset_middle;
    u32int offset_high;
    u32int reserved;
};

struct IdtRegister {
    u16int limit;
    u64int base;
};

_Static_assert(sizeof(IdtGate) == 16, "AMD64 IDT gate must be 16 bytes");
_Static_assert(sizeof(IdtRegister) == 10, "AMD64 IDTR must be 10 bytes");

extern void (*const p9_amd64_vectors[256])(void);
extern void p9_amd64_lidt(const IdtRegister*);

static IdtGate idt[256] __attribute__((aligned(16)));
static IdtRegister idtr;

void
trapinit0(void)
{
    int i;
    u64int addr;

    for(i = 0; i < 256; i++){
        addr = (u64int)(uintptr)p9_amd64_vectors[i];
        idt[i].offset_low = addr;
        idt[i].selector = KESEL;
        idt[i].ist = (i == 8) ? 1 : 0; /* #DF gets its own IST */
        idt[i].attr = 0x8e; /* present, DPL=0, interrupt gate */
        idt[i].offset_middle = addr >> 16;
        idt[i].offset_high = addr >> 32;
        idt[i].reserved = 0;
    }

    idtr.limit = sizeof(idt) - 1;
    idtr.base = (u64int)(uintptr)idt;
    p9_amd64_lidt(&idtr);
}

/*
 * The first real IRQ path: rearmed PIT channel 0 and the original Plan 9
 * /port timer queue.  Other hardware IRQs and userspace faults remain
 * fail-closed pending their native interrupt/notes implementations.
 */
void
p9_amd64_trap(Ureg *ur)
{
    if(ur->type == 32){
        outb(0x20, 0x20);       /* master PIC EOI, IRQ0 */
        timerintr(ur, 0);
        return;
    }
    panic("amd64 exception/irq: vector %llud err %#llux ip %#llux cs %#llux",
        (uvlong)ur->type, (uvlong)ur->error, (uvlong)ur->ip,
        (uvlong)ur->cs);
}
