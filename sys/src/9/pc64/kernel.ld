/*
 * Native Plan 9 AMD64 kernel layout for the Multiboot-v1 BIOS loader.
 * Low memory .boot: CPU transition only, never a replacement kernel.
 * Higher half: genuine Plan 9 /pc64 and /port kernel object files.
 */
OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(p9_amd64_multiboot_entry)

KERNEL_BASE = 0xffffffff80000000;

PHDRS
{
    boot PT_LOAD FLAGS(7);
    text PT_LOAD FLAGS(5);
    data PT_LOAD FLAGS(6);
}

SECTIONS
{
    . = 0x00100000;
    __boot_start = .;

    .multiboot ALIGN(4) :
    {
        KEEP(*(.multiboot))
    } :boot

    .boot.text ALIGN(16) :
    {
        *(.boot.text)
    } :boot

    .boot.rodata ALIGN(16) :
    {
        *(.boot.rodata)
    } :boot

    .boot.data ALIGN(16) :
    {
        *(.boot.data)
    } :boot

    .boot.paging ALIGN(4096) :
    {
        *(.boot.paging)
    } :boot

    .boot.bss (NOLOAD) :
    {
        *(.boot.bss)
    } :boot

    __boot_end = .;
    ASSERT(__boot_end < 0x00200000, "early boot data overlaps kernel load address")

    . = KERNEL_BASE + 0x00200000;
    __kernel_start = .;

    .text ALIGN(4096) : AT(ADDR(.text) - KERNEL_BASE)
    {
        *(.text .text.*)
    } :text

    PROVIDE(etext = .);

    .rodata ALIGN(4096) : AT(ADDR(.rodata) - KERNEL_BASE)
    {
        *(.rodata .rodata.*)
    } :text

    .data ALIGN(4096) : AT(ADDR(.data) - KERNEL_BASE)
    {
        *(.data .data.*)
    } :data

    .bss ALIGN(4096) (NOLOAD) : AT(ADDR(.bss) - KERNEL_BASE)
    {
        *(.bss .bss.* COMMON)
    } :data

    /* Physical addresses consumed by Multiboot-v1's raw-image loader. */
    __kernel_load_end = LOADADDR(.data) + SIZEOF(.data);
    __kernel_memory_end = LOADADDR(.bss) + SIZEOF(.bss);

    __kernel_end = .;
    PROVIDE(end = .);
    ASSERT(__kernel_end - KERNEL_BASE < 0x40000000, "early PML4 window exceeded")

    /DISCARD/ :
    {
        *(.eh_frame .eh_frame.*)
        *(.comment)
        *(.note.GNU-stack)
        *(.llvm_addrsig)
    }
}
