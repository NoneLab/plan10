/*
 * Native 16550 UART polling console for Plan 9/pc64 early kernel output.
 *
 * QEMU's default ISA COM1 uses 0x3f8.  After device initialisation, the
 * original Plan 9 devcons.c remains responsible for formatting and kmesg.
 */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

#define COM1         0x3f8
#define REG_DATA     0
#define REG_IER      1
#define REG_LCR      3
#define REG_LSR      5
#define REG_FCR      2
#define REG_MCR      4
#define TX_READY     0x20

extern void (*screenputs)(char *, int);

static void
serialputc(int c)
{
    int i;

    /* A missing or unconfigured serial device must not wedge panic(). */
    for(i = 0; i < 100000; i++)
        if(inb(COM1 + REG_LSR) & TX_READY){
            outb(COM1 + REG_DATA, c);
            return;
        }
}

static void
serialputs(char *s, int n)
{
    while(n-- > 0){
        if(*s == '\n')
            serialputc('\r');
        serialputc((uchar)*s++);
    }
}

void
p9_amd64_serialinit(void)
{
    outb(COM1 + REG_IER, 0);
    outb(COM1 + REG_LCR, 0x80);
    outb(COM1 + REG_DATA, 1);  /* 115200 baud / 1 */
    outb(COM1 + REG_IER, 0);
    outb(COM1 + REG_LCR, 3);   /* 8N1 */
    outb(COM1 + REG_FCR, 0xc7);
    outb(COM1 + REG_MCR, 3);
    screenputs = serialputs;
}

/* The original devcons.c sends to its selected serial backend, if any. */
void
uartputs(char *s, int n)
{
    serialputs(s, n);
}

/* Polling debugger input; used by the original Plan 9 /port/rdb code. */
int
uartgetc(void)
{
    while((inb(COM1 + REG_LSR) & 1) == 0)
        __asm__ volatile("pause");
    return inb(COM1 + REG_DATA) & 0xff;
}
