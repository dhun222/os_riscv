# qemu -kernel option cause each hart to jump to 0x8000_0000. 
# With kernel.ld file, code below must be placed at 0x8000_0000. 

.section .text.entry
.global _boot
_boot:
    # set sp of each hart
    # each hart has 4kB(= 1 page) sized kernel stack and guard page respectively. 
    # Each hart needs 8kB(= 2 pages) for the krenel, and uses only 4kB for actual stack. 
    # Disable paging
    csrw satp, 0
    # make space for stacks
    la sp, stack_bot
    li a0, 8 * 1024
    add sp, sp, a0
    csrr tp, mhartid
    mul a0, a0, tp
    add sp, sp, a0
    call init