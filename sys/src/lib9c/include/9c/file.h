#ifndef P9_LIB9C_FILE_H
#define P9_LIB9C_FILE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Native Plan 9 open/create modes; not POSIX O_* flags. */
enum {
    P9_OREAD   = 0,
    P9_OWRITE  = 1,
    P9_ORDWR   = 2,
    P9_OEXEC   = 3,
    P9_OTRUNC  = 16,
    P9_OCEXEC  = 32,
    P9_ORCLOSE = 64,
    P9_OEXCL   = 0x1000, /* create only */
};

/* Directory permission bit for p9Create(), not a POSIX mode_t bit. */
#define P9_DMDIR 0x80000000UL

int p9Open(const char *path, int mode);
int p9Create(const char *path, int mode, unsigned long perm);
int p9Remove(const char *path);

#define P9_DMAPPEND 0x40000000UL
#define P9_DMEXCL   0x20000000UL
#define P9_DMMOUNT  0x10000000UL
#define P9_DMAUTH   0x08000000UL
#define P9_DMTMP    0x04000000UL
#define P9_DMREAD   4
#define P9_DMWRITE  2
#define P9_DMEXEC   1

enum { P9_AEXIST = 0, P9_AEXEC = 1, P9_AWRITE = 2, P9_AREAD = 4 };
int p9Access(const char *path, int mode);
int p9Close(int fd);
int p9Dup(int oldfd, int newfd);
int p9Chdir(const char *path);
int p9Fd2path(int fd, char *buf, int len);
long p9Read(int fd, void *buf, long count);
long p9Write(int fd, const void *buf, long count);
long p9Pread(int fd, void *buf, long count, long long offset);
long p9Pwrite(int fd, const void *buf, long count, long long offset);
long long p9Seek(int fd, long long offset, int whence);
long p9Readn(int fd, void *buf, long count);

/* Native packed stat buffers, not POSIX struct stat. */
int p9Stat(const char *path, unsigned char *buf, int len);
int p9Fstat(int fd, unsigned char *buf, int len);
int p9Wstat(const char *path, const unsigned char *buf, int len);
int p9Fwstat(int fd, const unsigned char *buf, int len);
int p9Fauth(int fd, const char *aname);
int p9Fversion(int fd, int msize, char *version, int len);
int p9Iounit(int fd);
char *p9Getwd(char *buf, int len);

typedef struct P9IOchunk {
    void *addr;
    unsigned long len;
} P9IOchunk;

/* Coalesce into one native I/O operation as in the original libc. */
long p9Preadv(int fd, const P9IOchunk *io, int nio, long long offset);
long p9Pwritev(int fd, const P9IOchunk *io, int nio, long long offset);
long p9Readv(int fd, const P9IOchunk *io, int nio);
long p9Writev(int fd, const P9IOchunk *io, int nio);

#ifdef __cplusplus
}
#endif

#endif
