/*
 * AMD64 machine ABI transition from ../pc/dat.h.
 *
 * ONLY the explicit layouts tested in tests/amd64/abi.c are validated.
 * The PCArch/Mach/PMMU interfaces still contain 386 dependencies and need
 * real long-mode implementations before linking the original Plan 9 kernel.
 */
typedef struct BIOS32si	BIOS32si;
typedef struct BIOS32ci	BIOS32ci;
typedef struct Conf	Conf;
typedef struct Confmem	Confmem;
typedef union FPsave	FPsave;
typedef struct FPssestate FPssestate;
typedef struct FPstate	FPstate;
typedef struct ISAConf	ISAConf;
typedef struct Label	Label;
typedef struct Lock	Lock;
typedef struct MMU	MMU;
typedef struct Mach	Mach;
typedef struct Notsave	Notsave;
typedef struct PCArch	PCArch;
typedef struct Pcidev	Pcidev;
typedef struct PCMmap	PCMmap;
typedef struct PCMslot	PCMslot;
typedef struct Page	Page;
typedef struct PMMU	PMMU;
typedef struct Proc	Proc;
typedef struct Segdesc	Segdesc;
typedef struct SFPssestate SFPssestate;
typedef vlong		Tval;
typedef struct Ureg	Ureg;
typedef struct Vctl	Vctl;


#include "p9embed.h"

#define MAXSYSARG	5	/* for mount(fd, afd, mpt, flag, arg) */

#define KMESGSIZE (256*1024)	/* lots, for acpi debugging */
#define STAGESIZE 2048

/*
 *  parameters for sysproc.c
 */
#define AOUT_MAGIC	(I_MAGIC)

struct Lock
{
	P9_LOCK_FIELDS;
};

/* Clang/System V AMD64 requires preserved callee-saved registers across
 * coroutine/exception context switches (386 Plan 9 saved only SP/PC).
 * Offsets are verified in tests/amd64/abi.c and used by lowlevel.S.
 */
struct Label
{
	ulong	sp;
	ulong	pc;
	ulong	bx;
	ulong	bp;
	ulong	r12;
	ulong	r13;
	ulong	r14;
	ulong	r15;
};


/*
 * FPsave.status
 */
enum
{
	/* this is a state */
	FPinit=		0,
	FPactive=	1,
	FPinactive=	2,

	/* the following is a bit that can be or'd into the state */
	FPillegal=	0x100,
};

struct FPstate
{
    P9_FPSTATE_FIELDS;
};

struct FPssestate
{
    P9_FPSS_FIELDS;
} __attribute__((aligned(16)));

struct SFPssestate
{
    P9_SFPSS_FIELDS;
};

/*
 * the FP regs must be stored here, not somewhere pointed to from here.
 * port code assumes this.
 */
union FPsave {
	P9_EMBED_FPSTATE;
	P9_EMBED_SFPSS;
};

struct Confmem
{
	ulong	base;
	ulong	npage;
	ulong	kbase;
	ulong	klimit;
};

struct Conf
{
	ulong	nmach;		/* processors */
	ulong	nproc;		/* processes */
	ulong	monitor;	/* has monitor? */
	Confmem	mem[4];		/* physical memory */
	ulong	npage;		/* total physical pages of memory */
	ulong	upages;		/* user page pool */
	ulong	nimage;		/* number of page cache image headers */
	ulong	nswap;		/* number of swap pages */
	int	nswppo;		/* max # of pageouts per segment pass */
	ulong	base0;		/* base of bank 0 */
	ulong	base1;		/* base of bank 1 */
	ulong	copymode;	/* 0 is copy on write, 1 is copy on reference */
	ulong	ialloc;		/* max interrupt time allocation in bytes */
	ulong	pipeqsize;	/* size in bytes of pipe queues */
	int	nuart;		/* number of uart devices */
};

/*
 *  MMU stuff in proc
 */
#define NCOLOR 1
struct PMMU
{
	P9_PMMU_FIELDS;
};

/*
 *  things saved in the Proc structure during a notify
 */
struct Notsave
{
	P9_NOTSAVE_FIELDS;
};

#include "../port/portdat.h"

/* Architectural AMD64 TSS: 104 bytes, including seven IST pointers. */
typedef struct __attribute__((packed)) Tss {
    u32int reserved0;
    u64int rsp0;
    u64int rsp1;
    u64int rsp2;
    u64int reserved1;
    u64int ist[7];
    u64int reserved2;
    u16int reserved3;
    u16int iomap;
} Tss;

struct Segdesc
{
	u32int d0;
	u32int d1;
};

struct Mach
{
	int	machno;			/* physical id of processor (KNOWN TO ASSEMBLY) */
	ulong	splpc;			/* pc of last caller to splhi */

	ulong*	pdb;			/* page directory base for this processor (va) */
	Tss*	tss;			/* tss for this processor */
	Segdesc	*gdt;			/* gdt for this processor */

	Proc*	proc;			/* current process on this processor */
	Proc*	externup;		/* extern register Proc *up */

	Page*	pdbpool;
	int	pdbcnt;

	ulong	ticks;			/* of the clock since boot time */
	Label	sched;			/* scheduler wakeup */
	Lock	alarmlock;		/* access to alarm list */
	void*	alarm;			/* alarms bound to this clock */
	int	inclockintr;

	Proc*	readied;		/* for runproc */
	ulong	schedticks;		/* next forced context switch */

	int	tlbfault;
	int	tlbpurge;
	int	pfault;
	int	cs;
	int	syscall;
	int	load;
	int	intr;
	int	flushmmu;		/* make current proc flush it's mmu state */
	int	ilockdepth;
	Perf	perf;			/* performance counters */

	ulong	spuriousintr;
	int	lastintr;

	int	loopconst;

	Lock	apictimerlock;
	int	cpumhz;
	uvlong	cyclefreq;		/* Frequency of user readable cycle counter */
	uvlong	cpuhz;
	int	cpuidax;
	int	cpuiddx;
	char	cpuidid[16];
	char*	cpuidtype;
	int	havetsc;
	int	havepge;
	uvlong	tscticks;
	int	pdballoc;
	int	pdbfree;
	FPsave	*fpsavalign;

	vlong	mtrrcap;
	vlong	mtrrdef;
	vlong	mtrrfix[11];
	vlong	mtrrvar[32];		/* 256 max. */

	int	stack[1];
};

/*
 * KMap the structure doesn't exist, but the functions do.
 */
typedef struct KMap		KMap;
#define	VA(k)		((void*)(k))
KMap*	kmap(Page*);
void	kunmap(KMap*);

typedef struct Active Active;
struct Active
{
	P9_EMBED_LOCK;
	int	machs;			/* bitmap of active CPUs */
	int	exiting;		/* shutdown */
	int	ispanic;		/* shutdown in response to a panic */
	int	thunderbirdsarego;	/* lets the added processors continue to schedinit */
	int	rebooting;		/* just idle cpus > 0 */
};
extern Active active;

/*
 *  routines for things outside the PC model, like power management
 */
struct PCArch
{
	char*	id;
	int	(*ident)(void);		/* this should be in the model */
	void	(*reset)(void);		/* this should be in the model */
	int	(*serialpower)(int);	/* 1 == on, 0 == off */
	int	(*modempower)(int);	/* 1 == on, 0 == off */

	void	(*intrinit)(void);
	int	(*intrenable)(Vctl*);
	int	(*intrvecno)(int);
	int	(*intrdisable)(int);
	void	(*introff)(void);
	void	(*intron)(void);

	void	(*clockenable)(void);
	uvlong	(*fastclock)(uvlong*);
	void	(*timerset)(uvlong);

	void	(*resetothers)(void);	/* put other cpus into reset */
};

/* cpuid instruction result register bits */
enum {
	/* dx */
	Fpuonchip = 1<<0,
	Vmex	= 1<<1,		/* virtual-mode extensions */
	Pse	= 1<<3,		/* page size extensions */
	Tsc	= 1<<4,		/* time-stamp counter */
	Cpumsr	= 1<<5,		/* model-specific registers, rdmsr/wrmsr */
	Pae	= 1<<6,		/* physical-addr extensions */
	Mce	= 1<<7,		/* machine-check exception */
	Cmpxchg8b = 1<<8,
	Cpuapic	= 1<<9,
	Mtrr	= 1<<12,	/* memory-type range regs.  */
	Pge	= 1<<13,	/* page global extension */
	Pse2	= 1<<17,	/* more page size extensions */
	Clflush = 1<<19,
	Mmx	= 1<<23,
	Fxsr	= 1<<24,	/* have SSE FXSAVE/FXRSTOR */
	Sse	= 1<<25,	/* thus sfence instr. */
	Sse2	= 1<<26,	/* thus mfence & lfence instr.s */
};

/*
 *  a parsed plan9.ini line
 */
#define NISAOPT		8

struct ISAConf {
	char	*type;
	ulong	port;
	int	irq;
	ulong	dma;
	ulong	mem;
	ulong	size;
	ulong	freq;

	int	nopt;
	char	*opt[NISAOPT];
};

extern PCArch	*arch;			/* PC architecture */

/*
 * Each processor sees its own Mach structure at address MACHADDR.
 * However, the Mach structures must also be available via the per-processor
 * MMU information array machp, mainly for disambiguation and access to
 * the clock which is only maintained by the bootstrap processor (0).
 */
extern Mach* machp[MAXMACH];

#define	MACHP(n)	(machp[n])

extern Mach	*m;
/* The 386 fixed MACHADDR is not mapped by the initial AMD64 tables.
 * Until GS-base/per-CPU support is implemented, use the real boot Mach.
 */
extern Mach *p9_amd64_current_mach;
#define up (p9_amd64_current_mach->externup)

/*
 *  hardware info about a device
 */
typedef struct {
	ulong	port;
	int	size;
} Devport;

struct DevConf
{
	ulong	intnum;			/* interrupt number */
	char	*type;			/* card type, malloced */
	int	nports;			/* Number of ports */
	Devport	*ports;			/* The ports themselves */
};

typedef struct BIOS32ci {		/* BIOS32 Calling Interface */
	u32int	eax;
	u32int	ebx;
	u32int	ecx;
	u32int	edx;
	u32int	esi;
	u32int	edi;
} BIOS32ci;
