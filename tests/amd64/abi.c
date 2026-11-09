/* ABI checks against the actual machine and port structure declarations. */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "ureg.h"

#define CHECK(expr) _Static_assert((expr), #expr)

CHECK(sizeof(void*) == 8);
CHECK(sizeof(ulong) == 8);
CHECK(sizeof(uintptr) == 8);
CHECK(sizeof(uvlong) == 8);
CHECK(sizeof(Segdesc) == 8);
CHECK(sizeof(Tss) == 104);
CHECK(sizeof(FPssestate) == 512);
CHECK(__alignof__(FPssestate) >= 16);
CHECK(sizeof(Sargs) == 5 * 8);
CHECK(sizeof(Lock) == 56);
CHECK(sizeof(Ref) == 64);
CHECK(sizeof(Rendez) == 64);
CHECK(__builtin_offsetof(FPssestate, xregs) == 32);
CHECK(__builtin_offsetof(Ureg, ip) == __builtin_offsetof(Ureg, pc));
CHECK(__builtin_offsetof(QLock, use) == 0);
CHECK(__builtin_offsetof(Chan, ref) == sizeof(Lock));
CHECK(__builtin_offsetof(Ref, _p9_lock) == 0);
CHECK(__builtin_offsetof(Ref, key) == 0);
CHECK(__builtin_offsetof(Chan, _p9_ref) == 0);
CHECK(__builtin_offsetof(Chan, _p9_lock) == 0);
CHECK(__builtin_offsetof(Chan, key) == 0);
CHECK(__builtin_offsetof(Proc, _p9_pmmu) == __builtin_offsetof(Proc, mmupdb));
CHECK(__builtin_offsetof(Proc, _p9_notsave) == __builtin_offsetof(Proc, svflags));
CHECK(__builtin_offsetof(Proc, _p9_timer) == __builtin_offsetof(Proc, tmode));
CHECK(PTEPERTAB == 512);
CHECK(sizeof(ulong) == sizeof(void*));
CHECK(sizeof(Active) >= sizeof(Lock) + sizeof(int) * 5);
CHECK(__builtin_offsetof(Active, _p9_lock) == 0);
CHECK(__builtin_offsetof(Active, machs) == sizeof(Lock));
CHECK(__builtin_offsetof(Tss, rsp0) == 4);
CHECK(__builtin_offsetof(Tss, ist) == 36);
CHECK(__builtin_offsetof(Tss, iomap) == 102);
CHECK(__builtin_offsetof(Ureg, ax) == 0);
CHECK(__builtin_offsetof(Ureg, r15) == 112);
CHECK(__builtin_offsetof(Ureg, type) == 128);
CHECK(__builtin_offsetof(Ureg, error) == 136);
CHECK(__builtin_offsetof(Ureg, ip) == 144);
CHECK(__builtin_offsetof(Ureg, sp) == 168);
CHECK(__builtin_offsetof(Ureg, ss) == 176);
CHECK(sizeof(Ureg) == 184);
CHECK(P9_VECTOR_SYSCALL == 0x40);
CHECK(UESEL == 0x23);
CHECK(UDSEL == 0x1b);
CHECK(sizeof(Sargs) == MAXSYSARG * BY2WD);

CHECK(sizeof(Label) == 64);
CHECK(__builtin_offsetof(Label, sp) == 0);
CHECK(__builtin_offsetof(Label, pc) == 8);
CHECK(__builtin_offsetof(Label, bx) == 16);
CHECK(__builtin_offsetof(Label, bp) == 24);
CHECK(__builtin_offsetof(Label, r12) == 32);
CHECK(__builtin_offsetof(Label, r13) == 40);
CHECK(__builtin_offsetof(Label, r14) == 48);
CHECK(__builtin_offsetof(Label, r15) == 56);
CHECK((KZERO & ((1ULL << 21)-1)) == 0);
CHECK(KTZERO == KZERO + 0x00200000ULL);

/* Standalone translation unit: this checks type/layout only, not kernel boot. */
int p9_amd64_abi_checked;
