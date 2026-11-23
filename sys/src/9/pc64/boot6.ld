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
    . = ALIGN(4096);

    .data : {
        *(.data .data.*)
    } :data

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
