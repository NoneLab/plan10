    .text
    .global p9_syscall5
    .type p9_syscall5,@function
p9_syscall5:
    subq $48, %rsp
    movq %rsi, 8(%rsp)
    movq %rdx, 16(%rsp)
    movq %rcx, 24(%rsp)
    movq %r8, 32(%rsp)
    movq %r9, 40(%rsp)
    movq %rdi, %rax
    int $0x40
    addq $48, %rsp
    ret
    .size p9_syscall5, . - p9_syscall5
    .section .note.GNU-stack,"",@progbits