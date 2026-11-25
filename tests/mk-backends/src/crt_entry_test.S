/* Run the production crt0.S as an ordinary x86_64 static Linux ELF test.
 * Only exits() uses Linux's exit syscall; the tested startup itself is
 * unchanged, freestanding Plan 9 assembly.
 */
#ifdef TEST_STRONG_ARGV0
    .section .rodata
.Lrcname:
    .asciz "rc"
    .section .data
    .globl argv0
    .balign 8
argv0:
    .quad .Lrcname
#elif defined(TEST_STRONG_NULL_ARGV0)
    .section .bss
    .globl argv0
    .balign 8
argv0:
    .quad 0
#endif

    .section .text
    .globl main
main:
    cmpq $2, %rdi                 /* program name + one argument */
    jne .Lfail
    movq (%rsi), %rax             /* argv[0] */
    testq %rax, %rax
    jz .Lfail
    movq 8(%rsi), %rax            /* argv[1] */
    testq %rax, %rax
    jz .Lfail

    movq argv0(%rip), %rax
#ifdef TEST_STRONG_ARGV0
    leaq .Lrcname(%rip), %rdx
#else
    movq (%rsi), %rdx
#endif
    cmpq %rdx, %rax               /* weak initialized or strong preserved */
    jne .Lfail
    ret

.Lfail:
    movl $60, %eax                /* Linux exit(97), test failure */
    movl $97, %edi
    syscall
    ud2

    .globl exits
exits:
    testq %rdi, %rdi             /* Plan 9 exits(nil) */
    jne .Lfail
    movl $60, %eax                /* Linux exit(0), test success */
    xorl %edi, %edi
    syscall
    ud2

    .section .note.GNU-stack,"",@progbits
