/*
 * Machine-independent globals whose original Plan 9 C header exported
 * common/tentative definitions.  This port gives each one a single owner.
 * Functions owned by devcons.c, devwd.c or the future pc64/devarch.c
 * are deliberately not duplicated here.
 */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

struct Swapalloc swapalloc;
int (*parseboothdr)(Chan*, ulong, Execvals*);
void (*lprint)(char*, int);
void (*kproftimer)(ulong);
