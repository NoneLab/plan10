/*
 * AMD64 replacements for the 386 register, I/O and Label primitives in
 * ../pc/l.s.  Clang's integrated assembler uses standard GAS syntax.
 * SysV AMD64 calling convention; no Microsoft ABI extensions.
 *
 * These are kernel-mode primitives: do not execute privileged functions
 * from the host-based tests.  All code is part of the real Plan 9 port.
 */
    .text

    .macro P9_FUNC name
    .globl \name
    .type \name, @function
\name:
    .endm

P9_FUNC getcr0
    movq %cr0, %rax
    ret
P9_FUNC getcr2
    movq %cr2, %rax
    ret
P9_FUNC getcr3
    movq %cr3, %rax
    ret
P9_FUNC getcr4
    movq %cr4, %rax
    ret
P9_FUNC putcr0
    movq %rdi, %cr0
    ret
P9_FUNC putcr3
    movq %rdi, %cr3
    ret
P9_FUNC putcr4
    movq %rdi, %cr4
    ret
P9_FUNC invlpg
    invlpg (%rdi)
    ret
P9_FUNC wbinvd
    wbinvd
    ret
P9_FUNC mfence
    mfence
    ret

/* Save/restore interrupt-enable state, matching the original spl API. */
P9_FUNC splhi
    pushfq
    popq %rax
    cli
    andl $0x200, %eax
    ret
P9_FUNC spllo
    pushfq
    popq %rax
    sti
    andl $0x200, %eax
    ret
P9_FUNC splx
    testl $0x200, %edi
    jz 1f
    sti
    ret
1:
    cli
    ret

P9_FUNC spldone
    ret

P9_FUNC inb
    movl %edi, %edx
    xorl %eax, %eax
    inb %dx, %al
    ret
P9_FUNC ins
    movl %edi, %edx
    xorl %eax, %eax
    inw %dx, %ax
    ret
P9_FUNC inl
    movl %edi, %edx
    inl %dx, %eax
    ret
P9_FUNC outb
    movl %edi, %edx
    movl %esi, %eax
    outb %al, %dx
    ret
P9_FUNC outs
    movl %edi, %edx
    movl %esi, %eax
    outw %ax, %dx
    ret
P9_FUNC outl
    movl %edi, %edx
    movl %esi, %eax
    outl %eax, %dx
    ret

/* cpuid(int leaf, ulong regs[4]): all slots are native 64-bit. */
P9_FUNC cpuid
    pushq %rbx
    movl %edi, %eax
    xorl %ecx, %ecx
    cpuid
    movq %rax, 0(%rsi)
    movq %rbx, 8(%rsi)
    movq %rcx, 16(%rsi)
    movq %rdx, 24(%rsi)
    popq %rbx
    ret

/* tas(void*): the original Plan 9 atomic test-and-set primitive. */
P9_FUNC tas
    movl $1, %eax
    xchgl %eax, (%rdi)
    ret

/*
 * Label is an AMD64 SysV coroutine context, not a 386 two-word frame.
 * Fields: sp, pc, bx, bp, r12, r13, r14, r15 (8 bytes each).
 * setlabel must be declared returns_twice and gotolabel noreturn.
 */
P9_FUNC setlabel
    leaq 8(%rsp), %rax
    movq %rax, 0(%rdi)
    movq (%rsp), %rax
    movq %rax, 8(%rdi)
    movq %rbx, 16(%rdi)
    movq %rbp, 24(%rdi)
    movq %r12, 32(%rdi)
    movq %r13, 40(%rdi)
    movq %r14, 48(%rdi)
    movq %r15, 56(%rdi)
    xorl %eax, %eax
    ret

P9_FUNC gotolabel
    movq 0(%rdi), %rsp
    movq 8(%rdi), %rdx
    movq 16(%rdi), %rbx
    movq 24(%rdi), %rbp
    movq 32(%rdi), %r12
    movq 40(%rdi), %r13
    movq 48(%rdi), %r14
    movq 56(%rdi), %r15
    /* A nonzero return from the suspended setlabel. */
    movl $1, %eax
    jmp *%rdx

/*
 * lgdt changes the descriptor table but does not reload CS.  The early
 * Multiboot selector 0x08 becomes the data selector in the Plan 9 GDT;
 * reload CS with a 64-bit far return before enabling any interrupts.
 * GDT slots: data=1 (0x08), 64-bit code=2 (0x10).
 */
P9_FUNC p9_amd64_reload_segments
    leaq .Lafter_cs_reload(%rip), %rax
    pushq $0x10
    pushq %rax
    lretq
.Lafter_cs_reload:
    movw $0x08, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss
    ret

    .section .note.GNU-stack,"",@progbits
