/*
 * Clang / ISO C11 representation of Plan 9 anonymous embedded structs.
 *
 * The Plan 9 compiler permits `Lock;` inside a structure and permits
 * transparent access through the embedded type.  Clang does not provide
 * this facility without Microsoft extensions (which this port forbids).
 *
 * An anonymous union of the original type and a C11 anonymous struct
 * exposing its fields preserves a typed member and its prefix layout.
 * Pointers to enclosing types still require explicit casts at call sites.
 *
 * These definitions are for the x86-64 machine port only.  Legacy Plan 9
 * machine builds continue using the original declarations.
 */
#ifndef P9_AMD64_EMBED_H
#define P9_AMD64_EMBED_H

#if !defined(__clang__) || !defined(__x86_64__)
#error "p9embed.h is only for the Clang AMD64 kernel port"
#endif

#define P9_LOCK_FIELDS \
    ulong key; \
    ulong sr; \
    ulong pc; \
    Proc *p; \
    Mach *m; \
    ushort isilock; \
    long lockcycles

/* The visible overlay renames conflicting shadowed members. */
#define P9_LOCK_VISIBLE_FIELDS \
    ulong key; \
    ulong sr; \
    ulong _p9_lock_pc; \
    Proc *_p9_lock_proc; \
    Mach *m; \
    ushort isilock; \
    long lockcycles
#define P9_EMBED_LOCK \
    union { Lock _p9_lock; struct { P9_LOCK_VISIBLE_FIELDS; }; }

#define P9_REF_FIELDS \
    P9_EMBED_LOCK; \
    long ref
#define P9_EMBED_REF \
    union { Ref _p9_ref; struct { P9_REF_FIELDS; }; }

#define P9_RENDEZ_FIELDS \
    P9_EMBED_LOCK; \
    Proc *p
#define P9_EMBED_RENDEZ \
    union { Rendez _p9_rendez; struct { P9_RENDEZ_FIELDS; }; }

#define P9_QLOCK_FIELDS \
    Lock use; \
    Proc *_p9_qlock_head; \
    Proc *tail; \
    int locked; \
    uintptr qpc
#define P9_EMBED_QLOCK \
    union { QLock _p9_qlock; struct { P9_QLOCK_FIELDS; }; }

#define P9_RWLOCK_FIELDS \
    Lock use; \
    Proc *head; \
    Proc *tail; \
    ulong wpc; \
    Proc *wproc; \
    int readers; \
    int writer
#define P9_EMBED_RWLOCK \
    union { RWlock _p9_rwlock; struct { P9_RWLOCK_FIELDS; }; }

/* Timer is defined in portdat.h before its first anonymous embedding. */
#define P9_TIMER_FIELDS \
    int tmode; \
    vlong tns; \
    void (*tf)(Ureg*, Timer*); \
    void *ta; \
    P9_EMBED_LOCK; \
    Timers *tt; \
    Tval tticks; \
    Tval twhen; \
    Timer *tnext
#define P9_EMBED_TIMER \
    union { Timer _p9_timer; struct { P9_TIMER_FIELDS; }; }

#define P9_ISACONF_FIELDS \
    char *type; \
    ulong port; \
    int irq; \
    ulong dma; \
    ulong mem; \
    ulong size; \
    ulong freq; \
    int nopt; \
    char *opt[NISAOPT]
#define P9_EMBED_ISACONF \
    union { ISAConf _p9_isaconf; struct { P9_ISACONF_FIELDS; }; }

#define P9_HCIIMPL_FIELDS \
    void *aux; \
    void (*init)(Hci*); \
    void (*dump)(Hci*); \
    void (*interrupt)(Ureg*, void*); \
    void (*epopen)(Ep*); \
    void (*epclose)(Ep*); \
    long (*epread)(Ep*, void*, long); \
    long (*epwrite)(Ep*, void*, long); \
    char *(*seprintep)(char*, char*, Ep*); \
    int (*portenable)(Hci*, int, int); \
    int (*portreset)(Hci*, int, int); \
    int (*portstatus)(Hci*, int); \
    void (*shutdown)(Hci*); \
    void (*debug)(Hci*, int)
#define P9_EMBED_HCIIMPL \
    union { Hciimpl _p9_hciimpl; struct { P9_HCIIMPL_FIELDS; }; }

#define P9_SDPERM_FIELDS \
    char *name; \
    char *user; \
    ulong perm
#define P9_EMBED_SDPERM \
    union { SDperm _p9_sdperm; struct { P9_SDPERM_FIELDS; }; }

#define P9_NOTSAVE_FIELDS \
    ulong svflags; \
    ulong svcs; \
    ulong svss
#define P9_EMBED_NOTSAVE \
    union { Notsave _p9_notsave; struct { P9_NOTSAVE_FIELDS; }; }

#define P9_PMMU_FIELDS \
    Page *mmupdb; \
    Page *mmufree; \
    Page *mmuused; \
    Page *kmaptable; \
    uint lastkmap; \
    int nkmap
#define P9_EMBED_PMMU \
    union { PMMU _p9_pmmu; struct { P9_PMMU_FIELDS; }; }

/* 64-bit FXSAVE layout: register save area is exactly 512 bytes. */
#define P9_FPSS_FIELDS \
    ushort fcw; \
    ushort fsw; \
    uchar ftw; \
    uchar reserved0; \
    ushort fop; \
    u64int fpuip; \
    u64int fpudp; \
    u32int mxcsr; \
    u32int mxcsr_mask; \
    uchar xregs[480]
#define P9_EMBED_FPSS \
    union { FPssestate _p9_fpssestate; struct { P9_FPSS_FIELDS; }; }

#define P9_FPSTATE_FIELDS \
    ushort control; \
    ushort r1; \
    ushort status; \
    ushort r2; \
    ushort tag; \
    ushort r3; \
    ulong pc; \
    ushort selector; \
    ushort r4; \
    ulong operand; \
    ushort oselector; \
    ushort r5; \
    uchar regs[80]
#define P9_EMBED_FPSTATE \
    union { FPstate _p9_fpstate; struct { P9_FPSTATE_FIELDS; }; }

#define P9_SFPSS_FIELDS \
    P9_EMBED_FPSS; \
    uchar alignpad[FPalign]; \
    ulong magic
#define P9_EMBED_SFPSS \
    union { SFPssestate _p9_sfpssestate; struct { P9_SFPSS_FIELDS; }; }

/* Explicit, type-safe member selection at the source callsite.
 * All fields are defined by C11 anonymous union overlays, and the
 * enclosing structure owns the storage. No Microsoft extension needed.
 */
#define P9_LOCKPTR(p)       (&(p)->_p9_lock)
#define P9_QLPTR(p)         (&(p)->_p9_qlock)
#define P9_REFPTR(p)        (&(p)->_p9_ref)
#define P9_RENDEZPTR(p)     (&(p)->_p9_rendez)
#define P9_TIMERPTR(p)      (&(p)->_p9_timer)

#endif /* P9_AMD64_EMBED_H */
