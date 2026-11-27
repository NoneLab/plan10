#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "native_syscall.h"

/* Plan 9 stat wire format (little endian): size[2], type[2], dev[4],
 * qid[13], mode[4], atime[4], mtime[4], length[8], four counted strings.
 * Do not import native libc's Dir or its incompatible fstat prototype. */
static uint16_t
le16(const unsigned char *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t
le32(const unsigned char *p)
{
    return (uint32_t)le16(p) | ((uint32_t)le16(p + 2) << 16);
}

static uint64_t
le64(const unsigned char *p)
{
    return (uint64_t)le32(p) | ((uint64_t)le32(p + 4) << 32);
}

static int
native_stat(int fd, unsigned char *buf, size_t cap)
{
    size_t pos;
    unsigned int i;
    long n = __lib10c_sysresult(P9_CALL(FSTAT, fd, buf, cap, 0, 0));
    if (n < 0)
        return -1;
    if (n == 2 || n > (long)cap) {
        errno = EOVERFLOW;
        return -1;
    }
    if (n < 49 || (size_t)le16(buf) + 2 != (size_t)n) {
        errno = EIO;
        return -1;
    }
    pos = 41;
    for (i = 0; i < 4; ++i) {
        size_t len;
        if ((size_t)n - pos < 2) {
            errno = EIO;
            return -1;
        }
        len = le16(buf + pos);
        pos += 2;
        if (len > (size_t)n - pos) {
            errno = EIO;
            return -1;
        }
        pos += len;
    }
    if (pos != (size_t)n) {
        errno = EIO;
        return -1;
    }
    return 0;
}

/* devcons.c: device rune 'c', Qcons = 2. Unlike a path-name test this
 * identifies the native console even when it was bound into /dev. */
static int
native_console(const unsigned char *buf)
{
    return le16(buf + 2) == 'c' && le64(buf + 13) == 2;
}

__dso_hidden int
HIDDEN(fstat)(int fd, struct stat *st)
{
    unsigned char buf[1024];
    uint32_t mode;
    uint64_t length;

    if (st == NULL) {
        errno = EFAULT;
        return -1;
    }
    if (native_stat(fd, buf, sizeof(buf)) < 0)
        return -1;
    length = le64(buf + 33);
    if (length > INT64_MAX) {
        errno = EOVERFLOW;
        return -1;
    }
    mode = le32(buf + 21);
    memset(st, 0, sizeof(*st));
    st->st_mode = (mode & 0777U) |
        ((mode & 0x80000000U) ? S_IFDIR :
            (native_console(buf) ? S_IFCHR : S_IFREG));
    st->st_ino = (ino_t)le64(buf + 13);
    st->st_dev = (dev_t)le32(buf + 4);
    st->st_nlink = 1;
    st->st_uid = (uid_t)-1; /* No native string-user to POSIX uid mapping. */
    st->st_gid = (gid_t)-1;
    st->st_size = (off_t)length;
    st->st_atime = (time_t)le32(buf + 25);
    st->st_mtime = (time_t)le32(buf + 29);
    /* No reliable ctime, block allocation, or optimal I/O size in 9P stat.
     * Zero blksize intentionally disables stdio's regular-file seek shortcut. */
    return 0;
}
DEF_STRONG(fstat);

/* Bootstrap implementation: sequential writes, not an atomic writev syscall.
 * No allocation or TLS cancellation; partial writes preserve byte count. */
__dso_hidden ssize_t
HIDDEN(writev)(int fd, const struct iovec *iov, int count)
{
    size_t total = 0;
    ssize_t done = 0;
    int i;

    if (count <= 0 || count > 1024) {
        errno = EINVAL;
        return -1;
    }
    if (iov == NULL) {
        errno = EFAULT;
        return -1;
    }
    for (i = 0; i < count; ++i) {
        if (iov[i].iov_len > (size_t)SSIZE_MAX - total) {
            errno = EINVAL;
            return -1;
        }
        total += iov[i].iov_len;
    }
    if (total == 0)
        return HIDDEN(write)(fd, iov[0].iov_base, 0);
    for (i = 0; i < count; ++i) {
        ssize_t n;
        if (iov[i].iov_len == 0)
            continue;
        n = HIDDEN(write)(fd, iov[i].iov_base, iov[i].iov_len);
        if (n < 0)
            return done != 0 ? done : -1;
        done += n;
        if ((size_t)n < iov[i].iov_len)
            break;
    }
    return done;
}

__dso_hidden ssize_t
CANCEL(writev)(int fd, const struct iovec *iov, int count)
{
    return HIDDEN(writev)(fd, iov, count);
}
DEF_CANCEL(writev);

static int
native_fcntl(int fd, int cmd)
{
    unsigned char buf[1024];
    if (cmd != F_ISATTY) {
        errno = ENOTSUP;
        return -1;
    }
    if (native_stat(fd, buf, sizeof(buf)) < 0)
        return -1;
    if (!native_console(buf)) {
        errno = ENOTTY;
        return -1;
    }
    return 0;
}

__dso_hidden int
HIDDEN(fcntl)(int fd, int cmd, ...)
{
    return native_fcntl(fd, cmd);
}

__dso_hidden int
CANCEL(fcntl)(int fd, int cmd, ...)
{
    return native_fcntl(fd, cmd);
}
DEF_CANCEL(fcntl);
