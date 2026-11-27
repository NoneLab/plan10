/* Queueing and rendezvous rules adapted from libc/9sys/qlock.c. */
#include <stddef.h>
#include <9c/sync.h>
#include <9c/ipc.h>
#include <9c/process.h>

enum { Reader, Writer, Queued, Sleeping };
struct P9QLp {
    unsigned inuse;
    P9QLp *next;
    int state;
};

/* RFMEM processes have separate stacks. Wait nodes must be in shared BSS. */
static P9QLp nodes[1024];
static unsigned cursor;

static P9QLp *
newnode(int state)
{
    unsigned start = __atomic_fetch_add(&cursor, 1, __ATOMIC_RELAXED);
    for (unsigned i = 0; i < 1024; ++i) {
        P9QLp *n = &nodes[(start + i) % 1024];
        unsigned expected = 0;
        if (__atomic_compare_exchange_n(&n->inuse, &expected, 1, 0,
                __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)) {
            n->next = NULL;
            n->state = state;
            return n;
        }
    }
    p9Exits("lib9c: queue node exhaustion");
}

static void
append(P9QLp **head, P9QLp **tail, P9QLp *n)
{
    if (*tail) (*tail)->next = n;
    else *head = n;
    *tail = n;
}

static P9QLp *
pop(P9QLp **head, P9QLp **tail)
{
    P9QLp *n = *head;
    if (n) {
        *head = n->next;
        if (!*head) *tail = NULL;
        n->next = NULL;
    }
    return n;
}

static void
waitnode(P9QLp *n)
{
    while (p9Rendezvous(n, (void *)1) == (void *)-1) ;
    __atomic_thread_fence(__ATOMIC_ACQUIRE);
    __atomic_store_n(&n->inuse, 0, __ATOMIC_RELEASE);
}

static void
wake(P9QLp *n)
{
    __atomic_thread_fence(__ATOMIC_RELEASE);
    while (p9Rendezvous(n, NULL) == (void *)-1) ;
}

void
p9Qlock(P9QLock *q)
{
    P9QLp *n;
    p9Lock(&q->lock);
    if (!q->locked) { q->locked = 1; p9Unlock(&q->lock); return; }
    n = newnode(Queued);
    append(&q->head, &q->tail, n);
    p9Unlock(&q->lock);
    waitnode(n);
}

static P9QLp *
passq(P9QLock *q)
{
    P9QLp *n;
    if (!q->locked) p9Exits("lib9c: unlocked qlock");
    n = pop(&q->head, &q->tail);
    if (!n) q->locked = 0;
    return n;
}

void
p9Qunlock(P9QLock *q)
{
    P9QLp *n;
    p9Lock(&q->lock);
    n = passq(q);
    p9Unlock(&q->lock);
    if (n) wake(n);
}

int
p9Canqlock(P9QLock *q)
{
    int ok;
    if (!p9Canlock(&q->lock)) return 0;
    ok = !q->locked;
    if (ok) q->locked = 1;
    p9Unlock(&q->lock);
    return ok;
}

void
p9Rlock(P9RWLock *q)
{
    P9QLp *n;
    p9Lock(&q->lock);
    if (!q->writer && !q->head) {
        ++q->readers; p9Unlock(&q->lock); return;
    }
    n = newnode(Reader);
    append(&q->head, &q->tail, n);
    p9Unlock(&q->lock);
    waitnode(n);
}

int
p9Canrlock(P9RWLock *q)
{
    int ok;
    p9Lock(&q->lock);
    ok = !q->writer && !q->head;
    if (ok) ++q->readers;
    p9Unlock(&q->lock);
    return ok;
}

void
p9Runlock(P9RWLock *q)
{
    P9QLp *n = NULL;
    p9Lock(&q->lock);
    if (q->readers <= 0) p9Exits("lib9c: unlocked rwlock reader");
    if (--q->readers == 0 && q->head) {
        n = pop(&q->head, &q->tail);
        if (n->state != Writer) p9Exits("lib9c: invalid rwlock queue");
        q->writer = 1;
    }
    p9Unlock(&q->lock);
    if (n) wake(n);
}

void
p9Wlock(P9RWLock *q)
{
    P9QLp *n;
    p9Lock(&q->lock);
    if (!q->writer && !q->readers && !q->head) {
        q->writer = 1; p9Unlock(&q->lock); return;
    }
    n = newnode(Writer);
    append(&q->head, &q->tail, n);
    p9Unlock(&q->lock);
    waitnode(n);
}

int
p9Canwlock(P9RWLock *q)
{
    int ok;
    p9Lock(&q->lock);
    ok = !q->writer && !q->readers && !q->head;
    if (ok) q->writer = 1;
    p9Unlock(&q->lock);
    return ok;
}

void
p9Wunlock(P9RWLock *q)
{
    P9QLp *n;
    p9Lock(&q->lock);
    if (!q->writer) p9Exits("lib9c: unlocked rwlock writer");
    if (q->head && q->head->state == Writer) {
        n = pop(&q->head, &q->tail);
        p9Unlock(&q->lock);
        wake(n);
        return;
    }
    q->writer = 0;
    while (q->head && q->head->state == Reader) {
        n = pop(&q->head, &q->tail);
        ++q->readers;
        wake(n);
    }
    p9Unlock(&q->lock);
}

void
p9Rsleep(P9Rendez *r)
{
    P9QLp *me, *next;
    if (!r->l) p9Exits("lib9c: missing rendezvous lock");
    p9Lock(&r->l->lock);
    if (!r->l->locked) p9Exits("lib9c: rsleep without lock");
    me = newnode(Sleeping);
    append(&r->head, &r->tail, me);
    next = passq(r->l);
    p9Unlock(&r->l->lock);
    if (next) wake(next);
    waitnode(me);
}

int
p9Rwakeup(P9Rendez *r)
{
    P9QLp *n;
    if (!r->l) p9Exits("lib9c: missing rendezvous lock");
    p9Lock(&r->l->lock);
    if (!r->l->locked) p9Exits("lib9c: rwakeup without lock");
    n = pop(&r->head, &r->tail);
    if (n) {
        n->state = Queued;
        n->next = r->l->head;
        r->l->head = n;
        if (!r->l->tail) r->l->tail = n;
    }
    p9Unlock(&r->l->lock);
    return n != NULL;
}

int
p9Rwakeupall(P9Rendez *r)
{
    int n = 0;
    while (p9Rwakeup(r)) ++n;
    return n;
}
