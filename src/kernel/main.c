#include "types.h"
#include "riscv.h"
#include "config.h"
#include "virtio.h"
#include "uart.h"
#include "plic.h"
#include "sched.h"

void enableIntr(void) 
{
    return;
}

// Main function of the kernel. 
// Now, we're in S-mode. 
// Do the rest of the jobs about booting, 
// and make init user process and call sched function to run the user process
// init user process will execute user shell
void main(void) 
{
    // Initialize IO devices
    //   uart
    initUART();

    //   PLIC
    initPLIC();

    //   disk
    initDisk();

    // Make pagetable for initial user process 
    
    // execute init user process

    // Enable interrupts
    enableIntr();

    // Call sched function to switch to init user process
    sched();
}