#ifndef P9_LIB9C_PROCESS_H
#define P9_LIB9C_PROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Native rfork flags, from sys/include/libc.h. */
enum {
    P9_RFNAMEG  = 1 << 0,
    P9_RFENVG   = 1 << 1,
    P9_RFFDG    = 1 << 2,
    P9_RFNOTEG  = 1 << 3,
    P9_RFPROC   = 1 << 4,
    P9_RFMEM    = 1 << 5,
    P9_RFNOWAIT = 1 << 6,
    P9_RFCNAMEG = 1 << 10,
    P9_RFCENVG  = 1 << 11,
    P9_RFCFDG   = 1 << 12,
    P9_RFREND   = 1 << 13,
    P9_RFNOMNT  = 1 << 14,
};

/* With RFPROC: child returns 0, parent returns child PID, failure -1.
 * Without RFPROC: change the calling process's groups and return 0. */
int p9Rfork(int flags);

/* Native string-status termination; no stdio flush or C exit handlers. */
void p9Exits(const char *status) __attribute__((noreturn));

int p9Fork(void);
int p9Exec(const char *path, char *const argv[]);
/* Terminate the argument list with (char *)NULL. */
int p9Execl(const char *path, const char *arg0, ...);
int p9Getpid(void);
int p9Await(char *buf, int len);
int p9Sleep(long milliseconds);
long p9Alarm(unsigned long milliseconds);
long long p9Nsec(void);

/* Plan 9 native notes, not POSIX signals. */
enum { P9_NCONT = 0, P9_NDFLT, P9_NSAVE, P9_NRSTR };
enum { P9_PNPROC = 1, P9_PNGROUP = 2 };
int p9Notify(void (*handler)(void *, char *));
int p9Noted(int action);
int p9Postnote(int group, int pid, const char *note);

typedef struct P9Waitmsg {
    int pid;
    unsigned long time[3];
    char *msg;
} P9Waitmsg;
/* Wait returns one allocation; release with p9Free. Waitpid waits for any child. */
P9Waitmsg *p9Wait(void);
int p9Waitpid(void);

#ifdef __cplusplus
}
#endif

#endif
