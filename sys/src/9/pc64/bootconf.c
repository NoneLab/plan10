#include <u.h>
#include <libc.h>
#include "../boot/boot.h"

Method method[] =
{
    {
        "local",
        configlocal,
        connectlocal,
        nil
    },
    {
        nil,
        nil,
        nil,
        nil
    }
};

int cpuflag = 0;
char* rootdir = "/root";
char* bootdisk = "#S/sdC0/";
int (*cfs)(int) = nil;
char* argv0;

void
main(int argc, char** argv)
{
    boot(argc, argv);
}