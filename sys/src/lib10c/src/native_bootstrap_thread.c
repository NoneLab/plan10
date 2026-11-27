#include <limits.h>
#include <unistd.h>
#include "thread_private.h"

/* Only valid before pthread support. Do not silently bypass locks if a
 * future runtime sets __isthreaded. Avoid abort()/stdio recursion. */
static void
require_single_thread(void)
{
    if (__isthreaded)
        _exit(1);
}

void
__rcmtx_init(struct __rcmtx *m)
{
    require_single_thread();
    *m = (struct __rcmtx){ 0 };
}

int
__rcmtx_enter_try(struct __rcmtx *m)
{
    require_single_thread();
    if (m->depth == UINT_MAX)
        _exit(1);
    ++m->depth;
    return 1;
}

void
__rcmtx_enter(struct __rcmtx *m)
{
    (void)__rcmtx_enter_try(m);
}

void
__rcmtx_leave(struct __rcmtx *m)
{
    require_single_thread();
    if (m->depth == 0)
        _exit(1);
    --m->depth;
}
