#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <9c/error.h>
#include <9c/file.h>
#include <9c/process.h>
#include "../internal/native.h"

int
p9Errstr(char *buf, unsigned int len)
{
    /* ERRSTR itself must not use the error-string translation adapter. */
    if (!buf || !len) { errno = EINVAL; return -1; }
    return (int)P9C_CALL(ERRSTR, buf, len, 0, 0, 0);
}

void
p9Rerrstr(char *buf, unsigned int len)
{
    char tmp[P9_ERRMAX] = {0};
    size_t n;

    if (!buf || !len) return;
    if (p9Errstr(tmp, sizeof(tmp)) < 0) { *buf = 0; return; }
    n = strlen(tmp);
    if (n >= len) {
        n = len - 1;
        /* Do not cut a UTF-8 sequence at a continuation byte. */
        while (n && ((unsigned char)tmp[n] & 0xc0) == 0x80) --n;
    }
    memcpy(buf, tmp, n);
    buf[n] = 0;
    (void)p9Errstr(tmp, sizeof(tmp));
}

void
p9Werrstr(const char *fmt, ...)
{
    char buf[P9_ERRMAX] = {0};
    va_list ap;

    va_start(ap, fmt);
    (void)vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    (void)p9Errstr(buf, sizeof(buf));
}

void
p9Sysfatal(const char *fmt, ...)
{
    char buf[P9_ERRMAX] = {0};
    va_list ap;

    va_start(ap, fmt);
    (void)vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    (void)p9Write(2, buf, (long)strlen(buf));
    (void)p9Write(2, "\n", 1);
    p9Exits(buf[0] ? buf : "fatal");
}
