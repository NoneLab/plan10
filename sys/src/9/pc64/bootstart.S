    .section .text._start,"ax",@progbits
    .global _start
    .type _start,@function
_start:
    movq (%rsp), %rdi
    leaq 8(%rsp), %rsi
    movq (%rsi), %rax
    movq %rax, argv0(%rip)

    andq $-16, %rsp
    call main

    xorq %rdi, %rdi
    call exits
1:
    ud2
    jmp 1b
    .size _start, . - _start
    .section .note.GNU-stack,"",@progbits