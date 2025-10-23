# qemu -kernel option cause each hart to jump to 0x8000_0000. 
# With kernel.ld file, code below must be placed at 0x8000_0000. 

.section .entry
.global _entry
_entry:
    # set sp of each hart
    # Each hart needs 2kB for stack while booting.
    # Disable paging
    csrw satp, 0
    # make space for stacks
    la sp, _kernel_end
    li a0, 2 * 1024
    add sp, sp, a0
    csrr tp, mhartid
    mul a0, a0, tp
    add sp, sp, a0
    call init