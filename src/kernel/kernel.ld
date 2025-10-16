OUTPUT_ARCH( "riscv" )
ENTRY( _entry )

SECTIONS
{
    /* 
    * qemu virt board's ram starts at 0x80000000
    * _boot function must be located at 0x80000000
    */
    . = 0x80000000;
    .boot.text : {
        KEEP(*(.entry.text))
        *(.boot.text)
        . = ALIGN(8);
    }

    .boot.rodata : {
        *(.boot.rodata)
        . = ALIGN(8);
    }

    .boot.data : {
        *(.boot.data)
        . = ALIGN(8);
    }

    .boot.bss (NOLOAD) : {
        *(.boot.bss)
    }
    . = ALIGN(0x1000);
    PROVIDE(eboot = .);
    PROVIDE(etext = LOADADDR(.rodata));
    PROVIDE(erodata = LOADADDR(.data));
    PROVIDE(ebss = erodata + SIZEOF(.data) + SIZEOF(.bss));
    PROVIDE(_stack_bot = ebss);

    KERNEL_VMA = 0xffff800000000000;
    KERNEL_LMA = .;

    . = KERNEL_VMA;
    .text : AT(KERNEL_LMA + SIZEOF(.info)){
        KEEP(*(.text.main))
        *(.text .text.*)
        . = ALIGN(0x1000);
    }

    .rodata : AT(KERNEL_LMA + SIZEOF(.text) + SIZEOF(.info)){
        *(.srodata .srodata.*) 
        *(.rodata .rodata.*)
        . = ALIGN(0x1000);
    }

    .data : AT(KERNEL_LMA + SIZEOF(.info) + SIZEOF(.text) + SIZEOF(.rodata)) {
        *(.sdata .sdata.*)
        *(.data .data.*)
        . = ALIGN(0x1000);
    }

    .bss (NOLOAD) : AT(KERNEL_LMA + SIZEOF(.inf0) + SIZEOF(.text) + SIZEOF(.rodata) + SIZEOF(.data)) {
        *(.sbss .sbss.*)
        *(.bss .bss.*)
        . = ALIGN(0x1000);
    }

    /DISCARD/ : {
        *(.eh_frame)
        *(.eh_frame_hdr)
        *(.gcc_except_table)
    }
}