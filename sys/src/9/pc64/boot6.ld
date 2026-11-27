/* One native Plan 9 boot executable, convertible to S_MAGIC 6.out.
 * The file header is mapped at UTZERO=0x1000, followed by text at +40.
 * The original sysexec() aligns data to the next 4KB page boundary. */
OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)
PHDRS {
    text PT_LOAD FLAGS(5);
    data PT_LOAD FLAGS(6);
}
SECTIONS {
    . = 0x1028;
    
    .text 0x1028 : SUBALIGN(1) {
        KEEP(*(.text._start))
        *(.text .text.*)
        *(.rodata .rodata.*)
    } : text

    .init ALIGN(16) : {
        KEEP(*(.preinit))
        KEEP(*(.init))
    } : text

    .fini ALIGN(16) : {
        KEEP(*(.fini))
    } : text

    . = ALIGN(4096);

    .data : {
        *(.data .data.*)
    } : data

    .preinit_array ALIGN(8) : {
        HIDDEN(__preinit_array_start = .);
        KEEP(*(SORT_BY_INIT_PRIORITY(.preinit_array.*)))
        KEEP(*(.preinit_array))
        HIDDEN(__preinit_array_end = .);
    } : data

    .init_array ALIGN(8) : {
        HIDDEN(__init_array_start = .);
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)))
        KEEP(*(.init_array))
        HIDDEN(__init_array_end = .);
    } : data

    .fini_array ALIGN(8) : {
        HIDDEN(__fini_array_start = .);
        KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*)))
        KEEP(*(.fini_array))
        HIDDEN(__fini_array_end = .);
    } : data

    .bss (NOLOAD) : {
        *(.bss .bss.* COMMON)
        end = .;
    } : data

    /DISCARD/ : {
        *(.eh_frame .eh_frame.*)
        *(.comment .note.*)
        *(.llvm_addrsig)
    }
}
