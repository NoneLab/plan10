#include <sys/mman.h>
#include <errno.h>

/* Link-only backend. Plan 9 segattach/segfree are not mmap/munmap, and
 * cannot implement per-page protection or immutable mappings by themselves.
 * Never pretend that a guard page or immutable mapping was installed. */
__dso_hidden void *
HIDDEN(mmap)(void *addr, size_t len, int prot, int flags, int fd, off_t off)
{
    (void)addr; (void)len; (void)prot; (void)flags; (void)fd; (void)off;
    errno = ENOSYS;
    return MAP_FAILED;
}
DEF_STRONG(mmap);

__dso_hidden int
HIDDEN(munmap)(void *addr, size_t len)
{
    (void)addr; (void)len;
    errno = ENOSYS;
    return -1;
}
DEF_STRONG(munmap);

__dso_hidden int
HIDDEN(mprotect)(void *addr, size_t len, int prot)
{
    (void)addr; (void)len; (void)prot;
    errno = ENOSYS;
    return -1;
}
DEF_STRONG(mprotect);

__dso_hidden int
HIDDEN(mimmutable)(void *addr, size_t len)
{
    (void)addr; (void)len;
    errno = ENOSYS;
    return -1;
}
DEF_STRONG(mimmutable);

/* arc4random requires MAP_INHERIT_ZERO; no equivalent exists yet. */
__dso_hidden int
HIDDEN(minherit)(void *addr, size_t len, int inherit)
{
    (void)addr; (void)len; (void)inherit;
    errno = ENOSYS;
    return -1;
}
DEF_STRONG(minherit);
