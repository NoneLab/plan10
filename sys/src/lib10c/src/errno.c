// Temparary Imeplementation

#include <errno.h>

int _errno = 0;

int* __errno(void)
{
    return &_errno;
}