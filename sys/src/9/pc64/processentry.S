/* Return a forked user process from the saved AMD64 Ureg to CPL3. */
    .text
    .globl p9_amd64_forkret
    .type p9_amd64_forkret, @function
p9_amd64_forkret:
    /* rsp points to the 184-byte Ureg saved by forkchild(). */
    movq 0(%rsp), %rax
    movq 8(%rsp), %rbx
    movq 16(%rsp), %rcx
    movq 24(%rsp), %rdx
    movq 32(%rsp), %rsi
    movq 40(%rsp), %rdi
    movq 48(%rsp), %rbp
    movq 56(%rsp), %r8
    movq 64(%rsp), %r9
    movq 72(%rsp), %r10
    movq 80(%rsp), %r11
    movq 88(%rsp), %r12
    movq 96(%rsp), %r13
    movq 104(%rsp), %r14
    movq 112(%rsp), %r15
    /* Instruction pointer, CS, RFLAGS, RSP, SS are a contiguous frame. */
    addq $144, %rsp
    iretq
    .section .note.GNU-stack, "", @progbits
