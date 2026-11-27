    .section .text._start,"ax",@progbits
    .global _start
    .type _start,@function

_start:
    xorq    %rbp, %rbp

    /* Preserve argc and argv. */
    movq    (%rsp), %r12
    leaq    8(%rsp), %r13

    /* SysV AMD64 ABI stack alignment. */
    andq    $-16, %rsp

    /* Initialize libc runtime. */
    movq    %r12, %rdi
    movq    %r13, %rsi
    call    __lib10c_init

    /* Invoke main(argc, argv). */
    movq    %r12, %rdi
    movq    %r13, %rsi
    call    main

    call    __lib10c_fini

    /* Temporary bootstrap termination. */
    movl    %eax, %edi
    call    _exit

    ud2

    .size _start, . - _start
    .section .note.GNU-stack,"",@progbits