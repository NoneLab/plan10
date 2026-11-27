#include <errno.h>
#include <9c/ipc.h>
#include "../internal/native.h"

int
p9Pipe(int fd[2])
{
    return p9IntResult(P9C_CALL(PIPE, fd, 0, 0, 0, 0));
}

void *
p9Rendezvous(void *tag, void *value)
{
    long result = P9C_CALL(RENDEZVOUS, tag, value, 0, 0, 0);

    /* RENDEZVOUS returns a machine word, not a signed status code.
     * Its all-ones interruption sentinel is not a normal kernel error
     * string.  Do not pass arbitrary payloads to __lib10c_sysresult. */
    if (result == -1)
        errno = EINTR;
    return (void *)(__UINTPTR_TYPE__)result;
}

int
p9Semacquire(long *addr, int block)
{
    return p9IntResult(P9C_CALL(SEMACQUIRE, addr, block, 0, 0, 0));
}

int
p9Tsemacquire(long *addr, unsigned long milliseconds)
{
    return p9IntResult(P9C_CALL(TSEMACQUIRE, addr, milliseconds, 0, 0, 0));
}

long
p9Semrelease(long *addr, long delta)
{
    return __lib10c_sysresult(P9C_CALL(SEMRELEASE, addr, delta, 0, 0, 0));
}
