OUTPUT_ARCH( "riscv" )
ENTRY( _boot )

SECTIONS
{
    /* 
    * qemu start at 0x80000000
    * _boot function must be located at 0x80000000
    */
    . = 0x80000000;

    .text : {
        *(.text .text.*)
        . = ALIGN(0x1000);  /* 4kB page align */
    }

    .rodata : {
        . = ALIGN(16);
        *(.srodata .srodata.*) 
        . = ALIGN(16);
        *(.rodata .rodata.*)
    }

    .data : {
        . = ALIGN(16);
        *(.sdata .sdata.*)
        . = ALIGN(16);
        *(.data .data.*)
    }

    .bss : {
    . = ALIGN(16);
    *(.sbss .sbss.*)
    . = ALIGN(16);
    *(.bss .bss.*)
    }
}