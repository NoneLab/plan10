#include <errno.h>
#include "native_syscall.h"

/* Plan 9's ERRSTR exchanges the user's string with the kernel error.
 * Do not use stdio, malloc, or another error-translating wrapper here.
 * Unknown device/server-specific errors are deliberately mapped to EIO.
 */
static int
same(const char *a, const char *b)
{
    while (*a && *a == *b) {
        ++a;
        ++b;
    }
    return *a == *b;
}

long
__lib10c_sysresult(long result)
{
    static const struct {
        const char *message;
        int code;
    } errors[] = {
        { "fd out of range or not open", EBADF },
        { "inappropriate use of fd", EBADF },
        { "bad arg in system call", EINVAL },
        { "negative i/o offset", EINVAL },
        { "seek on a stream", ESPIPE },
        { "seek in directory", ESPIPE },
        { "permission denied", EACCES },
        { "file does not exist", ENOENT },
        { "file already exists", EEXIST },
        { "not a directory", ENOTDIR },
        { "file is a directory", EISDIR },
        { "interrupted", EINTR },
        { "no free file descriptors", EMFILE },
        { "virtual memory allocation failed", ENOMEM },
        { "kernel allocate failed", ENOMEM },
        { "i/o on hungup channel", EPIPE },
        { "connection timed out", ETIMEDOUT },
        { "connection refused", ECONNREFUSED },
    };
    char msg[256] = { 0 };
    unsigned int i;

    if (result >= 0)
        return result;
    errno = EIO;
    if (P9_CALL(ERRSTR, msg, sizeof(msg), 0, 0, 0) < 0)
        return -1;
    msg[sizeof(msg) - 1] = '\0';
    for (i = 0; i < sizeof(errors) / sizeof(errors[0]); ++i) {
        if (same(msg, errors[i].message)) {
            errno = errors[i].code;
            break;
        }
    }
    /* Restore the native error string after mapping it. ERRSTR swaps. */
    (void)P9_CALL(ERRSTR, msg, sizeof(msg), 0, 0, 0);
    return -1;
}
