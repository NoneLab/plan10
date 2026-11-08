/* Native kernel implementation of Plan 9's thread-local error retrieval.
 * The libc/9sys wrapper uses a userspace syscall and cannot run in /port.
 */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

extern char *utfecpy(char *, char *, char *);

void
rerrstr(char *buf, uint nbuf)
{
    if(nbuf == 0)
        return;
    if(up == nil || up->syserrstr == nil){
        buf[0] = 0;
        return;
    }
    utfecpy(buf, buf + nbuf, up->syserrstr);
}
