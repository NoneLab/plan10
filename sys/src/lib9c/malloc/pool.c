/* Build the existing allocator algorithm with lib9c's private C ABI.
 * The kernel and libc-plan9 still compile their original translation unit. */
#define P9_LIB9C_POOL 1
#include "../../libc/port/pool.c"
