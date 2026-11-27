#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>

typedef void (*crtinit_t)(void);
typedef void (*crtfini_t)(void);

extern crtinit_t __preinit_array_start[] __attribute__((weak));
extern crtinit_t __preinit_array_end[] __attribute__((weak));
extern crtinit_t __init_array_start[] __attribute__((weak));
extern crtinit_t __init_array_end[] __attribute__((weak));

extern crtfini_t __fini_array_start[] __attribute__((weak));
extern crtfini_t __fini_array_end[] __attribute__((weak));

static void 
__init(void)
{
    size_t count = 0;
    size_t i = 0;

    count = __preinit_array_end - __preinit_array_start;
    for (i = 0; i < count; i++)
        __preinit_array_start[i]();

    count = __init_array_end - __init_array_start;
    printf("Count : %d\n", count);
    for (i = 0; i < count; i++)
        __init_array_start[i]();
}

static void
__fini()
{
    size_t count = 0;
    size_t i = 0;

    count = __fini_array_end - __fini_array_start;
    for (i = 0; i < count; i++)
        __fini_array_start[i]();
}

void
setprogname(const char *p)
{
    while (*p != '\0') {
        if (*p == '/')
            __progname = p + 1;
        p++;
    }
}


void
__lib10c_init(int argc, char **argv)
{
    if (argc > 0 && argv[0] != NULL)
        setprogname(argv[0]);

#ifdef P9_LIB10C_BOOTSTRAP
    setvbuf(stdin,  NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
#endif

    __init();
}

void
__lib10c_fini(void)
{
    __fini();
}