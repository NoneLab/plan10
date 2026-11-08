/* Exercise actual kernel atomic.S primitives against the SysV AMD64 ABI. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

extern void _xinc(long *);
extern long _xdec(long *);
extern int cmpswap486(long *, long, long);

int
main(void)
{
    long ref = 1;

    _xinc(&ref);
    assert(ref == 2);
    assert(_xdec(&ref) == 1 && ref == 1);
    assert(_xdec(&ref) == 0 && ref == 0);
    assert(_xdec(&ref) == -1 && ref == -1);

    ref = 0;
    assert(cmpswap486(&ref, 0, 0x100000001L) == 1);
    assert(ref == 0x100000001L);
    assert(cmpswap486(&ref, 0, 2) == 0);
    assert(ref == 0x100000001L);
    puts("Plan 9/pc64 native atomics: PASS");
    return 0;
}
