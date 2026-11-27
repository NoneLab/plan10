#ifndef P9_LIB9C_IPC_H
#define P9_LIB9C_IPC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Both descriptors are bidirectional; close them with lib10c close(). */
int p9Pipe(int fd[2]);

/* Exchange a machine-word value in the current rendezvous group.
 * (void *)-1 is reserved for interruption; do not send it as a payload.
 * Other values, including NULL and high-bit-set values, are valid. */
void *p9Rendezvous(void *tag, void *value);

/* Native semaphore words are signed machine longs (64 bits on AMD64). */
int p9Semacquire(long *addr, int block);
int p9Tsemacquire(long *addr, unsigned long milliseconds);
long p9Semrelease(long *addr, long delta);

#ifdef __cplusplus
}
#endif

#endif
