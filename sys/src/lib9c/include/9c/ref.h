#ifndef P9_LIB9C_REF_H
#define P9_LIB9C_REF_H

#include <9c/sync.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize before publishing the object. A live reference is required
 * to acquire another reference; do not resurrect a released object. */
typedef struct P9Ref {
    long ref;
} P9Ref;

static inline void
p9Incref(P9Ref *r)
{
    p9Ainc(&r->ref);
}

/* Returns the remaining reference count. The caller owns final cleanup. */
static inline long
p9Decref(P9Ref *r)
{
    return p9Adec(&r->ref);
}

#ifdef __cplusplus
}
#endif
#endif
