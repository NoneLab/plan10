#ifndef P9_LIB9C_ENV_H
#define P9_LIB9C_ENV_H


#ifdef __cplusplus
extern "C" {
#endif

/* Native /env files, not lib10c environ. Returned p9Getenv string: p9Free. */
char *p9Getenv(const char *name);
int p9Putenv(const char *name, const char *value);
/* Static buffers; overwritten by subsequent calls, not thread-safe. */
const char *p9Getuser(void);
const char *p9Sysname(void);
int p9Getppid(void);

#ifdef __cplusplus
}
#endif

#endif
