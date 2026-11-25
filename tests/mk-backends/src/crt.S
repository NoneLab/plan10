    .section .text._start,"ax",@progbits
    .globl _start
_start:
    jmp p9_test_user_entry
    .section .note.GNU-stack,"",@progbits
