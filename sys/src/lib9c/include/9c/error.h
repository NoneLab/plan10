#ifndef P9_LIB9C_ERROR_H
#define P9_LIB9C_ERROR_H


#ifdef __cplusplus
extern "C" {
#endif

#define P9_ERRMAX 128
/* Exchange the native error string with buf (not a POSIX strerror call). */
int p9Errstr(char *buf, unsigned int len);
/* Read without replacing the native error string. */
void p9Rerrstr(char *buf, unsigned int len);
/* Standard printf formatting, NOT Plan 9 %r / %q / %lud. */
void p9Werrstr(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void p9Sysfatal(const char *fmt, ...) __attribute__((noreturn, format(printf, 1, 2)));

#ifdef __cplusplus
}
#endif

#endif
