/*
 * ATA-over-Ethernet (AoE) protocol
 */
enum {
	ACata,
	ACconfig,
};

enum {
	AQCread,
	AQCtest,
	AQCprefix,
	AQCset,
	AQCfset,
};

enum {
	AEcmd	= 1,
	AEarg,
	AEdev,
	AEcfg,
	AEver,
};

enum {
	Aoetype	= 0x88a2,
	Aoesectsz = 512,			/* standard sector size */
	Aoever	= 1,

	AFerr	= 1<<2,
	AFrsp	= 1<<3,

	AAFwrite= 1,
	AAFext	= 1<<6,
};

#if defined(__clang__) && defined(__x86_64__)
/* C11-compatible common AoE wire-header prefix. */
#define P9_AOEHDR_FIELDS \
uchar	dst[Eaddrlen]; \
uchar	src[Eaddrlen]; \
uchar	type[2]; \
uchar	verflag; \
uchar	error; \
uchar	major[2]; \
uchar	minor; \
uchar	cmd; \
uchar	tag[4];
#endif

typedef struct {
	uchar	dst[Eaddrlen];
	uchar	src[Eaddrlen];
	uchar	type[2];
	uchar	verflag;
	uchar	error;
	uchar	major[2];
	uchar	minor;
	uchar	cmd;
	uchar	tag[4];
	uchar	payload[];
} Aoehdr;

#define AOEHDRSZ	offsetof(Aoehdr, payload[0])

typedef struct {
#if defined(__clang__) && defined(__x86_64__)
    P9_AOEHDR_FIELDS
#else
    Aoehdr;
#endif
    uchar aflag;
	uchar	errfeat;
	uchar	scnt;
	uchar	cmdstat;
	uchar	lba[6];
	uchar	res[2];
	uchar	payload[];
} Aoeata;

#define AOEATASZ	offsetof(Aoeata, payload[0])

typedef struct {
#if defined(__clang__) && defined(__x86_64__)
    P9_AOEHDR_FIELDS
#else
    Aoehdr;
#endif
    uchar bufcnt[2];
	uchar	fwver[2];
	uchar	scnt;
	uchar	verccmd;
	uchar	cslen[2];
	uchar	payload[];
} Aoeqc;

#define AOEQCSZ		offsetof(Aoeqc, payload[0])
