#ifndef P9_PC64_EXEC64_H
#define P9_PC64_EXEC64_H

enum 
{
    P9_EXEC64_HEADER_SIZE = 40
};

typedef struct P9Exec64 P9Exec64;
struct P9Exec64
{
    u32int magic;
    u32int text;
    u32int data;
    u32int bss;
    u32int syms;
    u32int entry32;
    u32int spsz;
    u32int pcsz;
    uvlong entry;
};

int p9_exec64_decode(const uchar* raw, ulong len, P9Exec64* out);

#endif