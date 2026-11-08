/*
 * Machine-dependent Plan 9 debugging and register interfaces for AMD64.
 * Existing /port rdb, page, sysproc and proc devices call these methods.
 */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "ureg.h"
#include "../port/error.h"

void
callwithureg(void (*fn)(Ureg *))
{
    Ureg ur;

    memset(&ur, 0, sizeof ur);
    ur.ip = getcallerpc(&fn);
    ur.sp = (ulong)(uintptr)&fn;
    ur.cs = KESEL;
    ur.ss = KDSEL;
    fn(&ur);
}

void
dumpstack(void)
{
    uintptr *frame, *next;
    int i;

    frame = __builtin_frame_address(0);
    iprint("pc64: kernel stack trace\n");
    for(i = 0; i < 32; i++){
        if(frame == nil || ((uintptr)frame & 7) != 0)
            break;
        iprint("%2d: pc %#p bp %#p\n", i, (void*)frame[1], frame);
        next = (uintptr*)frame[0];
        if(next <= frame || (uintptr)next - (uintptr)frame > KSTACK)
            break;
        frame = next;
    }
}

ulong
dbgpc(Proc *p)
{
    Ureg *ur;

    ur = p->dbgreg;
    return ur == nil ? 0 : ur->pc;
}

void
setkernur(Ureg *ur, Proc *p)
{
    memset(ur, 0, sizeof *ur);
    ur->pc = p->sched.pc;
    ur->sp = p->sched.sp;
    ur->cs = KESEL;
    ur->ss = KDSEL;
}

ulong
userpc(void)
{
    if(up == nil || up->dbgreg == nil)
        return 0;
    return ((Ureg*)up->dbgreg)->pc;
}

void
setregisters(Ureg *ur, char *ureg, char *data, int n)
{
    u64int cs, ss, flags;

    cs = ur->cs;
    ss = ur->ss;
    flags = ur->flags;
    memmove(ureg, data, n);

    /* Always return to user CPL3 and keep privileged flags unchanged. */
    ur->cs = cs;
    ur->ss = ss;
    ur->flags = (ur->flags & 0x0000000000000cd5ULL)
              | (flags & ~0x0000000000000cd5ULL);
}

void
validalign(uintptr addr, unsigned align)
{
    if(align != 0 && (align & (align - 1)) == 0 && (addr & (align - 1)) == 0)
        return;
    if(up != nil)
        postnote(up, 1, "sys: unaligned address", NDebug);
    error(Ebadarg);
}

void
countpagerefs(ulong *ref, int printing)
{
    ulong i;
    Page *q;
    Proc *p;

    for(i = 0; i < conf.nproc; i++){
        p = proctab(i);
        if(p->mmupdb != nil){
            ulong idx = pagenumber(p->mmupdb);
            if(printing){
                if(ref[idx] != 0)
                    iprint("pc64: proc %lud has PML4 page %#p\n",
                           p->pid, (void*)p->mmupdb->pa);
            }else
                ref[idx]++;
        }
        for(q = p->mmuused; q != nil; q = q->next){
            if(printing){
                if(ref[pagenumber(q)] != 0)
                    iprint("pc64: proc %lud has PT page %#p\n",
                           p->pid, (void*)q->pa);
            }else
                ref[pagenumber(q)]++;
        }
        for(q = p->mmufree; q != nil; q = q->next){
            if(printing){
                if(ref[pagenumber(q)] != 0)
                    iprint("pc64: proc %lud has recycled PT %#p\n",
                           p->pid, (void*)q->pa);
            }else
                ref[pagenumber(q)]++;
        }
    }
}
