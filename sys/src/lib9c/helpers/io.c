#include <errno.h>
#include <limits.h>
#include <string.h>
#include <9c/file.h>
#include <9c/dir.h>
#include <9c/malloc.h>

long
p9Readn(int fd, void *buf, long count)
{
    long done = 0;
    if (count < 0) { errno = EINVAL; return -1; }
    while (done < count) {
        long n = p9Read(fd, (char *)buf + done, count - done);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) return done ? done : n;
        done += n;
    }
    return done;
}

static long
chunksize(const P9IOchunk *io, int nio)
{
    long total = 0;
    if (nio < 0 || (nio && !io)) { errno = EINVAL; return -1; }
    for (int i = 0; i < nio; ++i) {
        if (io[i].len > (unsigned long)(LONG_MAX - total) ||
            (io[i].len && !io[i].addr)) {
            errno = EINVAL;
            return -1;
        }
        total += (long)io[i].len;
    }
    return total;
}

long
p9Preadv(int fd, const P9IOchunk *io, int nio, long long offset)
{
    long total = chunksize(io, nio), result, remaining;
    char *buf, *p;
    if (total <= 0) return total;
    buf = p9Malloc((size_t)total);
    if (!buf) return -1;
    result = p9Pread(fd, buf, total, offset);
    p = buf;
    remaining = result;
    for (int i = 0; i < nio && remaining > 0; ++i) {
        long n = (long)io[i].len;
        if (n > remaining) n = remaining;
        if (n) memcpy(io[i].addr, p, (size_t)n);
        p += n;
        remaining -= n;
    }
    p9Free(buf);
    return result;
}

long
p9Pwritev(int fd, const P9IOchunk *io, int nio, long long offset)
{
    long total = chunksize(io, nio), result;
    char *buf, *p;
    if (total <= 0) return total;
    buf = p9Malloc((size_t)total);
    if (!buf) return -1;
    p = buf;
    for (int i = 0; i < nio; ++i) {
        if (io[i].len) memcpy(p, io[i].addr, io[i].len);
        p += io[i].len;
    }
    result = p9Pwrite(fd, buf, total, offset);
    p9Free(buf);
    return result;
}

long
p9Readv(int fd, const P9IOchunk *io, int nio)
{
    return p9Preadv(fd, io, nio, -1LL);
}

long
p9Writev(int fd, const P9IOchunk *io, int nio)
{
    return p9Pwritev(fd, io, nio, -1LL);
}

char *
p9Getwd(char *buf, int len)
{
    int fd, result, saved;
    if (!buf || len <= 0) { errno = EINVAL; return NULL; }
    fd = p9Open(".", P9_OREAD);
    if (fd < 0) return NULL;
    result = p9Fd2path(fd, buf, len);
    saved = errno;
    (void)p9Close(fd);
    errno = saved;
    return result < 0 ? NULL : buf;
}

/* Plan 9 access() tests the native open mode (read+exec, etc. are approximate). */
int
p9Access(const char *path, int mode)
{
    static const int omode[] = { P9_OREAD, P9_OEXEC, P9_OWRITE,
        P9_ORDWR, P9_OREAD, P9_OEXEC, P9_ORDWR, P9_ORDWR };
    int fd;
    if (mode < 0 || mode > 7) { errno = EINVAL; return -1; }
    if (!mode) {
        P9Dir *d = p9Dirstat(path);
        if (!d) return -1;
        p9Free(d);
        return 0;
    }
    fd = p9Open(path, omode[mode]);
    if (fd < 0) return -1;
    return p9Close(fd);
}
