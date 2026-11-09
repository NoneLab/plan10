#include <u.h>
#include <libc.h>
#include "sys.h"
#include "syscall.h"

int
open(char *path, int mode)
{
    return (int)p9_syscall5(OPEN, (ulong)path, mode, 0, 0, 0);
}

int
bind(char *name, char *old, int flag)
{
    return (int)p9_syscall5(BIND, (ulong)name, (ulong)old, flag, 0, 0);
}

int
exec(char *file, char *argv[])
{
    return (int)p9_syscall5(EXEC, (ulong)file, (ulong)argv, 0, 0, 0);
}

void
rerrstr(char *buf, uint nbuf)
{
    /* ERRSTR swaps kernel's current and system error strings. */
    if(nbuf != 0){
        p9_syscall5(ERRSTR, (ulong)buf, nbuf, 0, 0, 0);
        buf[nbuf-1] = 0;
    }
}

void
_exits(char *status)
{
    p9_syscall5(EXITS, (ulong)status, 0, 0, 0, 0);
    for(;;)
        __asm__ volatile("ud2");
}