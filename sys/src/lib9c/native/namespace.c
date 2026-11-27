#include <9c/namespace.h>
#include "../internal/native.h"

int
p9Mount(int fd, int afd, const char *path, int flags, const char *spec)
{
    return p9IntResult(P9C_CALL(MOUNT, fd, afd, path, flags, spec));
}

int
p9Bind(const char *source, const char *target, int flags)
{
    return p9IntResult(P9C_CALL(BIND, source, target, flags, 0, 0));
}

int
p9Unmount(const char *source, const char *target)
{
    return p9IntResult(P9C_CALL(UNMOUNT, source, target, 0, 0, 0));
}
