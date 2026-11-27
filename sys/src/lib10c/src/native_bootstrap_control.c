#include <sys/types.h>
#include <sys/sysctl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

/* Only the fixed AMD64 page size is currently supported. Other MIBs fail. */
__dso_hidden int
HIDDEN(sysctl)(const int *name, u_int namelen, void *oldp, size_t *oldlenp,
    void *newp, size_t newlen)
{
    int value;

    if (name == NULL || namelen == 0 || (oldp != NULL && oldlenp == NULL)) {
        errno = EINVAL;
        return -1;
    }
    if (namelen != 2 || name[0] != CTL_HW || name[1] != HW_PAGESIZE) {
        errno = ENOTSUP;
        return -1;
    }
    if (newp != NULL || newlen != 0) {
        errno = EPERM;
        return -1;
    }
    value = 4096;
    if (oldlenp != NULL) {
        size_t capacity = oldp != NULL ? *oldlenp : 0;
        *oldlenp = sizeof(value);
        if (oldp != NULL) {
            if (capacity < sizeof(value)) {
                errno = ENOMEM;
                return -1;
            }
            memcpy(oldp, &value, sizeof(value));
        }
    }
    return 0;
}
DEF_STRONG(sysctl);

/* Native credentials do not yet track POSIX set-id execution history.
 * Conservatively disable environment-controlled privileged configuration. */
__dso_hidden int
HIDDEN(issetugid)(void)
{
    return 1;
}
DEF_STRONG(issetugid);
