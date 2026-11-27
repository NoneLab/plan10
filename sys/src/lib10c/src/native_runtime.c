#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include "native_syscall.h"

/* Bootstrap globals only: Phase 2 must initialize them from argv and /env. */
static char *empty_environment[] = { 0 };
static char empty_progname[] = "";
char **environ = empty_environment;
char *__progname = empty_progname;

int
getpagesize(void)
{
    /* Matches BY2PG in sys/src/9/pc64/mem.h. */
    return 4096;
}
DEF_WEAK(getpagesize);

pid_t
getpid(void)
{
    char buf[32];
    long fd, n;
    unsigned long pid = 0;
    int saved_errno, i = 0, start;

    /* Same native device as libc/9sys/getpid.c; no PID caching across rfork. */
    fd = __lib10c_sysresult(P9_CALL(OPEN, "#c/pid", 0, 0, 0, 0));
    if (fd < 0)
        return (pid_t)-1;
    n = HIDDEN(read)((int)fd, buf, sizeof(buf) - 1);
    saved_errno = errno;
    if (HIDDEN(close)((int)fd) < 0 && n >= 0)
        return (pid_t)-1;
    if (n < 0) {
        errno = saved_errno;
        return (pid_t)-1;
    }
    buf[n] = '\0';
    while (buf[i] == ' ' || buf[i] == '\t')
        ++i;
    start = i;
    while (buf[i] >= '0' && buf[i] <= '9') {
        unsigned int digit = (unsigned int)(buf[i++] - '0');
        if (pid > ((unsigned long)INT_MAX - digit) / 10) {
            errno = EOVERFLOW;
            return (pid_t)-1;
        }
        pid = pid * 10 + digit;
    }
    if (i == start || pid == 0) {
        errno = EIO;
        return (pid_t)-1;
    }
    while (buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\n')
        ++i;
    if (buf[i] != '\0') {
        errno = EIO;
        return (pid_t)-1;
    }
    return (pid_t)pid;
}
DEF_STRONG(getpid);

void
_exit(int status)
{
    char msg[16];
    char *p = msg + sizeof(msg);
    unsigned int code = (unsigned int)status & 255U;

    /* Plan 9 wait messages are strings, not POSIX integer wait statuses.
     * Encode a nonzero low byte as decimal without allocating or using stdio.
     */
    if (code == 0)
        (void)P9_CALL(EXITS, 0, 0, 0, 0, 0);
    else {
        *--p = '\0';
        do {
            *--p = (char)('0' + code % 10);
            code /= 10;
        } while (code != 0);
        (void)P9_CALL(EXITS, p, 0, 0, 0, 0);
    }
    __builtin_trap(); /* EXITS cannot return on success. */
}
DEF_STRONG(_exit);

/* Temporary weak bridge for the existing crt0-amd64's exits(nil).
 * It is deliberately a raw Plan 9 exit: no atexit/stdio/TIB processing.
 * A dedicated lib10c CRT must replace this path in Phase 2.
 */
__attribute__((weak, noreturn)) void
exits(const char *status)
{
    (void)P9_CALL(EXITS, status, 0, 0, 0, 0);
    __builtin_trap();
}
