/*
 * AMD64 IDT vectors, register save/restore, and iretq.
 * The common handler constructs the layout defined by amd64/include/ureg.h.
 * Error-code vectors receive hardware-pushed error codes; the others get 0.
 * The saved Ureg is always 184 bytes and retains both CPL0/CPL3 stacks.
 */
    .text
    .altmacro

    .macro P9_VECTOR n
    .globl p9_amd64_vector_\n
    .type p9_amd64_vector_\n, @function
p9_amd64_vector_\n:
    .if (\n != 8) && (\n != 10) && (\n != 11) && (\n != 12) && (\n != 13) && (\n != 14) && (\n != 17) && (\n != 21) && (\n != 29) && (\n != 30)
    pushq $0
    .endif
    pushq $\n
    jmp p9_amd64_trap_entry
    .endm

    .set vec, 0
    .rept 256
    P9_VECTOR %vec
    .set vec, vec+1
    .endr

    .globl p9_amd64_trap_entry
    .type p9_amd64_trap_entry, @function
p9_amd64_trap_entry:
    cld
    subq $184, %rsp

    /* Offsets 0..112: the 15 GP registers. */
    movq %rax, 0(%rsp)
    movq %rbx, 8(%rsp)
    movq %rcx, 16(%rsp)
    movq %rdx, 24(%rsp)
    movq %rsi, 32(%rsp)
    movq %rdi, 40(%rsp)
    movq %rbp, 48(%rsp)
    movq %r8, 56(%rsp)
    movq %r9, 64(%rsp)
    movq %r10, 72(%rsp)
    movq %r11, 80(%rsp)
    movq %r12, 88(%rsp)
    movq %r13, 96(%rsp)
    movq %r14, 104(%rsp)
    movq %r15, 112(%rsp)

    movw %ds, 120(%rsp)
    movw %es, 122(%rsp)
    movw %fs, 124(%rsp)
    movw %gs, 126(%rsp)

    /* Software type/error pair, then hardware RIP/CS/RFLAGS. */
    movq 184(%rsp), %rax
    movq %rax, 128(%rsp)
    movq 192(%rsp), %rax
    movq %rax, 136(%rsp)
    movq 200(%rsp), %rax
    movq %rax, 144(%rsp)
    movq 208(%rsp), %rax
    movq %rax, 152(%rsp)
    movq 216(%rsp), %rax
    movq %rax, 160(%rsp)

    testb $3, 152(%rsp)
    jnz 1f
    /* CPL0: CPU didn't push a saved RSP/SS. */
    leaq 224(%rsp), %rax
    movq %rax, 168(%rsp)
    xorq %rax, %rax
    movw %ss, %ax
    movq %rax, 176(%rsp)
    jmp 2f
1:
    movq 224(%rsp), %rax
    movq %rax, 168(%rsp)
    movq 232(%rsp), %rax
    movq %rax, 176(%rsp)
2:
    movq %rsp, %r12
    movq %rsp, %rdi
    andq $-16, %rsp
    call p9_amd64_trap
    movq %r12, %rsp

    /* Restore all registers and discard software type/error. */
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
    movq 0(%rsp), %rax
    addq $200, %rsp
    iretq

    /* Native 10-byte GDTR/IDTR pointers, not the old 6-byte 386 ones. */
    .globl p9_amd64_lgdt
p9_amd64_lgdt:
    lgdt (%rdi)
    ret
    .globl p9_amd64_lidt
p9_amd64_lidt:
    lidt (%rdi)
    ret
    .globl p9_amd64_ltr
p9_amd64_ltr:
    ltr %di
    ret

    .section .rodata,"a",@progbits
    .p2align 3
    .globl p9_amd64_vectors
    .type p9_amd64_vectors, @object
p9_amd64_vectors:
    .macro P9_VECTOR_PTR n
    .quad p9_amd64_vector_\n
    .endm
    .set vec, 0
    .rept 256
    P9_VECTOR_PTR %vec
    .set vec, vec+1
    .endr
    .size p9_amd64_vectors, 2048

    .section .note.GNU-stack,"",@progbits
