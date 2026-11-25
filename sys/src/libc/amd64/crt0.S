/* Common native Plan 9/amd64 userspace entry point.
 *
 * Kernel exec supplies argc followed by argv on the initial user stack.
 * No TLS, libc runtime initialization, constructors or signal setup yet.
 *
 * Plan 9 programs conventionally expose argv0.  boot supplies it in
 * bootconf.c and rc supplies an initialized value in exec.c; a weak fallback
 * keeps standalone commands (including KFS and init) linkable.  Only fill
 * argv0 when it is null, preserving rc's existing initial value.
 */
    .section .text._start,"ax",@progbits
    .global _start
    .type _start,@function
_start:
    movq (%rsp), %rdi            /* argc */
    leaq 8(%rsp), %rsi           /* argv */

    cmpq $0, argv0(%rip)
    jne 1f
    movq (%rsi), %rax
    movq %rax, argv0(%rip)
1:
    andq $-16, %rsp             /* AMD64 call-site alignment */
    call main

    xorl %edi, %edi             /* Plan 9 exits(nil) */
    call exits
2:
    ud2
    jmp 2b
    .size _start, . - _start

    /* An executable's strong argv0 definition overrides this fallback. */
    .section .bss,"aw",@nobits
    .balign 8
    .weak argv0
    .type argv0,@object
argv0:
    .quad 0
    .size argv0, 8

    .section .note.GNU-stack,"",@progbits
