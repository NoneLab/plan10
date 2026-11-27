#ifndef P9_LIB9C_DIR_H
#define P9_LIB9C_DIR_H

#include <stddef.h>
#include <9c/file.h>

#ifdef __cplusplus
extern "C" {
#endif

#define P9_STATMAX 65535U
#define P9_STATFIXLEN 49U
#define P9_QIDSZ 13U
#define P9_QTDIR    0x80
#define P9_QTAPPEND 0x40
#define P9_QTEXCL   0x20
#define P9_QTMOUNT  0x10
#define P9_QTAUTH   0x08
#define P9_QTTMP    0x04
#define P9_QTFILE   0x00

/* In-memory representation; never transmit sizeof(P9Dir) or sizeof(P9Qid). */
typedef struct P9Qid {
    unsigned long long path;
    unsigned int vers;
    unsigned char type;
} P9Qid;

typedef struct P9Dir {
    unsigned short type;
    unsigned int dev;
    P9Qid qid;
    unsigned int mode, atime, mtime;
    long long length;
    char *name, *uid, *gid, *muid;
} P9Dir;

/* Codec shared by native stat helpers and future lib9pp. */
int p9Statcheck(const unsigned char *buf, size_t len);
size_t p9SizeD2M(const P9Dir *d);
size_t p9ConvD2M(const P9Dir *d, unsigned char *buf, size_t cap);
/* Decode exactly one stat; strings are copied into separate caller storage.
 * cap must cover all four NUL-terminated strings. Returns 0 on failure. */
size_t p9ConvM2D(const unsigned char *buf, size_t len, P9Dir *d,
                 char *strings, size_t cap);
void p9Nulldir(P9Dir *d);
/* Results are single allocations belonging to lib9c. */
P9Dir *p9Dirstat(const char *path);
P9Dir *p9Dirfstat(int fd);
int p9Dirwstat(const char *path, const P9Dir *d);
int p9Dirfwstat(int fd, const P9Dir *d);
long p9Dirread(int fd, P9Dir **out);
long p9Dirreadall(int fd, P9Dir **out);

#ifdef __cplusplus
}
#endif

#endif
