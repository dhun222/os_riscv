#include "types.h"
#include "riscv.h"
#include "paging.h"
#include "plic.h"
#include "uart.h"
#include "trap.h"
#include "timer.h"
#include "schedule.h"
#include "memory.h"

#define N_HART 2

extern char _main_paddr[];

volatile static int booted = 0;

// Main function of the kernel. 
// Now, we're in S-mode and virtual address space 
// All interrupts are disabled. 
// Do the rest of the jobs about booting, 
// and make init user process and call sched function to run the user process
// init user process will execute user shell
__attribute__((section(".text.main")))
void main() 
{
    int hartid = get_hartid();
    // Global init
    if (hartid == 0) {
        plic_init(N_HART);
        uart_init();
        memory_init();
        
        __sync_synchronize();
        booted = 1;
    }
    int x;
    do {
        asm volatile("lw %0, %1" : "=r" (x) : "m" (booted));
    } while (x == 0);

    // Per-hart init
    // paging 
    enable_mmu();
    // register S-mode interrupt handler
    asm volatile("csrw stvec, %0" : : "r" (trap));
    // Initial user proc
    //proc_init();
    

    // Call sched function to switch to init user process
    __sync_synchronize();
    asm volatile("amoadd.d %0, %1, %2" : "=r" (x) : "r" (1), "m" (booted));
    do {
        asm volatile("ld %0, %1" : "=r" (x) : "m" (booted));
    } while(x != N_HART + 1);


    //sched();
    asm volatile("sd %0, -8(fp)" : : "r" (ret_user));
    return;
}

/* 
* for M-Mode settings and paging init
* enables mmu and goes to S-Mode. 
* Runs on all harts
*/
__attribute__((section(".boot.text")))
void init(void) 
{
    uint64 mstatus;
    asm volatile("csrr %0, mstatus" : "=r" (mstatus));
    // set PP bits and SPIE bits
    mstatus = mstatus & (~RISCV_MPP_MASK);
    mstatus = mstatus | RISCV_MPP_S;
    mstatus = mstatus & (~RISCV_SPIE);  // disable S-mode interrupt before ceding control to S-mode
    mstatus = mstatus & (~RISCV_SIE);
    mstatus = mstatus & (~RISCV_MIE);   // disable M-mode interrupt for now
    mstatus = mstatus & (~RISCV_MPIE);
    asm volatile("csrw mstatus, %0" : : "r" (mstatus));

    // We have to read out DT. 
    // But for now, I'm going to use jsut defined constants
    
    // set all sie bits
    //asm volatile("csrw mie, %0" : : "r" (0x222));

    // Delegate all interrupt and exception handling to S-Mode
    asm volatile("csrw mideleg, %0" : : "r" (0xffff));
    asm volatile("csrw medeleg, %0" : : "r" (0xffff));

    // Give S-Mode all authority on physical memory
    // TOR mode, all authority, tob boundary is same as max value in 56bit
    asm volatile("csrw pmpcfg0, %0" : : "r" (0x0f));
    asm volatile("csrw pmpaddr0, %0" : : "r" (0x3fffffffffffffull));

    // paging
    stack_init();
    paging_init();

    // timer
    timer_init();
    
    //
    asm volatile("csrw stvec, %0" : : "r" (0x1000));
    //
    asm volatile("csrw mepc, %0" : : "r" (KERNEL_BASE + _main_paddr));
    asm volatile("mret");

    return;
}