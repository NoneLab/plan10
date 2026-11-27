/* Native semaphore-backed lock, following libc/port/lock.c. */
#include <errno.h>
#include <9c/sync.h>
#include <9c/ipc.h>
#include <9c/process.h>

long p9Ainc(long *p) { return __atomic_add_fetch(p, 1, __ATOMIC_SEQ_CST); }
long p9Adec(long *p) { return __atomic_sub_fetch(p, 1, __ATOMIC_SEQ_CST); }
int p9Cas32(unsigned int *p, unsigned int old, unsigned int value)
{
    return __atomic_compare_exchange_n(p, &old, value, 0,
        __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}
int p9Casl(unsigned long *p, unsigned long old, unsigned long value)
{
    return __atomic_compare_exchange_n(p, &old, value, 0,
        __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}
int p9Casp(void **p, void *old, void *value)
{
    return __atomic_compare_exchange_n(p, &old, value, 0,
        __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

static void
release(P9Lock *l)
{
    long n = p9Adec(&l->key);
    if (n < 0) p9Exits("lib9c: unlocked lock");
    if (n && p9Semrelease(&l->sem, 1) < 0)
        p9Exits("lib9c: semrelease failed");
}

void
p9Lock(P9Lock *l)
{
    if (p9Ainc(&l->key) == 1) return;
    for (;;) {
        int r = p9Semacquire(&l->sem, 1);
        if (r > 0) break;
        if (r < 0 && errno != EINTR) p9Exits("lib9c: semacquire failed");
    }
    __atomic_thread_fence(__ATOMIC_ACQUIRE);
}

void p9Unlock(P9Lock *l) { release(l); }
int p9Canlock(P9Lock *l)
{
    long expected = 0;
    /* A failed try-lock must not post a semaphore token.  The old
     * increment/undo path can leave a token while the owner is alive. */
    return __atomic_compare_exchange_n(&l->key, &expected, 1, 0,
        __ATOMIC_ACQUIRE, __ATOMIC_RELAXED);
}
