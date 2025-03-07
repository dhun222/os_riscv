#include "riscv.h"
#include "pmem.h"
#include "plic.h"

#define PLIC_PRIO           0x0000      // PLIC priority register
#define PLIC_IP             0x1000      // PLIC interrupt pending bit
#define PLIC_ENABLE         0x2000      // PLIC interrupt enable bit
#define PLIC_TARGET         0x200000    // PLIC per-target(hart) registers
#define PLIC_TARGET_OFFSET  0x1000      // offset for threshold, claim/complete, registers of each target(hart context)


void initPLIC(void)
{
    // Set Priority

    // 
    return;
}

// Claim for interrupt. Returns id of claimed interrupt.
inline uint32 claimPLIC(void)
{
    // Read claim register
    int hart = getHartid();
    int id = *(PLIC_CLAIM(hart));

    return id;
}

void completePLIC(int intr_id)
{
    // Write complete reigster
    int hart = getHartid();
    *(PLIC_CLAIM(hart)) = intr_id;
}

void 