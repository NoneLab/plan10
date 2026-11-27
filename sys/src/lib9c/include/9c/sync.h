#ifndef P9_LIB9C_SYNC_H
#define P9_LIB9C_SYNC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Zero initialize these objects before use. All participants must share
 * the lock storage and the same rendezvous group. Not POSIX pthread locks. */
typedef struct P9Lock { long key, sem; } P9Lock;
typedef struct P9QLp P9QLp;
typedef struct P9QLock {
    P9Lock lock;
    int locked;
    P9QLp *head, *tail;
} P9QLock;
typedef struct P9RWLock {
    P9Lock lock;
    int readers, writer;
    P9QLp *head, *tail;
} P9RWLock;
typedef struct P9Rendez {
    P9QLock *l;
    P9QLp *head, *tail;
} P9Rendez;

long p9Ainc(long *p);
long p9Adec(long *p);
int p9Cas32(unsigned int *p, unsigned int old, unsigned int value);
int p9Casl(unsigned long *p, unsigned long old, unsigned long value);
int p9Casp(void **p, void *old, void *value);
void p9Lock(P9Lock *l);
void p9Unlock(P9Lock *l);
int p9Canlock(P9Lock *l);
void p9Qlock(P9QLock *q);
void p9Qunlock(P9QLock *q);
int p9Canqlock(P9QLock *q);
void p9Rlock(P9RWLock *q);
void p9Runlock(P9RWLock *q);
int p9Canrlock(P9RWLock *q);
void p9Wlock(P9RWLock *q);
void p9Wunlock(P9RWLock *q);
int p9Canwlock(P9RWLock *q);
/* Caller holds r->l. Sleep releases it and returns holding it again. */
void p9Rsleep(P9Rendez *r);
int p9Rwakeup(P9Rendez *r);
int p9Rwakeupall(P9Rendez *r);

#ifdef __cplusplus
}
#endif
#endif
