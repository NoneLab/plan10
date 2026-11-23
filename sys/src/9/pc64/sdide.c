/*
 * Plan 9/pc64: QEMU PC-compatible primary IDE controller.
 *
 * The machine-independent #S implementation remains ../port/devsd.c.
 * This SDifc implements ATA PIO (polling, LBA28, primary master only).
 * No DMA, PCI enumeration, interrupt controller dependency or invented
 * filesystem layer is used.  The disk itself is a real QEMU IDE drive.
 */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "../port/sd.h"

enum {
    Idebase         = 0x1f0,
    Idectl          = 0x3f6,
    IdeData         = 0,
    IdeCount        = 2,
    IdeLba0         = 3,
    IdeLba1         = 4,
    IdeLba2         = 5,
    IdeDrive        = 6,
    IdeStatus       = 7,
    IdeCommand      = 7,
    IdeErr          = 0x01,
    IdeDrq          = 0x08,
    IdeFault        = 0x20,
    IdeBusy         = 0x80,
    IdeIdentify     = 0xec,
    IdeRead         = 0x20,
    IdeWrite        = 0x30,
    IdeFlush        = 0xe7,
    IdeSectorSize   = 512,
    IdeLba28Limit   = 0x10000000,
    IdePollLimit    = 1000000,
};

typedef struct Pc64ide Pc64ide;
struct Pc64ide {
    QLock io;
    uvlong sectors;
    char model[41];
    int present;
};
static Pc64ide ide;
static SDifc ideifc;

static int
idewait(int data)
{
    int i, s;

    for(i = 0; i < IdePollLimit; i++){
        s = inb(Idectl);
        if(s == 0 || s == 0xff)
            return -1;
        if((s & IdeBusy) != 0)
            continue;
        if((s & (IdeErr | IdeFault)) != 0)
            return -1;
        if(!data || (s & IdeDrq) != 0)
            return 0;
    }
    return -1;
}

/* ATA requires a >=400ns settling delay after drive selection. */
static void
idesettle(void)
{
    int i;

    for(i = 0; i < 4; i++)
        (void)inb(Idectl);
}

static int
ideidentify(void)
{
    ushort words[256];
    ulong n;
    int i, j;

    /* Disable IDE IRQ delivery: this driver uses polling exclusively. */
    outb(Idectl, 2);
    outb(Idebase + IdeDrive, 0xe0);  /* primary master, LBA */
    idesettle();
    outb(Idebase + IdeCount, 0);
    outb(Idebase + IdeLba0, 0);
    outb(Idebase + IdeLba1, 0);
    outb(Idebase + IdeLba2, 0);
    outb(Idebase + IdeCommand, IdeIdentify);
    if(idewait(1) < 0)
        return -1;
    for(i = 0; i < 256; i++)
        words[i] = ins(Idebase + IdeData);
    if((words[49] & (1 << 9)) == 0)    /* LBA capability */
        return -1;
    n = (ulong)words[60] | ((ulong)words[61] << 16);
    if(n == 0)
        return -1;
    /* Do not advertise sectors inaccessible through 28-bit LBA. */
    if(n > IdeLba28Limit)
        n = IdeLba28Limit;
    ide.sectors = n;
    for(i = 0, j = 0; i < 20; i++){
        ide.model[j++] = words[27 + i] >> 8;
        ide.model[j++] = words[27 + i] & 0xff;
    }
    ide.model[40] = 0;
    ide.present = 1;
    return 0;
}

static SDev*
idepnp(void)
{
    SDev *sdev;

    if(ideidentify() < 0){
        print("pc64: #S primary IDE master not present\n");
        return nil;
    }
    sdev = malloc(sizeof(*sdev));
    if(sdev == nil)
        return nil;
    sdev->ifc = &ideifc;
    sdev->idno = 'C';              /* original PC's sdC0 convention */
    sdev->nunit = 1;
    print("pc64: #S ATA disk %s, %llud sectors (%llud MiB)\n",
        ide.model, ide.sectors, ide.sectors / 2048);
    return sdev;
}

static int
ideverify(SDunit *unit)
{
    if(!ide.present || unit->subno != 0)
        return 0;
    unit->inquiry[0] = SDperdisk;
    memmove(unit->inquiry + 8, "Plan9 IDE PIO", 13);
    return 1;
}

static int
ideonline(SDunit *unit)
{
    if(!ide.present)
        return 0;
    unit->sectors = ide.sectors;
    unit->secsize = IdeSectorSize;
    return 1;
}

/* One LBA28 sector per command, serialized across #S callers. */
static int
idetransfer(int write, uvlong lba, uchar *buf)
{
    int i, value;

    if(lba >= ide.sectors || lba >= IdeLba28Limit)
        return -1;
    if(idewait(0) < 0)
        return -1;
    outb(Idebase + IdeDrive, 0xe0 | ((lba >> 24) & 0x0f));
    idesettle();
    outb(Idebase + IdeCount, 1);
    outb(Idebase + IdeLba0, lba & 0xff);
    outb(Idebase + IdeLba1, (lba >> 8) & 0xff);
    outb(Idebase + IdeLba2, (lba >> 16) & 0xff);
    outb(Idebase + IdeCommand, write ? IdeWrite : IdeRead);
    if(idewait(1) < 0)
        return -1;

    for(i = 0; i < IdeSectorSize; i += 2){
        if(write){
            value = buf[i] | (buf[i+1] << 8);
            outs(Idebase + IdeData, value);
        }else{
            value = ins(Idebase + IdeData);
            buf[i] = value;
            buf[i+1] = value >> 8;
        }
    }
    return idewait(0);
}

static long
idebio(SDunit *unit, int lun, int write, void *data, long nb, uvlong bno)
{
    uchar *p;
    long i;
    int ok;

    if(lun != 0 || nb < 0 || (uvlong)nb > ide.sectors ||
       bno > ide.sectors - (uvlong)nb || unit->secsize != IdeSectorSize)
        return -1;
    p = data;
    qlock(&ide.io);
    ok = 1;
    for(i = 0; i < nb; i++){
        if(idetransfer(write, bno + i, p + i * IdeSectorSize) < 0){
            ok = 0;
            break;
        }
    }
    if(ok && write && nb > 0){
        outb(Idebase + IdeCommand, IdeFlush);
        if(idewait(0) < 0)
            ok = 0;
    }
    qunlock(&ide.io);
    return ok ? nb * IdeSectorSize : -1;
}

/* Raw SCSI pass-through is not implemented for ATA PIO. Avoid NULL callbacks. */
static int
iderio(SDreq *r)
{
    USED(r);
    return SDnostatus;
}

static SDifc ideifc = {
    .name = "pc64ide",
    .pnp = idepnp,
    .verify = ideverify,
    .online = ideonline,
    .rio = iderio,
    .bio = idebio,
};

SDifc *sdifc[] = { &ideifc, nil };

/* Called by the current pc64 getconf(), before plan9.ini support. */
char*
p9_amd64_idepartconf(void)
{
    static char part[64];

    if(!ide.present)
        return nil;
    /* Dedicated raw KFS disk: the whole device is partition "fs". */
    snprint(part, sizeof part, "fs 0 %llud", ide.sectors);
    return part;
}