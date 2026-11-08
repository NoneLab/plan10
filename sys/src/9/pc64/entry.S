/*
 * Real Plan 9/pc64 kernel entry for a Multiboot-v1 loader.
 *
 * This is NOT a second/bootstrap kernel: it is the machine entry for the
 * Plan 9 kernel linked from ../port, just as ../pc/l.s is for 386.
 * QEMU's Multiboot-v1 loader historically rejects an ELF64 header. The
 * very same linked native Plan 9 kernel is therefore exported as a flat
 * physical image (9pc64.mb) with Multiboot load-address fields. The ELF64
 * remains the debug symbol file; this is NOT a second kernel/bootloader.
 *
 * Entry requirements: EAX=0x2badb002, EBX=Multiboot info physical address;
 * 32-bit protected mode, paging disabled, interrupts disabled.
 *
 * Early address space: 0..1GiB mapped 1:1 and mapped again at KZERO
 * (0xffffffff80000000). The final Plan 9 MMU must replace these tables.
 */

    .section .multiboot, "a", @progbits
    .balign 4
    .globl p9_multiboot_header
p9_multiboot_header:
    .long 0x1badb002
    /* Address fields let QEMU load the same native kernel as a flat image. */
    .long 0x00010003                 /* aligned modules + memory map + addresses */
    .long -(0x1badb002 + 0x00010003)
    .long p9_multiboot_header       /* physical address of this header */
    .long __boot_start              /* lowest physical load address */
    .long __kernel_load_end         /* last physical byte stored in the file */
    .long __kernel_memory_end       /* includes zero-filled kernel BSS */
    .long p9_amd64_multiboot_entry  /* protected-mode 32-bit entry */

    .section .boot.text, "ax", @progbits
    .code32
    .globl p9_amd64_multiboot_entry
    .type p9_amd64_multiboot_entry, @function
p9_amd64_multiboot_entry:
    cli
    cld
    cmpl $0x2badb002, %eax
    jne .Lbad_boot
    movl %ebx, p9_mb_info_phys

    movl $p9_boot_stack_end, %esp
    lgdt p9_boot_gdtr

    movl %cr0, %eax
    andl $~0x0000000c, %eax
    orl $0x00000022, %eax
    movl %eax, %cr0

    movl %cr4, %eax
    orl $0x00000620, %eax
    movl %eax, %cr4

    /* Physical page tables and protected-mode PAE. */
    movl $p9_boot_pml4, %eax
    movl %eax, %cr3

    /* IA32_EFER.LME: enable 64-bit mode before setting CR0.PG. */
    movl $0xc0000080, %ecx
    rdmsr
    orl $0x100, %eax
    wrmsr

    movl %cr0, %eax
    orl $0x80000001, %eax
    movl %eax, %cr0

    ljmp $0x08, $p9_amd64_long_entry

.Lbad_boot:
    cli
.Lstop_32:
    hlt
    jmp .Lstop_32

    .code64
    .globl p9_amd64_long_entry
    .type p9_amd64_long_entry, @function
p9_amd64_long_entry:
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss
    xorw %ax, %ax
    movw %ax, %fs
    movw %ax, %gs

    /* Now use the kernel's higher-half stack, not boot-stage memory. */
    movabsq $p9_amd64_early_stack_end, %rsp
    andq $-16, %rsp
    xorq %rbp, %rbp
    movl p9_mb_info_phys(%rip), %edi
    movabsq $p9_amd64_kentry, %rax
    call *%rax

.Lstop_64:
    cli
    hlt
    jmp .Lstop_64

    .section .boot.rodata, "a", @progbits
    .balign 8
p9_boot_gdt:
    .quad 0
    .quad 0x00af9a000000ffff         /* ring 0, 64-bit code */
    .quad 0x00cf92000000ffff         /* ring 0, data */
p9_boot_gdt_end:
p9_boot_gdtr:
    .word p9_boot_gdt_end - p9_boot_gdt - 1
    .long p9_boot_gdt

    .section .boot.data, "aw", @progbits
    .balign 4
p9_mb_info_phys:
    .long 0

    .section .boot.paging, "aw", @progbits
    .balign 4096
    .globl p9_boot_pml4
p9_boot_pml4:
    .quad p9_boot_pdpt_low + 3       /* low identity map */
    .fill 510, 8, 0
    .quad p9_boot_pdpt_high + 3      /* PML4 slot 511: higher-half */

    .balign 4096
p9_boot_pdpt_low:
    .quad p9_boot_pd + 3
    .fill 511, 8, 0

    .balign 4096
p9_boot_pdpt_high:
    .fill 510, 8, 0
    .quad p9_boot_pd + 3            /* -2GiB window: PDPT slot 510 */
    .quad 0

    .balign 4096
p9_boot_pd:
    .set p9_pd_index, 0
    .rept 512
    .quad (p9_pd_index << 21) + 0x83 /* 2MiB present, writable, huge */
    .set p9_pd_index, p9_pd_index + 1
    .endr

    .section .boot.bss, "aw", @nobits
    .balign 16
    .skip 16384
p9_boot_stack_end:

    .section .bss.p9early, "aw", @nobits
    .balign 16
    .skip 32768
    .globl p9_amd64_early_stack_end
p9_amd64_early_stack_end:

    .section .note.GNU-stack, "", @progbits
