#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <syslog.h>
#include "native_syscall.h"

/* In Phase 1 the cancellation entry points forward to the same native
 * operations. No pthread cancellation is provided until TLS/thread bring-up.
 * hidden/unistd.h maps read/write/close to their _cancel entry points.
 */
ssize_t
HIDDEN(read)(int fd, void *buf, size_t count)
{
    if (count > (size_t)LONG_MAX) {
        errno = EINVAL;
        return -1;
    }
    return __lib10c_sysresult(P9_CALL(PREAD, fd, buf, count, -1L, 0));
}

ssize_t
read(int fd, void *buf, size_t count)
{
    return HIDDEN(read)(fd, buf, count);
}
DEF_CANCEL(read);

ssize_t
HIDDEN(write)(int fd, const void *buf, size_t count)
{
    if (count > (size_t)LONG_MAX) {
        errno = EINVAL;
        return -1;
    }
    return __lib10c_sysresult(P9_CALL(PWRITE, fd, buf, count, -1L, 0));
}

ssize_t
write(int fd, const void *buf, size_t count)
{
    return HIDDEN(write)(fd, buf, count);
}
DEF_CANCEL(write);

int
HIDDEN(close)(int fd)
{
    return (int)__lib10c_sysresult(P9_CALL(CLOSE, fd, 0, 0, 0, 0));
}

int
close(int fd)
{
    return HIDDEN(close)(fd);
}
DEF_CANCEL(close);

off_t
lseek(int fd, off_t offset, int whence)
{
    long long result;

    if (whence != SEEK_SET && whence != SEEK_CUR && whence != SEEK_END) {
        errno = EINVAL;
        return (off_t)-1;
    }
    /* Plan 9 AMD64: result pointer, fd, full 64-bit offset, reserved, whence. */
    if (__lib10c_sysresult(P9_CALL(SEEK, &result, fd, offset, 0, whence)) < 0)
        return (off_t)-1;
    return (off_t)result;
}
DEF_STRONG(lseek);

int
sendsyslog(const char *buf, size_t len, int flags)
{
    /* There is no syslog daemon transport yet. LOG_CONS enables a best-effort
     * diagnostic fallback to stderr, not a full OpenBSD logging service.
     */
    if (!(flags & LOG_CONS)) {
        errno = ENOSYS;
        return -1;
    }
    while (len != 0) {
        size_t chunk = len > (size_t)LONG_MAX ? (size_t)LONG_MAX : len;
        ssize_t n = HIDDEN(write)(STDERR_FILENO, buf, chunk);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        if (n == 0) {
            errno = EIO;
            return -1;
        }
        buf += n;
        len -= (size_t)n;
    }
    return 0;
}
DEF_STRONG(sendsyslog);
