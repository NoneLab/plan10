#include <u.h>
#include <libc.h>

int
execl(char *f, ...)
{
    va_list ap;
    char* args[64], *s;
    int n = 0;

    va_start(ap, f);
    while(n < (int)nelem(args) - 1)
    {
        s = va_arg(ap, char*);
        args[n++] = s;
        if (s == nil)
            break;
    }
    va_end(ap);

    if (n == nelem(args) - 1 && args[n - 1] != nil)
    {
        werrstr("execl: too many arguments");
        return -1;
    }

    return exec(f, args);
}