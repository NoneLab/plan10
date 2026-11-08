/* Original Plan 9 kernel's function-pointer hooks, for native AMD64. */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

extern void p9_amd64_rdtsc(uvlong*);

int (*cmpswap)(long*, long, long) = cmpswap486;
void (*coherence)(void) = mfence;
void (*cycles)(uvlong*) = p9_amd64_rdtsc;
