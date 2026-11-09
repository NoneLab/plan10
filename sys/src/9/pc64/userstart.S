    .section .text._start,"ax",@progbits
    .global _start
    .type _start, @function
_start:
    movq 8(%rsp), %rdi
    leaq 8(%rsp), %rsi
    andq $-16, %rsp
    call startboot
1:  ud2
    jmp 1b
    .size _start, . - _start
    .section .note.GNU-stack,"",@progbits