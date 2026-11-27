#include "thread_private.h"

/*
 * Bootstrap thread callback table.
 *
 * Thread callbacks remain disabled until the native
 * Plan10 thread runtime is initialized.
 */
__dso_hidden struct thread_callbacks _thread_cb = { 0 };