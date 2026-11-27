#include <errno.h>
#include <limits.h>
#include <9c/process.h>
#include "../internal/native.h"

int
p9Rfork(int flags)
{
    return p9IntResult(P9C_CALL(RFORK, flags, 0, 0, 0, 0));
}

void
p9Exits(const char *status)
{
    (void)P9C_CALL(EXITS, status, 0, 0, 0, 0);
    __builtin_trap(); /* EXITS never returns on success. */
}

int
p9Fork(void)
{
    return p9Rfork(P9_RFPROC | P9_RFFDG | P9_RFREND);
}

int
p9Exec(const char *path, char *const argv[])
{
    return p9IntResult(P9C_CALL(EXEC, path, argv, 0, 0, 0));
}

int
p9Await(char *buf, int len)
{
    if (len <= 0) { errno = EINVAL; return -1; }
    return p9IntResult(P9C_CALL(AWAIT, buf, len, 0, 0, 0));
}

int
p9Sleep(long milliseconds)
{
    if (milliseconds < 0 || milliseconds > INT_MAX) {
        errno = EINVAL;
        return -1;
    }
    return p9IntResult(P9C_CALL(SLEEP, milliseconds, 0, 0, 0, 0));
}

long
p9Alarm(unsigned long milliseconds)
{
    return __lib10c_sysresult(P9C_CALL(ALARM, milliseconds, 0, 0, 0, 0));
}

long long
p9Nsec(void)
{
    long long result;

    if (p9IntResult(P9C_CALL(NSEC, &result, 0, 0, 0, 0)) < 0)
        return -1;
    return result;
}

int
p9Notify(void (*handler)(void *, char *))
{
    return p9IntResult(P9C_CALL(NOTIFY, handler, 0, 0, 0, 0));
}

int
p9Noted(int action)
{
    return p9IntResult(P9C_CALL(NOTED, action, 0, 0, 0, 0));
}
