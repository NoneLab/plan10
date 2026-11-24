    .section .text._start,"ax",@progbits
    .global _start
    .type _start,@function
_start:
    movq (%rsp), %rdi
    leaq 8(%rsp), %rsi
    andq $-16, %rsp
    call main
    xorl %edi, %edi
    call exits

1:
    ud2
    jmp 1b
    .size _start, . - _start
    .section .note.GNU-stack,"",@progbits