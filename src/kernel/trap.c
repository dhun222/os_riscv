#include "types.h"
#include "riscv.h"
#include "plic.h"

#define EXCEPTION_CODE_MASK (0xff)

#define PANIC()  \
    do {                                                    \
        /*kprintf("PANIC: %s:%d\n", __FILE__, __LINE__);*/      \
        while(1);                                           \
    }while(0);                                              \

__attribute__((section(".text.trap"), used))
void trap(void)
{
    // Read scause register
    int64 scause;
    asm volatile("csrr %0, scause" : "=r" (scause));
    int exception_code = scause & EXCEPTION_CODE_MASK;

    if (scause < 0) {
        // if MSB is 1 (which means it's negative in integer), it's interrupt
        if (exception_code == 5) {
            // Timer interrupt
        } 
        else if (exception_code == 9) {
            // External interrupt
            int id = plic_claim();
            if (id) {
                plic_do_service(id);
                plic_complete(id);
            }
        }
    }
    else {
        // Exceptions
        uint64 sstatus;
        asm volatile ("csrr %0, sstatus" : "=r" (sstatus));
        int pp = (sstatus & RISCV_MPP_MASK);
        if (pp) {
            // From S-mode
            PANIC();
        }
        else {
            // From U-mode
            switch (exception_code) {
                case 8:
                // syscall
                break;
                case 13:
                // load page fault
                // We can utilize this on copy on write for fork
                default:
                // Kill this user process
                // We may put specific encodidng into a0 register as return value

            }
        }
    }

    // return
    asm volatile ("sret":::);
}
