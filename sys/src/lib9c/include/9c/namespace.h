#ifndef P9_LIB9C_NAMESPACE_H
#define P9_LIB9C_NAMESPACE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Native mount/bind flags; MREPL is zero, not an independent bit. */
enum {
    P9_MREPL   = 0x0000,
    P9_MBEFORE = 0x0001,
    P9_MAFTER  = 0x0002,
    P9_MCREATE = 0x0004,
    P9_MCACHE  = 0x0010,
};

/* Mount returns a mount ID, or -1 on failure.  The kernel consumes fd
 * on success; on failure it remains the caller's responsibility.
 * afd = -1 means no authentication channel; spec must be a string. */
int p9Mount(int fd, int afd, const char *path, int flags, const char *spec);
int p9Bind(const char *source, const char *target, int flags);

/* source == NULL removes all mounts at target. */
int p9Unmount(const char *source, const char *target);

#ifdef __cplusplus
}
#endif

#endif
