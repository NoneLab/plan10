#include <signal.h>
#include <errno.h>

/* Keep OpenBSD w_sigaction.c and w_sigprocmask.c as the public wrappers.
 * The native Note-to-POSIX-signal dispatcher is not implemented yet. */
__dso_hidden int
HIDDEN(sigaction)(int sig, const struct sigaction *act, struct sigaction *old)
{
    (void)sig; (void)act; (void)old;
    errno = ENOSYS;
    return -1;
}

__dso_hidden int
HIDDEN(sigprocmask)(int how, const sigset_t *set, sigset_t *old)
{
    (void)how; (void)set; (void)old;
    errno = ENOSYS;
    return -1;
}

__dso_hidden int
HIDDEN(thrkill)(pid_t tid, int sig, void *tcb)
{
    (void)tid; (void)sig; (void)tcb;
    errno = ENOSYS;
    return -1;
}
DEF_STRONG(thrkill);
