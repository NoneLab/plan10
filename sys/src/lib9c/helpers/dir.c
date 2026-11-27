/* 9P2000 stat encoding, adapted from libc/9sys/conv{M2D,D2M}.c.
 * Lengths are validated before reading or writing any variable field. */
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <9c/dir.h>
#include <9c/malloc.h>

_Static_assert(sizeof(unsigned short) == 2 && sizeof(unsigned int) == 4 &&
    sizeof(unsigned long long) == 8, "9P scalar sizes");

enum { Fixed = 41, Maxrecord = P9_STATMAX + 2 };

static unsigned long long
getle(const unsigned char *p, unsigned n)
{
    unsigned long long v = 0;
    for (unsigned i = 0; i < n; ++i)
        v |= (unsigned long long)p[i] << (i * 8);
    return v;
}

static void
putle(unsigned char *p, unsigned n, unsigned long long v)
{
    for (unsigned i = 0; i < n; ++i)
        p[i] = (unsigned char)(v >> (8 * i));
}

int
p9Statcheck(const unsigned char *buf, size_t len)
{
    size_t pos = Fixed;
    if (!buf || len < P9_STATFIXLEN || getle(buf, 2) != len - 2)
        goto invalid;
    for (int i = 0; i < 4; ++i) {
        size_t n;
        if (len - pos < 2) goto invalid;
        n = (size_t)getle(buf + pos, 2);
        pos += 2;
        if (n > len - pos || memchr(buf + pos, 0, n)) goto invalid;
        pos += n;
    }
    if (pos == len) return 0;
invalid:
    errno = EPROTO;
    return -1;
}

size_t
p9SizeD2M(const P9Dir *d)
{
    size_t len = P9_STATFIXLEN;
    const char *s[4];
    if (!d) { errno = EINVAL; return 0; }
    s[0] = d->name; s[1] = d->uid; s[2] = d->gid; s[3] = d->muid;
    for (int i = 0; i < 4; ++i) {
        size_t n = s[i] ? strlen(s[i]) : 0;
        if (n > Maxrecord - len) { errno = EMSGSIZE; return 0; }
        len += n;
    }
    return len;
}

size_t
p9ConvD2M(const P9Dir *d, unsigned char *buf, size_t cap)
{
    size_t len = p9SizeD2M(d), pos = Fixed;
    const char *s[4];
    if (!len) return 0;
    if (!buf || cap < 2) { errno = EMSGSIZE; return 0; }
    putle(buf, 2, len - 2);
    if (cap < len) return 2; /* Native two-byte size probe. */
    putle(buf + 2, 2, d->type);
    putle(buf + 4, 4, d->dev);
    buf[8] = d->qid.type;
    putle(buf + 9, 4, d->qid.vers);
    putle(buf + 13, 8, d->qid.path);
    putle(buf + 21, 4, d->mode);
    putle(buf + 25, 4, d->atime);
    putle(buf + 29, 4, d->mtime);
    putle(buf + 33, 8, (unsigned long long)d->length);
    s[0] = d->name; s[1] = d->uid; s[2] = d->gid; s[3] = d->muid;
    for (int i = 0; i < 4; ++i) {
        size_t n = s[i] ? strlen(s[i]) : 0;
        putle(buf + pos, 2, n);
        pos += 2;
        if (n) memcpy(buf + pos, s[i], n);
        pos += n;
    }
    return len;
}

size_t
p9ConvM2D(const unsigned char *buf, size_t len, P9Dir *d,
          char *strings, size_t cap)
{
    size_t pos = Fixed;
    char **s[4];
    if (!d || !strings) { errno = EINVAL; return 0; }
    if (p9Statcheck(buf, len) < 0) return 0;
    /* Four counted strings occupy len - 49 bytes, plus four NULs. */
    if (cap < len - P9_STATFIXLEN + 4) { errno = EMSGSIZE; return 0; }
    d->type = (unsigned short)getle(buf + 2, 2);
    d->dev = (unsigned int)getle(buf + 4, 4);
    d->qid.type = buf[8];
    d->qid.vers = (unsigned int)getle(buf + 9, 4);
    d->qid.path = getle(buf + 13, 8);
    d->mode = (unsigned int)getle(buf + 21, 4);
    d->atime = (unsigned int)getle(buf + 25, 4);
    d->mtime = (unsigned int)getle(buf + 29, 4);
    d->length = (long long)getle(buf + 33, 8);
    s[0] = &d->name; s[1] = &d->uid; s[2] = &d->gid; s[3] = &d->muid;
    for (int i = 0; i < 4; ++i) {
        size_t n = (size_t)getle(buf + pos, 2);
        pos += 2;
        *s[i] = strings;
        memmove(strings, buf + pos, n);
        strings[n] = 0;
        strings += n + 1;
        pos += n;
    }
    return len;
}

void
p9Nulldir(P9Dir *d)
{
    static char empty[] = "";
    memset(d, 0xff, sizeof(*d));
    d->name = d->uid = d->gid = d->muid = empty;
}

static P9Dir *
packdir(const unsigned char *buf, size_t len, long *count)
{
    size_t pos = 0, n = 0;
    P9Dir *d;
    char *str;
    while (pos < len) {
        size_t m;
        if (len - pos < 2) goto invalid;
        m = 2 + (size_t)getle(buf + pos, 2);
        if (m > len - pos || p9Statcheck(buf + pos, m) < 0) goto invalid;
        pos += m;
        ++n;
    }
    *count = 0;
    if (!n) return NULL;
    if (n > ((size_t)-1 - len) / sizeof(*d) || n > LONG_MAX) {
        errno = ENOMEM;
        *count = -1;
        return NULL;
    }
    d = p9Malloc(n * sizeof(*d) + len);
    if (!d) { *count = -1; return NULL; }
    str = (char *)(d + n);
    pos = 0;
    for (size_t i = 0; i < n; ++i) {
        size_t m = 2 + (size_t)getle(buf + pos, 2);
        if (!p9ConvM2D(buf + pos, m, d + i, str, m)) {
            p9Free(d); *count = -1; return NULL;
        }
        str += m;
        pos += m;
    }
    *count = (long)n;
    return d;
invalid:
    errno = EPROTO;
    *count = -1;
    return NULL;
}

static P9Dir *
readstat(const char *path, int fd)
{
    unsigned char *buf = p9Malloc(Maxrecord);
    P9Dir *d = NULL;
    long count;
    int n;
    if (!buf) return NULL;
    n = path ? p9Stat(path, buf, Maxrecord) : p9Fstat(fd, buf, Maxrecord);
    if (n > 0 && n <= Maxrecord && p9Statcheck(buf, (size_t)n) == 0)
        d = packdir(buf, (size_t)n, &count);
    else if (n >= 0) errno = EPROTO;
    p9Free(buf);
    return d;
}

P9Dir *p9Dirstat(const char *path) { return readstat(path, -1); }
P9Dir *p9Dirfstat(int fd) { return readstat(NULL, fd); }

static int
writestat(const char *path, int fd, const P9Dir *d)
{
    size_t len = p9SizeD2M(d);
    unsigned char *buf;
    int result;
    if (!len) return -1;
    buf = p9Malloc(len);
    if (!buf) return -1;
    if (p9ConvD2M(d, buf, len) != len) { p9Free(buf); return -1; }
    result = path ? p9Wstat(path, buf, (int)len) : p9Fwstat(fd, buf, (int)len);
    p9Free(buf);
    return result;
}

int p9Dirwstat(const char *path, const P9Dir *d) { return writestat(path, -1, d); }
int p9Dirfwstat(int fd, const P9Dir *d) { return writestat(NULL, fd, d); }

static long
readdir(int fd, P9Dir **out, int all)
{
    unsigned char *buf = NULL;
    size_t total = 0;
    long n, count = -1;
    if (!out) { errno = EINVAL; return -1; }
    *out = NULL;
    do {
        unsigned char *next;
        if (total > (size_t)-1 - Maxrecord) { errno = ENOMEM; goto done; }
        next = p9Realloc(buf, total + Maxrecord);
        if (!next) goto done;
        buf = next;
        n = p9Read(fd, buf + total, Maxrecord);
        if (n < 0) goto done;
        total += (size_t)n;
    } while (all && n > 0);
    *out = packdir(buf, total, &count);
done:
    p9Free(buf);
    return count;
}

long p9Dirread(int fd, P9Dir **out) { return readdir(fd, out, 0); }
long p9Dirreadall(int fd, P9Dir **out) { return readdir(fd, out, 1); }
