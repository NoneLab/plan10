/* Native PC-compatible machine halt and reset for the Plan 9 kernel. */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

void
exit(int ispanic)
{
    active.exiting = 1;
    if(ispanic)
        active.ispanic = ispanic;
    chandevshutdown();
    /* ISA reset line: standard PC keyboard controller pulse. */
    splhi();
    outb(0xcf9, 0x06);
    outb(0x64, 0xfe);
    for(;;)
        __asm__ volatile("hlt");
}

void
reboot(void *entry, void *code, ulong size)
{
    /* An in-memory hot-reload requires a separate full transition plan. */
    if(entry != nil || code != nil || size != 0)
        panic("pc64: replacing a running kernel is not supported");
    exit(0);
}

/* Plan 9's libc/9sys abort() is a user-mode null dereference; the kernel
 * must instead produce a deterministic panic that cannot return. */
void
abort(void)
{
    panic("pc64: abort invoked in kernel");
    for(;;)
        __asm__ volatile("cli; hlt" ::: "memory");
}
