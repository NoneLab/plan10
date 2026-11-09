ENTRY(_start)
SECTIONS
{
    . = 0x1020;
    .text : {
        KEEP(*(.text._start))
        *(.text .text.*)
    }
    .rodata : {
        *(.rodata .rodata.*)
    }
    .data : {
        *(.data .data.*)
    }
    .bss : {
        *(.bss .bss.* COMMON)
    }
    /DISCARD/ : {
        *(.eh_frame)
        *(.note*)
        *(.comment*)
    }
}
ASSERT(. <= 0x2000, "initcode exceeds the initial user page")