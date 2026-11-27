#include <errno.h>
#include <9c/file.h>
#include "../internal/native.h"

int
p9Open(const char *path, int mode)
{
    return p9IntResult(P9C_CALL(OPEN, path, mode, 0, 0, 0));
}

int
p9Create(const char *path, int mode, unsigned long perm)
{
    return p9IntResult(P9C_CALL(CREATE, path, mode, perm, 0, 0));
}

int
p9Remove(const char *path)
{
    return p9IntResult(P9C_CALL(REMOVE, path, 0, 0, 0, 0));
}

int
p9Close(int fd)
{
    return p9IntResult(P9C_CALL(CLOSE, fd, 0, 0, 0, 0));
}

int
p9Dup(int oldfd, int newfd)
{
    return p9IntResult(P9C_CALL(DUP, oldfd, newfd, 0, 0, 0));
}

int
p9Chdir(const char *path)
{
    return p9IntResult(P9C_CALL(CHDIR, path, 0, 0, 0, 0));
}

int
p9Fd2path(int fd, char *buf, int len)
{
    if (len <= 0) { errno = EINVAL; return -1; }
    return p9IntResult(P9C_CALL(FD2PATH, fd, buf, len, 0, 0));
}

long
p9Pread(int fd, void *buf, long count, long long offset)
{
    if (count < 0) { errno = EINVAL; return -1; }
    return __lib10c_sysresult(P9C_CALL(PREAD, fd, buf, count, offset, 0));
}

long
p9Pwrite(int fd, const void *buf, long count, long long offset)
{
    if (count < 0) { errno = EINVAL; return -1; }
    return __lib10c_sysresult(P9C_CALL(PWRITE, fd, buf, count, offset, 0));
}

long
p9Read(int fd, void *buf, long count)
{
    return p9Pread(fd, buf, count, -1LL);
}

long
p9Write(int fd, const void *buf, long count)
{
    return p9Pwrite(fd, buf, count, -1LL);
}

long long
p9Seek(int fd, long long offset, int whence)
{
    long long result;

    /* AMD64 native SEEK writes the result through argument zero. */
    if (p9IntResult(P9C_CALL(SEEK, &result, fd, offset, 0, whence)) < 0)
        return -1;
    return result;
}

int
p9Fauth(int fd, const char *aname)
{
    return p9IntResult(P9C_CALL(FAUTH, fd, aname, 0, 0, 0));
}

int
p9Fversion(int fd, int msize, char *version, int len)
{
    if (msize < 0 || len < 0) { errno = EINVAL; return -1; }
    return p9IntResult(P9C_CALL(FVERSION, fd, msize, version, len, 0));
}

int
p9Stat(const char *path, unsigned char *buf, int len)
{
    if (len < 0) { errno = EINVAL; return -1; }
    return p9IntResult(P9C_CALL(STAT, path, buf, len, 0, 0));
}

int
p9Fstat(int fd, unsigned char *buf, int len)
{
    if (len < 0) { errno = EINVAL; return -1; }
    return p9IntResult(P9C_CALL(FSTAT, fd, buf, len, 0, 0));
}

int
p9Wstat(const char *path, const unsigned char *buf, int len)
{
    if (len < 0) { errno = EINVAL; return -1; }
    return p9IntResult(P9C_CALL(WSTAT, path, buf, len, 0, 0));
}

int
p9Fwstat(int fd, const unsigned char *buf, int len)
{
    if (len < 0) { errno = EINVAL; return -1; }
    return p9IntResult(P9C_CALL(FWSTAT, fd, buf, len, 0, 0));
}
