/*
 * AMD64 Plan 9 device configuration.
 *
 * Equivalent to the device table generated from ../pc/pc by mkdevc.
 * Keep the original Dev vtables and configuration semantics; this is not a
 * substitute kernel or synthetic hardware abstraction.
 */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

extern Dev rootdevtab;
extern Dev consdevtab;
extern Dev envdevtab;
extern Dev pipedevtab;
extern Dev procdevtab;
extern Dev mntdevtab;
extern Dev srvdevtab;
extern Dev dupdevtab;
extern Dev kprofdevtab;
extern Dev wddevtab;

Dev *devtab[] = {
    &rootdevtab,
    &consdevtab,
    &envdevtab,
    &pipedevtab,
    &procdevtab,
    &mntdevtab,
    &srvdevtab,
    &dupdevtab,
    &kprofdevtab,
    &wddevtab,
    nil,
};

/* Report the actual machine implementation to #c/config. */
char configfile[] = "dev\n"
                    "\troot\n\tcons\n\tenv\n\tpipe\n\tproc\n"
                    "\tmnt\n\tsrv\n\tdup\n\tkprof\n\twd\n";
char *conffile = "pc64/pc64";
int cpuserver = 0;
ulong kerndate;
