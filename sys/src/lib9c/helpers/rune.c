/* Locale-independent UTF-8 and Rune helpers based on libc/port/rune.c
 * and the original UTF string helpers. */
#include <stddef.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <9c/rune.h>
#include <9c/malloc.h>

static int
runesize(unsigned char c)
{
    if (c < 0x80) return 1;
    if (c >= 0xc2 && c <= 0xdf) return 2;
    if (c >= 0xe0 && c <= 0xef) return 3;
    if (c >= 0xf0 && c <= 0xf4) return 4;
    return 1;
}

/* n bounds every byte access; invalid input consumes one byte. */
static int
decode(P9Rune *out, const unsigned char *s, size_t n)
{
    int len;
    P9Rune r;
    if (!n) return 0;
    len = runesize(s[0]);
    if (s[0] < 0x80) { *out = s[0]; return 1; }
    if (len == 1 || (size_t)len > n) goto invalid;
    r = s[0] & (0x7fU >> len);
    for (int i = 1; i < len; ++i) {
        if ((s[i] & 0xc0) != 0x80) goto invalid;
        r = (r << 6) | (s[i] & 0x3f);
    }
    if ((len == 2 && r < 0x80) || (len == 3 && r < 0x800) ||
        (len == 4 && r < 0x10000) || r > P9_Runemax ||
        (r >= 0xd800 && r <= 0xdfff)) goto invalid;
    *out = r;
    return len;
invalid:
    *out = P9_Runeerror;
    return 1;
}

int
p9Chartorune(P9Rune *r, const char *s)
{
    size_t n = 1;
    int need = runesize((unsigned char)*s);
    while (n < (size_t)need && s[n]) ++n;
    return decode(r, (const unsigned char *)s, n);
}

int
p9Runetochar(char *s, const P9Rune *r)
{
    P9Rune c = *r;
    int n;
    if (c > P9_Runemax || (c >= 0xd800 && c <= 0xdfff)) c = P9_Runeerror;
    if (c < 0x80) { *s = (char)c; return 1; }
    n = c < 0x800 ? 2 : c < 0x10000 ? 3 : 4;
    for (int i = n - 1; i > 0; --i) { s[i] = (char)(0x80 | (c & 0x3f)); c >>= 6; }
    s[0] = (char)(((0xffU << (8 - n)) & 0xffU) | c);
    return n;
}

int p9Runelen(long r) { P9Rune c = r < 0 || r > P9_Runemax ? P9_Runeerror : (P9Rune)r; char b[4]; return p9Runetochar(b, &c); }
int p9Runenlen(const P9Rune *r, int n)
{
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        int k = p9Runelen(r[i]);
        if (sum > INT_MAX - k) { errno = EOVERFLOW; return -1; }
        sum += k;
    }
    return sum;
}
int p9Fullrune(const char *s, int n) { return n > 0 && n >= runesize((unsigned char)*s); }
int p9Utfnlen(const char *s, long n)
{
    int count = 0;
    while (n > 0 && *s) {
        P9Rune r;
        int len;
        if (runesize((unsigned char)*s) > n) break;
        len = decode(&r, (const unsigned char *)s, (size_t)n);
        s += len; n -= len;
        if (count == INT_MAX) { errno = EOVERFLOW; return -1; }
        ++count;
    }
    return count;
}
int p9Utflen(const char *s)
{
    size_t n = strlen(s);
    if (n > LONG_MAX) { errno = EOVERFLOW; return -1; }
    return p9Utfnlen(s, (long)n);
}
char *p9Utfrune(const char *s, long r)
{
    for (;;) { P9Rune c; int n = p9Chartorune(&c, s); if (c == r) return (char *)s; if (!*s) return NULL; s += n; }
}
char *p9Utfrrune(const char *s, long r)
{
    char *last = NULL;
    for (;;) { P9Rune c; int n = p9Chartorune(&c, s); if (c == r) last = (char *)s; if (!*s) return last; s += n; }
}
char *p9Utfutf(const char *s, const char *needle)
{
    size_t n = strlen(needle);
    if (!n) return (char *)s;
    while (*s) { P9Rune r; if (!strncmp(s, needle, n)) return (char *)s; s += p9Chartorune(&r, s); }
    return NULL;
}
char *p9Utfecpy(char *dst, char *end, const char *src)
{
    if (dst >= end) return dst;
    while (*src) {
        P9Rune r;
        int n = p9Chartorune(&r, src);
        if (end - dst <= n) break;
        memcpy(dst, src, n); dst += n; src += n;
    }
    *dst = 0;
    return dst;
}
long p9Runestrlen(const P9Rune *s) { const P9Rune *p = s; while (*p) ++p; return (long)(p - s); }
P9Rune *p9Runestrcpy(P9Rune *dst, const P9Rune *src) { P9Rune *p = dst; while ((*p++ = *src++)) {} return dst; }
P9Rune *p9Runestrcat(P9Rune *dst, const P9Rune *src) { p9Runestrcpy(dst + p9Runestrlen(dst), src); return dst; }
P9Rune *p9Runestrchr(const P9Rune *s, P9Rune r) { while (*s && *s != r) ++s; return *s == r ? (P9Rune *)s : NULL; }
P9Rune *p9Runestrrchr(const P9Rune *s, P9Rune r) { const P9Rune *p = NULL; do { if (*s == r) p = s; } while (*s++); return (P9Rune *)p; }
int p9Runestrcmp(const P9Rune *a, const P9Rune *b) { while (*a && *a == *b) { ++a; ++b; } return (*a > *b) - (*a < *b); }
int p9Runestrncmp(const P9Rune *a, const P9Rune *b, long n) { while (n-- > 0) { if (*a != *b) return (*a > *b) - (*a < *b); if (!*a) break; ++a; ++b; } return 0; }
P9Rune *p9Runestrncpy(P9Rune *dst, const P9Rune *src, long n) { P9Rune *p = dst; while (n-- > 0) { *p++ = *src; if (*src) ++src; } return dst; }
P9Rune *p9Runestrncat(P9Rune *dst, const P9Rune *src, long n) { P9Rune *p = dst + p9Runestrlen(dst); while (n-- > 0 && *src) *p++ = *src++; *p = 0; return dst; }
P9Rune *p9Runestrecpy(P9Rune *dst, P9Rune *end, const P9Rune *src) { if (dst >= end) return dst; while (dst + 1 < end && *src) *dst++ = *src++; *dst = 0; return dst; }
P9Rune *p9Runestrdup(const P9Rune *s) { size_t n = (size_t)p9Runestrlen(s); P9Rune *p; if (n >= (size_t)-1 / sizeof(*s)) { errno = ENOMEM; return NULL; } p = p9Malloc((n + 1) * sizeof(*s)); if (p) memcpy(p, s, (n + 1) * sizeof(*s)); return p; }
P9Rune *p9Runestrstr(const P9Rune *s, const P9Rune *needle) { long n = p9Runestrlen(needle); if (!n) return (P9Rune *)s; for (; *s; ++s) if (!p9Runestrncmp(s, needle, n)) return (P9Rune *)s; return NULL; }
