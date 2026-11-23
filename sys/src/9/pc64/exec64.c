#include "u.h"
#include "exec64.h"
#include <a.out.h>

static u32int
be32(const uchar* p)
{
    return ((u32int)p[0] << 24) | ((u32int)p[1] << 16) | ((u32int)p[2] << 8) | (u32int)p[3];
}

static uvlong
be64(const uchar* p)
{
    return ((uvlong)be32(p) << 32) | be32(p + 4);
}

int
p9_exec64_decode(const uchar *raw, ulong len, P9Exec64 *out)
{
    if (raw == nil || out == nil || len != P9_EXEC64_HEADER_SIZE)
        return -1;

    out->magic = be32(raw);
    if (out->magic != S_MAGIC)
        return -1;

    out->text = be32(raw + 4);
    out->data = be32(raw + 8);
    out->bss = be32(raw + 12);
    out->syms = be32(raw + 16);
    out->entry32 = be32(raw + 20);
    out->spsz = be32(raw + 24);
    out->pcsz = be32(raw + 28);
    out->entry = be64(raw + 32);

    return 0;
}