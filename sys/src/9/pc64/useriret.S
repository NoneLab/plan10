    .text
    .global p9_amd64_iretuser
    .type p9_amd64_iretuser, @function
p9_amd64_iretuser:
    cli
    pushq 176(%rdi)           /* SS */
    pushq 168(%rdi)           /* RSP */
    pushq 160(%rdi)           /* RFLAGS */
    pushq 152(%rdi)           /* CS */
    pushq 144(%rdi)           /* RIP */

    /* Restore the user register state without disturbing the iret frame. */
    movq 0(%rdi), %rax
    movq 8(%rdi), %rbx
    movq 16(%rdi), %rcx
    movq 24(%rdi), %rdx
    movq 48(%rdi), %rbp
    movq 56(%rdi), %r8
    movq 64(%rdi), %r9
    movq 72(%rdi), %r10
    movq 80(%rdi), %r11
    movq 88(%rdi), %r12
    movq 96(%rdi), %r13
    movq 104(%rdi), %r14
    movq 112(%rdi), %r15
    movq 32(%rdi), %rsi
    movq 40(%rdi), %rdi
    iretq
    .size p9_amd64_iretuser, . - p9_amd64_iretuser
    .section .note.GNU-stack,"",@progbits