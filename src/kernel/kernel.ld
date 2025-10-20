OUTPUT_ARCH( "riscv" )
ENTRY( _entry )

SECTIONS
{
    /* 
    * qemu virt board's ram starts at 0x80000000
    * _boot function must be located at 0x80000000
    */
    RAM_START = 0x80000000;
    . = RAM_START;
    .boot.text : {
        KEEP(*(.entry))
        *(.boot.text)
        . = ALIGN(8);
    }
    PROVIDE(_main_paddr = .);
    . += SIZEOF(.text);
    PROVIDE(etext = .);

    .boot.rodata : {
        KEEP(*(.boot.info))
        *(.boot.rodata)
        . = ALIGN(8);
    }
    . += SIZEOF(.rodata);
    PROVIDE(erodata = .);

    .boot.data : {
        *(.boot.data)
        . = ALIGN(8);
    }

    . += SIZEOF(.data);
    .boot.bss (NOLOAD) : {
        *(.boot.bss)
    }

    . += SIZEOF(.bss);
    PROVIDE(_stack_bot = .);

    KERNEL_VMA = 0xffff800080000000;
    KERNEL_LMA = RAM_START;

    . = KERNEL_VMA;
    . += SIZEOF(.boot.text);
    .text : AT(KERNEL_LMA + SIZEOF(.boot.text)) {
        KEEP(*(.text.main))
        *(.text .text.*)
        . = ALIGN(4);
        KEEP(*(.text.trap))
        . = ALIGN(0x1000);
    }

    PROVIDE(_boot_info = .);
    . += SIZEOF(.boot.rodata);
    .rodata : AT(LOADADDR(.text) + SIZEOF(.text) + SIZEOF(.boot.rodata)) {
        *(.srodata .srodata.*) 
        *(.rodata .rodata.*)
        . = ALIGN(0x1000);
    }

    . += SIZEOF(.boot.data);
    .data : AT(LOADADDR(.rodata) + SIZEOF(.rodata) + SIZEOF(.boot.data)) {
        *(.sdata .sdata.*)
        *(.data .data.*)
        . = ALIGN(0x1000);
    }

    . += SIZEOF(.boot.bss);
    .bss (NOLOAD) : AT(LOADADDR(.data) + SIZEOF(.data) + SIZEOF(.boot.bss)) {
        . += SIZEOF(.boot.bss);
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