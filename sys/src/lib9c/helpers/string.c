#include <string.h>
#include <errno.h>
#include <9c/rune.h>
#include <9c/malloc.h>
#include <9c/string.h>

/* Quoting follows libc/port/tokenize.c: a doubled quote inside a
 * single-quoted section represents one literal quote. */
int
p9Tokenize(char *s, char **args, int maxargs)
{
    int n = 0;
    while (n < maxargs) {
        char *out;
        int quoted = 0;
        s += strspn(s, " \t\r\n");
        if (!*s) break;
        args[n++] = out = s;
        while (*s && (quoted || !strchr(" \t\r\n", *s))) {
            if (*s != '\'') { *out++ = *s++; continue; }
            ++s;
            if (quoted && *s == '\'') { *out++ = *s++; continue; }
            quoted = !quoted;
        }
        if (*s) ++s;
        *out = 0;
    }
    return n;
}

char *
p9Strecpy(char *dst, char *end, const char *src)
{
    if (dst >= end) return dst;
    while (dst + 1 < end && *src) *dst++ = *src++;
    *dst = 0;
    return dst;
}

/* Native cistr* operates on ASCII case, independent of locale. */
static int
fold(unsigned char c)
{
    return c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c;
}

int
p9Cistrcmp(const char *a, const char *b)
{
    while (*a && fold(*a) == fold(*b)) { ++a; ++b; }
    return fold(*a) - fold(*b);
}

int
p9Cistrncmp(const char *a, const char *b, int n)
{
    while (n-- > 0) {
        int d = fold(*a) - fold(*b);
        if (d || !*a) return d;
        ++a; ++b;
    }
    return 0;
}

char *
p9Cistrstr(const char *s, const char *needle)
{
    size_t n = strlen(needle);
    if (!n) return (char *)s;
    for (; *s; ++s) {
        const char *a = s, *b = needle;
        while (*b && *a && fold(*a) == fold(*b)) { ++a; ++b; }
        if (!*b) return (char *)s;
    }
    return NULL;
}

int
p9Getfields(char *s, char **args, int maxargs, int merge, const char *sep)
{
    int n = 0, intok = 0;
    if (maxargs <= 0) return 0;
    args[0] = s;
    if (!merge) ++n;
    while (*s) {
        P9Rune r;
        int k = p9Chartorune(&r, s);
        if (p9Utfrune(sep, r)) {
            if (n >= maxargs) break;
            *s = 0;
            intok = 0;
            args[n] = s + k;
            if (!merge) ++n;
        } else if (merge && !intok) {
            if (n >= maxargs) break;
            ++n;
            intok = 1;
        }
        s += k;
    }
    return n;
}

/* Preserve quotes in returned fields (unlike p9Tokenize). */
int
p9Gettokens(char *s, char **args, int maxargs, const char *sep)
{
    int n = 0;
    while (n < maxargs) {
        P9Rune r;
        int k, quoted = 0;
        while (*s && (k = p9Chartorune(&r, s), p9Utfrune(sep, r))) {
            memset(s, 0, k);
            s += k;
        }
        if (!*s) break;
        args[n++] = s;
        while (*s) {
            k = p9Chartorune(&r, s);
            if (!quoted && p9Utfrune(sep, r)) break;
            if (r == '\'') {
                if (quoted && s[1] == '\'') { s += 2; continue; }
                quoted = !quoted;
            }
            s += k;
        }
    }
    return n;
}

/* Keep #device attach specifications intact, as cleanname(2) does. */
char *
p9Cleanname(char *path)
{
    char *root = path, *src, *dst, *floor;
    int rooted = 0;
    if (*root == '#') {
        P9Rune r;
        if (!root[1]) return path;
        root += 1 + p9Chartorune(&r, root + 1);
        while (*root && *root != '/') ++root;
        if (!*root) return path;
        ++root;
        rooted = 1;
    } else if (*root == '/') { ++root; rooted = 1; }
    src = dst = floor = root;
    while (*src) {
        char *start;
        size_t len;
        while (*src == '/') ++src;
        if (!*src) break;
        start = src;
        while (*src && *src != '/') ++src;
        len = (size_t)(src - start);
        if (len == 1 && start[0] == '.') continue;
        if (len == 2 && start[0] == '.' && start[1] == '.') {
            if (dst > floor) {
                while (dst > floor && dst[-1] != '/') --dst;
                if (dst > floor) --dst;
                continue;
            }
            if (rooted) continue;
            if (dst > root) *dst++ = '/';
            *dst++ = '.'; *dst++ = '.';
            floor = dst;
            continue;
        }
        if (dst > root) *dst++ = '/';
        memmove(dst, start, len);
        dst += len;
    }
    if (dst == path) *dst++ = '.';
    /* Preserve a real root slash, including #/ device roots. */
    if (dst == root && rooted && root > path + 1 && path[0] == '#') --dst;
    *dst = 0;
    return path;
}

char *
p9Quotestrdup(const char *s)
{
    size_t n = strlen(s), quotes = 0;
    int need = !n;
    char *out, *p;
    for (size_t i = 0; i < n; ++i) {
        if (s[i] == '\'') { ++quotes; need = 1; }
        else if ((unsigned char)s[i] <= ' ') need = 1;
    }
    if (!need) return p9Strdup(s);
    if (n > (size_t)-1 - 3 || quotes > (size_t)-1 - n - 3) {
        errno = ENOMEM; return NULL;
    }
    out = p9Malloc(n + quotes + 3);
    if (!out) return NULL;
    p = out; *p++ = '\'';
    for (; *s; ++s) { if (*s == '\'') *p++ = '\''; *p++ = *s; }
    *p++ = '\''; *p = 0;
    return out;
}

char *
p9Unquotestrdup(const char *s)
{
    char *out = p9Strdup(s), *p;
    int quoted = 0;
    if (!out) return NULL;
    p = out;
    while (*s && (quoted || (*s != ' ' && *s != '\t'))) {
        if (*s != '\'') { *p++ = *s++; continue; }
        ++s;
        if (quoted && *s == '\'') { *p++ = *s++; continue; }
        quoted = !quoted;
    }
    memcpy(p, s, strlen(s) + 1);
    return out;
}
