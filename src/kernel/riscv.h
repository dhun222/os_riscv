#pragma once
// riscv architecture specific operations

#include "types.h"

// M-mode mstatus reigster bitmap
#define RISCV_MPP_S     (1<<11)     // M-mode previous mode as S-mode
#define RISCV_MPP_MASK  (3<<11)     // Mask for MPP bits
#define RISCV_MIE       (1<<3)      // M-mode global interrupt enable bit

// S-mode sstatus register bitmab
#define RISCV_SIE       (1<<1)      // S-mode global interrupt enable bit
#define RISCV_SPIE      (1<<5)      // S-mode previous global interrupt enable bit


struct context_struct {
    uint64 ra;
    uint64 sp;
    uint64 t0, t1, t2, t3, t4, t5, t6;
    uint64 a0, a1, a2, a3, a4, a5, a6, a7;
};

inline int get_hartid()
{
    uint64 hart;
    asm volatile ("mv %0, tp" : "=r" (hart) :);

    return (int)hart;
}

// ------- maybe for debugging -------
inline void disbale_intr()
{
    uint64 sstatus;
    asm volatile ("csrr %0, sstatus" : "=r" (sstatus)); 
    sstatus = sstatus & (~(1<<1));
    asm volatile ("csrw sstatus, %0" : : "r" (sstatus));
    return;
}

inline void enable_intr()
{
    uint64 sstatus;
    asm volatile ("csrr %0, sstatus" : "=r" (sstatus));
    sstatus = sstatus | (1<<1);
    asm volatile ("csrw sstatus, %0" : : "r" (sstatus));
    return;
}
// -----------------------------------