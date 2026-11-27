#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <9c/env.h>
#include <9c/file.h>
#include <9c/malloc.h>

static int
envpath(char *buf, size_t len, const char *name)
{
    int n;
    if (!name || !*name || strchr(name, '/') || !strcmp(name, ".") ||
        !strcmp(name, "..")) { errno = EINVAL; return -1; }
    n = snprintf(buf, len, "/env/%s", name);
    if (n < 0 || (size_t)n >= len) { errno = ENAMETOOLONG; return -1; }
    return 0;
}

char *
p9Getenv(const char *name)
{
    char path[256], *buf = NULL;
    int fd, saved;
    long long len;
    long n;
    if (envpath(path, sizeof(path), name) < 0) return NULL;
    fd = p9Open(path, P9_OREAD);
    if (fd < 0) return NULL;
    len = p9Seek(fd, 0, 2);
    if (len < 0) goto done;
    if (len >= LONG_MAX) { errno = ENOMEM; goto done; }
    if (p9Seek(fd, 0, 0) < 0) goto done;
    buf = p9Malloc((size_t)len + 1);
    if (!buf) goto done;
    n = p9Readn(fd, buf, (long)len);
    if (n < 0) { p9Free(buf); buf = NULL; goto done; }
    for (long i = 0; i + 1 < n; ++i) if (!buf[i]) buf[i] = ' ';
    buf[n] = 0;
done:
    saved = errno;
    (void)p9Close(fd);
    errno = saved;
    return buf;
}

int
p9Putenv(const char *name, const char *value)
{
    char path[256];
    int fd, saved;
    long n, len;
    if (!value) { errno = EINVAL; return -1; }
    if (envpath(path, sizeof(path), name) < 0) return -1;
    fd = p9Create(path, P9_OWRITE, 0664);
    if (fd < 0) return -1;
    len = (long)strlen(value);
    n = p9Write(fd, value, len);
    saved = errno;
    (void)p9Close(fd);
    if (n != len) { errno = n < 0 ? saved : EIO; return -1; }
    return 0;
}

static int
readtext(const char *path, char *buf, size_t len)
{
    int fd = p9Open(path, P9_OREAD), saved;
    long n;
    if (fd < 0) return -1;
    n = p9Read(fd, buf, (long)len - 1);
    saved = errno;
    (void)p9Close(fd);
    errno = saved;
    if (n <= 0) return -1;
    buf[n] = 0;
    return 0;
}

const char *
p9Getuser(void)
{
    static char user[64];
    return readtext("/dev/user", user, sizeof(user)) < 0 ? "none" : user;
}

const char *
p9Sysname(void)
{
    static char name[128];
    if (readtext("#c/sysname", name, sizeof(name)) < 0) name[0] = 0;
    return name;
}

int
p9Getppid(void)
{
    char buf[32], *end;
    long n;
    if (readtext("/dev/ppid", buf, sizeof(buf)) < 0) return -1;
    errno = 0;
    n = strtol(buf, &end, 10);
    if (errno || end == buf || n < 0 || n > INT_MAX ||
        end[strspn(end, " \t\r\n")]) { errno = EPROTO; return -1; }
    return (int)n;
}
