#ifndef P9_LIB9C_MALLOC_H
#define P9_LIB9C_MALLOC_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* All returned storage belongs to lib9c, not to the standard C allocator. */
void *p9Malloc(size_t size);
void *p9Mallocz(size_t size, int clear);
void *p9Calloc(size_t count, size_t size);
void *p9Realloc(void *ptr, size_t size);
void  p9Free(void *ptr);
size_t p9Msize(void *ptr);
char *p9Strdup(const char *str);

#ifdef __cplusplus
}
#endif

#endif
