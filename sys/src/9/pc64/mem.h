#ifndef P9_PC64_MEM_H
#define P9_PC64_MEM_H

/*
 * Native AMD64 address-space definitions for the Plan 9 port.
 * These are the intended ABI constants, not a bootable MMU implementation.
 * A complete page-table manager and entry sequence are still required.
 */
#define BI2BY       8
#define BI2WD       64
#define BY2WD       8
#define BY2V        8
#define BY2PG       4096
#define PGSHIFT     12
#define WD2PG       (BY2PG / BY2WD)
#define CACHELINESZ 64
#define BLOCKALIGN  8
#define FPalign     16
/* Power-of-two byte units used by the original port allocators. */
#define KiB (1024UL)
#define MiB (1024UL * KiB)
#define GiB (1024UL * MiB)

#define KSTACK      (16 * 1024)
#define MAXMACH     32

#define P9_VECTOR_SYSCALL 0x40

#define HZ          100
#define MS2HZ       (1000 / HZ)
#define TK2SEC(t)   ((t) / HZ)

/* A canonical high-half kernel mapping, independent of 386 KZERO. */
#define KZERO       0xffffffff80000000ULL
#define KTZERO      (KZERO + 0x00200000ULL) /* matches pc64/kernel.ld */
#define UZERO       0ULL
#define UTZERO      (UZERO + BY2PG)
#define USTKTOP     0x00007fffffffe000ULL
#define USTKSIZE    (16 * 1024 * 1024)
#define TSTKTOP     (USTKTOP - USTKSIZE)
#define TSTKSIZ     100
#define UTROUND(t)  ROUNDUP((t), BY2PG)

/* Reserved for a future per-CPU Mach mapping; boot code uses a global Mach. */
#define MACHADDR    (KZERO + 0x16000ULL)
#define MACHSIZE    BY2PG

/* AMD64 page-table entries use 64-bit values and 512 entries per level. */
#define SEGMAPSIZE  1984
#define SSEGMAPSIZE 16
#define PTEPERTAB   512
#define PTEMAPMEM   (PTEPERTAB * BY2PG)
#define PTEVALID    (1ULL << 0)
#define PTEWRITE    (1ULL << 1)
#define PTEUSER     (1ULL << 2)
#define PTEWT       (1ULL << 3)
#define PTEUNCACHED (1ULL << 4)
#define PTESIZE     (1ULL << 7)
#define PTEGLOBAL   (1ULL << 8)
#define PTERONLY    0ULL
#define PTEKERNEL   0ULL

#define PML4X(a)    (((u64int)(a) >> 39) & 0x1ff)
#define PDPX(a)     (((u64int)(a) >> 30) & 0x1ff)
#define PDX(a)      (((u64int)(a) >> 21) & 0x1ff)
#define PTX(a)      (((u64int)(a) >> 12) & 0x1ff)
#define PPN(x)      ((x) & ~(BY2PG - 1))
#define getpgcolor(a) 0

/* Old PC-compatible I/O and descriptor selectors, pending trap port. */
#define NULLSEG     0
#define KDSEG       1
#define KESEG       2
#define UDSEG       3
#define UESEG       4
#define TSSSEG      5
#define NGDT        7
#define SELECTOR(i,t,p) (((i) << 3) | (t) | (p))
#define SELGDT      0
#define KDSEL       SELECTOR(KDSEG, SELGDT, 0)
#define KESEL       SELECTOR(KESEG, SELGDT, 0)
#define UESEL       SELECTOR(UESEG, SELGDT, 3)
#define UDSEL       SELECTOR(UDSEG, SELGDT, 3)
#define TSSSEL      SELECTOR(TSSSEG, SELGDT, 0)

#define P9_PROC_ALIGN 16

#endif
