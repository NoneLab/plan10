/*
 * Native AMD64 Plan 9 note delivery and restoration.
 *
 * Equivalent user-facing semantics to ../pc/trap.c notify()/noted(),
 * with a SysV AMD64 call frame instead of 386 stack-passed arguments.
 *
 * User stack, descending addresses:
 *   interrupted RSP - 256  (preserved fault/debugging context)
 *   saved Ureg              (up->ureg)
 *   previous up->ureg       (one 64-bit word below saved Ureg)
 *   ERRMAX-byte note string
 *   SysV call frame        (RSP % 16 == 8; dummy return address)
 *
 * The user handler receives RDI=saved Ureg*, RSI=note string.
 * A handler must finish with noted() or _exits(), not return.
 */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "ureg.h"

_Static_assert(sizeof(Ureg) == 184, "AMD64 note Ureg must match trapvec.S");

/* SysV AMD64 stack alignment: RSP is 8 modulo 16 at function entry. */
static ulong
notestack(ulong msg)
{
    return ((msg - 2*BY2WD) & ~(ulong)15) + BY2WD;
}

static int
noteregvalid(Ureg *ur)
{
    /* In long mode iretq takes CS, SS, RIP, RFLAGS and RSP. */
    if((ur->cs & 0xffff) != UESEL || (ur->ss & 0xffff) != UDSEL)
        return 0;
    if(ur->ip < UTZERO || ur->ip >= USTKTOP ||
       ur->sp < UTZERO || ur->sp >= USTKTOP)
        return 0;
    if(!okaddr(ur->ip, 1, 0) || !okaddr(ur->sp, BY2WD, 0))
        return 0;
    return 1;
}

/*
 * Send one pending note to the process, or honor a pending procctl.
 * Returning 1 means the saved trap frame now points at the user handler.
 */
int
notify(Ureg *ur)
{
    Note *n;
    ulong reg, oldslot, msg, sp, originalsp, handler;
    int s, l;

    if(up->procctl){
        s = splhi();
        procctl(up);
        splx(s);
    }
    if(up->nnote == 0)
        return 0;

    if(up->fpstate == FPactive){
        /* Keep the interrupted FP state out of the integer-only handler. */
        __asm__ volatile("fxsave64 %0" : "=m"(up->fpsave) :: "memory");
        up->fpstate = FPinactive;
    }
    up->fpstate |= FPillegal;

    s = spllo();
    qlock(&up->debug);
    if(up->nnote == 0){
        qunlock(&up->debug);
        splx(s);
        return 0;
    }
    up->notepending = 0;
    n = &up->note[0];
    if(strncmp(n->msg, "sys:", 4) == 0){
        l = strlen(n->msg);
        if(l > ERRMAX-24)
            l = ERRMAX-24;
        seprint(n->msg+l, n->msg+sizeof(n->msg), " pc=%#llux", (uvlong)ur->ip);
    }

    if(n->flag != NUser && (up->notified || up->notify == nil)){
        if(n->flag == NDebug)
            pprint("suicide: %s\n", n->msg);
        qunlock(&up->debug);
        pexit(n->msg, n->flag != NDebug);
    }
    if(up->notified){
        qunlock(&up->debug);
        splx(s);
        return 0;
    }
    if(up->notify == nil){
        qunlock(&up->debug);
        pexit(n->msg, n->flag != NDebug);
    }

    handler = (ulong)(uintptr)up->notify;
    originalsp = ur->sp;
    /* Avoid underflow before calculating the user-owned note frame. */
    if(originalsp < UTZERO + 256 + sizeof(Ureg) + ERRMAX + 8*BY2WD ||
       originalsp >= USTKTOP){
        qunlock(&up->debug);
        pprint("suicide: invalid user stack for note\n");
        pexit("Suicide", 0);
    }

    reg = (originalsp - 256 - sizeof(Ureg)) & ~(ulong)(BY2WD-1);
    oldslot = reg - BY2WD;
    msg = oldslot - ERRMAX;
    sp = notestack(msg);
    if(sp < UTZERO || !okaddr(handler, 1, 0) ||
       !okaddr(sp, (reg + sizeof(Ureg)) - sp, 1)){
        qunlock(&up->debug);
        pprint("suicide: bad address in notify\n");
        pexit("Suicide", 0);
    }

    memmove((void*)(uintptr)reg, ur, sizeof(Ureg));
    *(ulong*)(uintptr)oldslot = (ulong)(uintptr)up->ureg;
    memmove((void*)(uintptr)msg, n->msg, ERRMAX);
    *(ulong*)(uintptr)sp = 0; /* returning from a note handler is invalid */

    up->ureg = (void*)(uintptr)reg;
    ur->di = reg;
    ur->si = msg;
    ur->ip = handler;
    ur->sp = sp;
    ur->flags &= ~(ulong)0x400; /* SysV ABI: clear direction flag */
    up->notified = 1;
    up->nnote--;
    memmove(&up->lastnote, &up->note[0], sizeof(Note));
    if(up->nnote != 0)
        memmove(&up->note[0], &up->note[1], up->nnote*sizeof(Note));
#ifdef P9_NOTE_TRACE
    print("pc64: note pid=%lud text=%s msg=%s handler=%#lux\n",
        up->pid, up->text, up->lastnote.msg, handler);
#endif
    qunlock(&up->debug);
    splx(s);
    return 1;
}

/*
 * Restore the interrupted user state (NCONT/NRSTR), or transfer to an
 * alternate APE note handler without releasing the saved note (NSAVE).
 */
void
noted(Ureg *ur, ulong action)
{
    Ureg saved;
    ulong reg, prev, msg, sp;

    qlock(&up->debug);
    if(action != NRSTR && !up->notified){
        qunlock(&up->debug);
        pprint("call to noted() when not notified\n");
        pexit("Suicide", 0);
    }
    up->notified = 0;
    up->fpstate &= ~FPillegal;
    reg = (ulong)(uintptr)up->ureg;
    if(reg < UTZERO + BY2WD || reg >= USTKTOP ||
       !okaddr(reg - BY2WD, BY2WD + sizeof(Ureg), 0)){
        qunlock(&up->debug);
        pprint("bad ureg in noted\n");
        pexit("Suicide", 0);
    }
    memmove(&saved, (void*)(uintptr)reg, sizeof saved);
    prev = *(ulong*)(uintptr)(reg - BY2WD);

    if(action == NDFLT){
        if(up->lastnote.flag == NDebug)
            pprint("suicide: %s\n", up->lastnote.msg);
        qunlock(&up->debug);
        pexit(up->lastnote.msg, up->lastnote.flag != NDebug);
    }
    if(action != NCONT && action != NRSTR && action != NSAVE){
        up->lastnote.flag = NDebug;
        qunlock(&up->debug);
        pprint("unknown noted arg %#lux\n", action);
        pexit("Suicide", 0);
    }

    if(!noteregvalid(&saved)){
        qunlock(&up->debug);
        pprint("suicide: invalid registers in noted\n");
        pexit("Suicide", 0);
    }
    /* User Ureg cannot elevate CS/SS, change privileged RFLAGS, or
     * arbitrarily modify segment selectors not restored by trapvec.S. */
    saved.flags = (ur->flags & ~(ulong)0xcd5) | (saved.flags & (ulong)0xcd5);
    saved.ds = ur->ds;
    saved.es = ur->es;
    saved.fs = ur->fs;
    saved.gs = ur->gs;
    saved.type = ur->type;
    saved.error = ur->error;
    memmove(ur, &saved, sizeof saved);

    switch(action){
    case NCONT:
    case NRSTR:
        if(prev != 0 && (prev < UTZERO || prev >= USTKTOP ||
           !okaddr(prev, sizeof(Ureg), 0))){
            qunlock(&up->debug);
            pprint("suicide: corrupted note chain\n");
            pexit("Suicide", 0);
        }
        up->ureg = (void*)(uintptr)prev;
        qunlock(&up->debug);
        return;
    case NSAVE:
        /* Preserve the Ureg chain, clear the current notification, and
         * enter saved.ip with the original note arguments (APE). */
        msg = reg - BY2WD - ERRMAX;
        sp = notestack(msg);
        if(sp < UTZERO || !okaddr(sp, reg - sp, 1)){
            qunlock(&up->debug);
            pprint("suicide: corrupted saved note frame\n");
            pexit("Suicide", 0);
        }
        *(ulong*)(uintptr)sp = 0;
        ur->sp = sp;
        ur->di = reg;
        ur->si = msg;
        ur->flags &= ~(ulong)0x400;
        qunlock(&up->debug);
        return;
    }
}
