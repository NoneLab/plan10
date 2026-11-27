#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <9c/process.h>
#include <9c/file.h>
#include <9c/malloc.h>
#include <9c/string.h>

static int
parseulong(const char *s, unsigned long *out)
{
    unsigned long n = 0;
    if (!*s) return -1;
    for (; *s; ++s) {
        unsigned d;
        if (*s < '0' || *s > '9') return -1;
        d = (unsigned)(*s - '0');
        if (n > (ULONG_MAX - d) / 10) return -1;
        n = n * 10 + d;
    }
    *out = n;
    return 0;
}

static int
awaitfields(char *buf, int cap, unsigned long fields[4], char **msg)
{
    char *args[6];
    int n = p9Await(buf, cap - 1);
    if (n < 0) return -1;
    if (n >= cap - 2) { errno = EMSGSIZE; return -1; }
    buf[n] = 0;
    if (p9Tokenize(buf, args, 6) != 5) goto invalid;
    for (int i = 0; i < 4; ++i)
        if (parseulong(args[i], &fields[i]) < 0) goto invalid;
    if (fields[0] == 0 || fields[0] > INT_MAX) goto invalid;
    *msg = args[4];
    return 0;
invalid:
    errno = EPROTO;
    return -1;
}

P9Waitmsg *
p9Wait(void)
{
    char buf[512], *msg;
    unsigned long v[4];
    P9Waitmsg *w;
    size_t n;
    if (awaitfields(buf, sizeof(buf), v, &msg) < 0) return NULL;
    n = strlen(msg) + 1;
    w = p9Malloc(sizeof(*w) + n);
    if (!w) return NULL;
    w->pid = (int)v[0];
    for (int i = 0; i < 3; ++i) w->time[i] = v[i + 1];
    w->msg = (char *)(w + 1);
    memcpy(w->msg, msg, n);
    return w;
}

int
p9Waitpid(void)
{
    char buf[512], *msg;
    unsigned long v[4];
    return awaitfields(buf, sizeof(buf), v, &msg) < 0 ? -1 : (int)v[0];
}

int
p9Postnote(int group, int pid, const char *note)
{
    char path[64];
    int fd, saved;
    long n, len;
    if ((group != P9_PNPROC && group != P9_PNGROUP) || pid <= 0 || !note) {
        errno = EINVAL; return -1;
    }
    snprintf(path, sizeof(path), "/proc/%d/%s", pid,
        group == P9_PNPROC ? "note" : "notepg");
    fd = p9Open(path, P9_OWRITE);
    if (fd < 0) return -1;
    len = (long)strlen(note);
    n = p9Write(fd, note, len);
    saved = errno;
    (void)p9Close(fd);
    if (n != len) { errno = n < 0 ? saved : EIO; return -1; }
    return 0;
}

int
p9Iounit(int fd)
{
    char buf[256], path[64], *args[10];
    unsigned long v;
    long n;
    snprintf(path, sizeof(path), "#d/%dctl", fd);
    fd = p9Open(path, P9_OREAD);
    if (fd < 0) return 0;
    n = p9Read(fd, buf, sizeof(buf)-1);
    (void)p9Close(fd);
    if (n <= 0) return 0;
    buf[n] = 0;
    if (p9Tokenize(buf, args, 10) != 10 ||
        parseulong(args[7], &v) < 0 || v > INT_MAX) return 0;
    return (int)v;
}

int
p9Getpid(void)
{
    return (int)getpid();
}

/* The original 32-bit execl took &f+1; that is invalid with SysV AMD64
 * register arguments. Build an explicit NULL-terminated argv instead. */
int
p9Execl(const char *path, const char *arg0, ...)
{
    va_list ap;
    size_t n = 0;
    const char *arg;
    char **argv;
    int r, saved;
    va_start(ap, arg0);
    for (arg = arg0; arg; arg = va_arg(ap, const char *)) {
        if (n >= (size_t)-1 / sizeof(*argv) - 1) {
            va_end(ap); errno = E2BIG; return -1;
        }
        ++n;
    }
    va_end(ap);
    argv = p9Malloc((n + 1) * sizeof(*argv));
    if (!argv) return -1;
    va_start(ap, arg0);
    arg = arg0;
    for (size_t i = 0; i < n; ++i) {
        argv[i] = (char *)arg;
        arg = va_arg(ap, const char *);
    }
    va_end(ap);
    argv[n] = NULL;
    r = p9Exec(path, argv);
    saved = errno;
    p9Free(argv);
    errno = saved;
    return r;
}
