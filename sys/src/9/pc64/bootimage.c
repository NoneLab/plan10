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
}