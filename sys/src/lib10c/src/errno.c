// Temparary Imeplementation

#include <errno.h>
#include <stdlib.h>

int _errno = 0;

int* __errno(void)
{
    return &_errno;
}

void
exits(char *s)
{
    exit(_errno);
}
