OUTPUT_ARCH( "riscv" )
ENTRY( _boot )

SECTIONS
{
    /* 
    * qemu virt board's ram starts at 0x80000000
    * _boot function must be located at 0x80000000
    */
    . = 0x80000000;

    .text : {
        KEEP(*(.text.entry));
        . = ALIGN(0x1000);
        KEEP(*(.text.trap));
        *(.text .text.*)
        PROVIDE(etext = .);
    }

    .rodata : {
        . = ALIGN(0x1000);
        *(.srodata .srodata.*) 
        . = ALIGN(0x1000);
        *(.rodata .rodata.*)
    }

    .data : {
        . = ALIGN(8);
        *(.sdata .sdata.*)
        . = ALIGN(8);
        *(.data .data.*)
    }

    .bss : {
    . = ALIGN(8);
    *(.sbss .sbss.*)
    . = ALIGN(8);
    *(.bss .bss.*)
    }

    . = ALIGN(0x1000);
    stack_bot = .;
}