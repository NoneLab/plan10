/*
 * AMD64 monotonic timestamps for Plan 9 /port timer interfaces.
 * QEMU TSC frequency is calibrated against the PC PIT channel 2 before
 * enabling interrupt-driven timers.  The PIT is an ISA device, not a
 * host-userland clock or a separate bootstrap implementation.
 */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

#define PIT_HZ      1193182UL
#define PIT_GATE    0x61
#define PIT_COUNTER 0x42
#define PIT_MODE    0x43

extern void p9_amd64_rdtsc(uvlong *);

static uvlong tickspersecond;

/* Channel 2 one-shot count calibration: around 20 ms. */
void
p9_amd64_clockinit(void)
{
    uvlong start, stop;
    ulong ctr;
    int i;
    uchar gate;

    gate = inb(PIT_GATE);
    outb(PIT_GATE, (gate & ~2) | 1);
    outb(PIT_MODE, 0xb0);      /* channel 2, lobyte/hibyte, mode 0 */
    ctr = PIT_HZ / 50;
    outb(PIT_COUNTER, ctr & 255);
    outb(PIT_COUNTER, ctr >> 8);
    p9_amd64_rdtsc(&start);
    for(i = 0; i < 100000000; i++)
        if(inb(PIT_GATE) & 0x20)
            break;
    p9_amd64_rdtsc(&stop);
    outb(PIT_GATE, gate);

    if(i == 100000000 || stop <= start)
        panic("pc64: PIT/clock calibration failed");
    tickspersecond = (stop - start) * 50;
    if(tickspersecond == 0)
        panic("pc64: no TSC frequency");
}

uvlong
fastticks(uvlong *hz)
{
    uvlong v;

    if(hz != nil)
        *hz = tickspersecond;
    p9_amd64_rdtsc(&v);
    return v;
}

ulong
perfticks(void)
{
    return (ulong)fastticks(nil);
}

void
microdelay(int us)
{
    uvlong start, until;

    if(us <= 0 || tickspersecond == 0)
        return;
    start = fastticks(nil);
    until = (tickspersecond / 1000000ULL) * us;
    while(fastticks(nil) - start < until)
        __asm__ volatile("pause");
}

void
delay(int ms)
{
    while(ms-- > 0)
        microdelay(1000);
}

/*
 * Program PIT channel 0 in interrupting one-shot mode.  A long delay is
 * broken up into <=65535 PIT cycles; timerintr() reprograms the remainder.
 * The /port timer queue remains the sole scheduler of timer events.
 */
void
timerset(vlong next)
{
    uvlong now, delta, cycles, divider;

    if(tickspersecond == 0)
        panic("pc64: timerset before calibration");
    now = fastticks(nil);
    delta = (uvlong)next > now ? (uvlong)next - now : 0;
    divider = tickspersecond / PIT_HZ;
    if(divider == 0)
        panic("pc64: TSC is slower than PIT");
    cycles = delta / divider;
    if(cycles < 2)
        cycles = 2;
    if(cycles > 65535)
        cycles = 65535;

    outb(0x43, 0x30);   /* channel 0, 16-bit, mode 0 one-shot */
    outb(0x40, cycles & 0xff);
    outb(0x40, cycles >> 8);
}

void
p9_amd64_picinit(void)
{
    /* Rebase the PC's dual 8259 to IDT vectors 0x20..0x2f. */
    outb(0x21, 0xff);
    outb(0xa1, 0xff);
    outb(0x20, 0x11);
    outb(0xa0, 0x11);
    outb(0x21, 0x20);
    outb(0xa1, 0x28);
    outb(0x21, 0x04);
    outb(0xa1, 0x02);
    outb(0x21, 0x01);
    outb(0xa1, 0x01);
    outb(0x21, 0xff);
    outb(0xa1, 0xff);
}

void
p9_amd64_irqstart(void)
{
    /* Only the PIT is wired; do not unmask device IRQs without drivers. */
    outb(0x21, 0xfe);
    spllo();
}

ulong
µs(void)
{
    uvlong t;

    if(tickspersecond == 0)
        return 0;
    t = fastticks(nil);
    return (ulong)(t / (tickspersecond / 1000000ULL));
}
