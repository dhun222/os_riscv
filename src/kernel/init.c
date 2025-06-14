#include "types.h"
#include "riscv.h"
#include "plic.h"
#include "uart.h"
#include "trap.h"

/*
#include "virtio.h"
#include "sched.h"
*/

// Main function of the kernel. 
// Now, we're in S-mode. 
// All interrupts are disabled. 
// Do the rest of the jobs about booting, 
// and make init user process and call sched function to run the user process
// init user process will execute user shell
void main(void) 
{
    int hartid = get_hartid();
    // Global init
    if (hartid == 0) {
        plic_source_init();
        uart_init();
        puts("Hello World!\n");
        //disk_init();
        //rtc_init();
    }

    // Per-hart init
    // PLIC init
    plic_hart_init(hartid);
    enable_intr();
    while(1);

    // Make pagetable for initial user process 

    // Enable paging
    
    // Enable U-mode interrupt

    // Call sched function to switch to init user process
    //sched();
}

/* 
* for M-Mode settings
* do its job and go to S-Mode. 
* Runs on all harts
*/
void init(void) 
{
    uint64 mstatus;
    asm volatile("csrr %0, mstatus" : "=r" (mstatus));
    // set PP bits and SPIE bits
    mstatus = mstatus & (~RISCV_MPP_MASK);
    mstatus = mstatus | RISCV_MPP_S;
    mstatus = mstatus & (~RISCV_SPIE);  // disable S-mode interrupt
    asm volatile("csrw mstatus, %0" : : "r" (mstatus));

    // register S-mode interrupt handler
    asm volatile("csrw stvec, %0" : : "r" (trap));

    // Delegate all interrupt and exception handling to S-Mode
    asm volatile("csrw mideleg, %0" : : "r" (0xffff));
    asm volatile("csrw medeleg, %0" : : "r" (0xffff));

    // Give S-Mode all authority on physical memory
    // TOR mode, all authority, tob boundary is same as max value in 56bit
    asm volatile("csrw pmpcfg0, %0" : : "r" (0x0f));
    asm volatile("csrw pmpaddr0, %0" : : "r" (0x3fffffffffffffull));
    
    // set mepc to address of kernel main function
    asm volatile("csrw mepc, %0" : : "r" (main));

    // set all mie bits
    asm volatile("csrw mie, %0" : : "r" (0x222));

    // jump to kernel main switching to S-Mode
    asm volatile("mret");
}