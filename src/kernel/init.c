#include "types.h"
#include "config.h"
#include "riscv.h"

__attribute__ ((aligned (16))) char stack_start[4096 * NHART];

void main();

/* 
* for M-Mode settings
* do its job and go to S-Mode. 
* Runs on all harts
*/
void init(void) 
{
    // Disable paging
    asm volatile("csrw satp, %0" : : "r" (0));

    // Disable all M-mode interrupts during this procedure
    uint64 mstatus;
    asm volatile("csrr %0, mstatus" : "=r" (mstatus));
    mstatus = mstatus & (~RISCV_MIE_MASK);
    mstatus = mstatus | RISCV_MIE;

    // set PP bits to S-Mode
    mstatus = mstatus & (~RISCV_MPP_MASK);
    mstatus = mstatus | RISCV_MPP_S;
    asm volatile("csrw mstatus, %0" : : "r" (mstatus));

    // Delegate all interrupt and exception handling to S-Mode
    asm volatile("csrw mideleg, %0" : : "r" (0xffff));
    asm volatile("csrw medeleg, %0" : : "r" (0xffff));

    // Give S-Mode all authority on physical memory
    // TOR mode, all authority, tob boundary is same as max value in 56bit
    asm volatile("csrw pmpcfg0, %0" : : "r" (0x0f));
    asm volatile("csrw pmpaddr0, %0" : : "r" (0xffffffffffffff));

    // set mepc to address of kernel main function
    asm volatile("csrw mepc, %0" : : "r" (main));

    // save hartid into tp which is not used for any process
    // in boot.S, a0 contains the mhartid
    asm volatile("addi tp, a0, 0");

    // jump to kernel main switching to S-Mode
    asm volatile("mret");
}