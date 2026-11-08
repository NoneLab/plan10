/*
 * Single definitions for AMD64 machine-wide objects.
 * The 386 compiler tolerated tentative definitions in headers;
 * modern Clang uses -fno-common and must see a single owner.
 */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"

/* The boot CPU starts on a real kernel-resident Mach, not an unmapped
 * fixed-address 386 CPU0MACH page.  SMP per-CPU placement is still pending.
 */
static Mach bootmach;
Mach *m = &bootmach;
Mach *p9_amd64_current_mach = &bootmach;
Mach *machp[MAXMACH];
Active active;
