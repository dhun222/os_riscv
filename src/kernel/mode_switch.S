# for mode switching
# recover kernel's context (sp, pc ...)

.section .text.trap
.global trap
trap:
    # save all user context
    sd t0, 0(sp)
    sd t1, 8(sp)
    sd t2, 16(sp)
    sd t3, 24(sp)
    sd t4, 32(sp)
    sd t5, 40(sp)
    sd t6, 48(sp)
    sd a0, 56(sp)
    sd a1, 64(sp)
    sd a2, 72(sp)
    sd a3, 80(sp)
    sd a4, 88(sp)
    sd a5, 96(sp)
    sd a6, 104(sp)
    sd a7, 112(sp)
    
    # recover kernel stack pointer and save user stack pointer
    mv t0, sp
    csrr sp, sscratch
    csrw sscratch, t0

    # call trap handler function
    call trap_handler

ret_user:
    # All jobs are done
    # store kernel stack and recover user stack
    # We don't have to save kernel context, because kernel reaches here with normal return
    csrr t0, sscratch
    csrw sscratch, sp
    mv sp, t0

    # Recover user context
    ld t0, 0(sp)
    ld t1, 8(sp)
    ld t2, 16(sp)
    ld t3, 24(sp)
    ld t4, 32(sp)
    ld t5, 40(sp)
    ld t6, 48(sp)
    ld a0, 56(sp)
    ld a1, 64(sp)
    ld a2, 72(sp)
    ld a3, 80(sp)
    ld a4, 88(sp)
    ld a5, 96(sp)
    ld a6, 104(sp)
    ld a7, 112(sp)
    
    # return to user
    sret

