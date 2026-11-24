#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

#ifdef P9_HAVE_BOOT6_IMAGE
#include "boot6_amd64.h"
#endif

#ifdef P9_HAVE_KFS_IMAGE
#include "kfs6_amd64.h"
#endif

#ifdef P9_HAVE_INIT_IMAGE
#include "init6_amd64.h"
#endif

#ifdef P9_HAVE_RC_IMAGE
#include "rc6_amd64.h"
#endif

#ifdef P9_HAVE_RCMAIN_DATA
#include "rcmain_data.h"
#endif

void
bootlinks()
{
#ifdef P9_HAVE_BOOT6_IMAGE
    addbootfile("boot", p9_boot6_image, sizeof(p9_boot6_image));
    print("pc64: registered original /boot/boot, %lud bytes\n", (ulong)sizeof(p9_boot6_image));
#endif
#ifdef P9_HAVE_KFS_IMAGE
    addbootfile("kfs", p9_kfs6_image, sizeof(p9_kfs6_image));
    print("pc64: registered original /boot/kfs, %lud bytes\n", (ulong)sizeof(p9_kfs6_image));
#endif
#ifdef P9_HAVE_INIT_IMAGE
    addbootfile("init", p9_init6_image, sizeof(p9_init6_image));
    print("pc64: registered original /boot/init, %lud bytes\n", (ulong)sizeof(p9_init6_image));
#endif
#ifdef P9_HAVE_RC_IMAGE
    addbootfile("rc", p9_rc6_image, sizeof(p9_rc6_image));
    print("pc64: registered original /boot/rc, %lud bytes\n", (ulong)sizeof(p9_rc6_image));
#endif
#ifdef P9_HAVE_RCMAIN_DATA
    addbootfile("rcmain", p9_rcmain_data, sizeof(p9_rcmain_data)-1);
    print("pc64: registered original /boot/rcmain, %lud bytes\n", (ulong)(sizeof(p9_rcmain_data)-1));
#endif
}