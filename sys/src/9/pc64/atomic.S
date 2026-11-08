/*
 * Plan 9/pc64 atomic and low-level accounting primitives.
 * Every operation obeys SysV AMD64 (long is 64 bits), not the 386 ABI.
 * The original _xdec contract returns -1, 0 or +1, NOT the refcount.
 */
    .text

    .globl _xinc
    .type _xinc, @function
_xinc:
    lock incq (%rdi)
    ret

    .globl _xdec
    .type _xdec, @function
_xdec:
    xorq %rax, %rax
    xorq %rdx, %rdx
    lock decq (%rdi)
    setg %al
    sets %dl
    subq %rdx, %rax
    ret

    .globl cmpswap486
    .type cmpswap486, @function
cmpswap486:
    movq %rsi, %rax
    lock cmpxchgq %rdx, (%rdi)
    sete %al
    movzbl %al, %eax
    ret

    .globl islo
    .type islo, @function
islo:
    pushfq
    popq %rax
    andq $0x200, %rax
    ret

    /* Uses the caller's frame pointer (all kernel C units build with
     * -fno-omit-frame-pointer). %rbp points to the invoking function's
     * frame, and 8(%rbp) is the address at which that function returns. */
    .globl getcallerpc
    .type getcallerpc, @function
getcallerpc:
    movq 8(%rbp), %rax
    ret

    .globl p9_amd64_rdtsc
    .type p9_amd64_rdtsc, @function
p9_amd64_rdtsc:
    lfence
    rdtsc
    shlq $32, %rdx
    orq %rdx, %rax
    movq %rax, (%rdi)
    ret

    .section .note.GNU-stack,"",@progbits
