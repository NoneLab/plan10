#ifndef P9_AMD64_SYSCALL_H
#define P9_AMD64_SYSCALL_H

long p9_syscall5(
    unsigned long nr,
    unsigned long a0,
    unsigned long a1,
    unsigned long a2,
    unsigned long a3,
    unsigned long a4
);

#endif