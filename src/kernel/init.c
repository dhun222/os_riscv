#include "types.h"
#include "riscv.h"
#include "paging.h"
#include "plic.h"
#include "uart.h"
#include "trap.h"

/*
#include "virtio.h"
#include "sched.h"
*/

#define N_HART 2

extern char _stack_bot[];
extern char _main[];
volatile uint64 stack_top __attribute__((section(".boot.bss")));
volatile uint64 stack_bot __attribute__((section(".boot.bss")));
extern int paging_on;
extern struct boot_info_struct boot_info_src;

volatile char booted = 0;

__attribute__((section(".boot.text")))
void breakpoint()
{
    return;
}
// Main function of the kernel. 
// Now, we're in S-mode and virtual address space 
// All interrupts are disabled. 
// Do the rest of the jobs about booting, 
// and make init user process and call sched function to run the user process
// init user process will execute user shell
__attribute__((section(".text.main")))
void main() 
{
    enable_mmu();

    int hartid = get_hartid();
    // Global init
    if (hartid == 0) {
        plic_source_init();
        uart_init();
        //disk_init();
        //rtc_init();
    }

    // Per-hart init
    // register S-mode interrupt handler
    asm volatile("csrw stvec, %0" : : "r" (trap));
    // PLIC init
    plic_hart_init(hartid);
    enable_intr();

    // Call sched function to switch to init user process
    __sync_synchronize();
    while(booted != N_HART);
    //sched();
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
    mstatus = mstatus & (~RISCV_MIE);   // disable M-mode interrupt for now
    asm volatile("csrw mstatus, %0" : : "r" (mstatus));

    // We have to read out DT. 
    // But for now, I'm going to use jsut defined constants
    
    // set all mie bits
    asm volatile("csrw mie, %0" : : "r" (0x222));

    // Delegate all interrupt and exception handling to S-Mode
    asm volatile("csrw mideleg, %0" : : "r" (0xffff));
    asm volatile("csrw medeleg, %0" : : "r" (0xffff));

    // Give S-Mode all authority on physical memory
    // TOR mode, all authority, tob boundary is same as max value in 56bit
    asm volatile("csrw pmpcfg0, %0" : : "r" (0x0f));
    asm volatile("csrw pmpaddr0, %0" : : "r" (0x3fffffffffffffull));
    
    // paging
    int hartid = get_hartid();
    if (hartid == 0) {
        stack_bot = (uint64)_stack_bot;
        stack_top = stack_bot + (N_HART) * 8 * 1024;
        paging_init();
    }
    __sync_synchronize();
    int x;
    do {
        asm volatile("lw %0, %1" : "=r" (x) : "m" (paging_on));
    } while (x == 0);

    // write satp
    uint64 satp = ((uint64)9 << 60) | (boot_info_src.pt_pool - boot_info_src.map_offset);
    asm volatile("\
        csrw satp, %0   \n\
        sfence.vma      \n\
        fence.i         \n\
        " : : "r" (satp));

    asm volatile("csrw mepc, %0" : : "r" (KERNEL_BASE + 0x1000));
    asm volatile("mret");

    return;
}