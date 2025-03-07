# qemu -kernel option cause each hart to jump to 0x8000_0000. 
# With kernel.ld file, code below must be placed at 0x8000_0000. 

.section .text
.global _boot
_boot:
    # set sp of each hart
    # each hart has 4kB(= 1 page) sized kernel stack + guard page. 
    # Each hart needs 8kB(= 2 pages) for the krenel, and uses only 4kB for actual stack. 
    # stack_start is declared in init.c
    # openSBI saves mhartid into a0 register. I'm going to follow that rule. 
    la sp, stack_start
    csrr a0, mhartid
    li a1, 1024 * 4
    mul a1, a1, a0
    add sp, sp, a1
    call init