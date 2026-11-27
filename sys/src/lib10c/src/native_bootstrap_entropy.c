#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

/* AMD64 hardware seed source. Do not use the current timing/LCG #c/random
 * as a cryptographic entropy source. No RDSEED => fail explicitly. */
static int
has_rdseed(void)
{
    unsigned int a, b, c, d;

    a = 0;
    c = 0;
    __asm__ volatile("cpuid" : "+a"(a), "=b"(b), "+c"(c), "=d"(d));
    if (a < 7)
        return 0;
    a = 7;
    c = 0;
    __asm__ volatile("cpuid" : "+a"(a), "=b"(b), "+c"(c), "=d"(d));
    return (b & (1U << 18)) != 0;
}

static int
seed_word(uint64_t *word)
{
    unsigned int i;
    for (i = 0; i < 128; ++i) {
        uint64_t v;
        unsigned char ok;
        __asm__ volatile("rdseed %0; setc %1" : "=&r"(v), "=qm"(ok) : : "cc");
        if (ok) {
            *word = v;
            return 0;
        }
        __asm__ volatile("pause");
    }
    return -1;
}

__dso_hidden int
HIDDEN(getentropy)(void *buf, size_t len)
{
    unsigned char tmp[256];
    uint64_t word = 0;
    size_t pos, chunk;

    if (len > sizeof(tmp)) {
        errno = EIO;
        return -1;
    }
    if (len == 0)
        return 0;
    if (buf == NULL) {
        errno = EFAULT;
        return -1;
    }
    if (!has_rdseed()) {
        errno = ENOSYS;
        return -1;
    }
    for (pos = 0; pos < len; pos += chunk) {
        if (seed_word(&word) != 0) {
            explicit_bzero(tmp, sizeof(tmp));
            explicit_bzero(&word, sizeof(word));
            errno = EIO;
            return -1;
        }
        chunk = len - pos;
        if (chunk > sizeof(word))
            chunk = sizeof(word);
        memcpy(tmp + pos, &word, chunk);
    }
    memcpy(buf, tmp, len);
    explicit_bzero(tmp, sizeof(tmp));
    explicit_bzero(&word, sizeof(word));
    return 0;
}
DEF_STRONG(getentropy);
