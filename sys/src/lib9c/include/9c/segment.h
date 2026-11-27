#ifndef P9_LIB9C_SEGMENT_H
#define P9_LIB9C_SEGMENT_H


#ifdef __cplusplus
extern "C" {
#endif

enum { P9_SG_RONLY = 0040, P9_SG_CEXEC = 0100 };
/* Native segment operations, not POSIX mmap/munmap. */
void *p9Segattach(int attr, const char *name, void *addr, unsigned long len);
/* Native SEGBRK result: zero for resize, base for a zero-address query,
 * (void *)-1 on error. Do not resize the allocator's BSS segment. */
void *p9Segbrk(void *addr, void *newtop);
int p9Segdetach(void *addr);
int p9Segfree(void *addr, unsigned long len);
int p9Segflush(void *addr, unsigned long len);

#ifdef __cplusplus
}
#endif

#endif
