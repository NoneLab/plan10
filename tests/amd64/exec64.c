/* Decode the original Plan 9 S_MAGIC extended 6.out on-disk ABI. */
extern int puts(const char *);
#include <assert.h>
#include "u.h"
#include "exec64.h"
#include <a.out.h>

static void
put32(uchar *p, u32int v)
{
    p[0] = v >> 24;
    p[1] = v >> 16;
    p[2] = v >> 8;
    p[3] = v;
}

int
main(void)
{
    uchar raw[P9_EXEC64_HEADER_SIZE] = {0};
    P9Exec64 e;

    assert(sizeof raw == 40);
    put32(raw, S_MAGIC);
    put32(raw + 4, 0x1234);
    put32(raw + 8, 0x5678);
    put32(raw + 12, 0x99);
    put32(raw + 20, 0);  /* native entry lives in expanded header */
    put32(raw + 32, 0x00000001);
    put32(raw + 36, 0x23456789);
    assert(p9_exec64_decode(raw, 40, &e) == 0);
    assert(e.magic == S_MAGIC && e.text == 0x1234);
    assert(e.data == 0x5678 && e.bss == 0x99);
    assert(e.entry == 0x0000000123456789ULL);
    assert(p9_exec64_decode(raw, 32, &e) == -1);
    put32(raw, I_MAGIC);
    assert(p9_exec64_decode(raw, 40, &e) == -1);
    puts("Plan 9/pc64 S_MAGIC extended 6.out decoder: PASS");
    return 0;
}