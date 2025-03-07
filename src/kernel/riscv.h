// riscv architecture specific operations

// M-mode mstatus reigster bitmap
#define RISCV_MPP_S     (1<<11)     // M-mode previous mode as S-mode
#define RISCV_MPP_MASK  (3<<11)     // Mask for MPP bits
#define RISCV_MIE       (1<<3)      // M-mode global interrupt enable bit
#define RISCV_MIE_MASK  (1<<3)      // Mask for MIE bit

// S-mode sstatus register bitmab
#define RISCV_SIE       (1<<1)      // S-mode global interrupt enable bit
#define RISCV_SPIE      (1<<5)      // S-mode previous global interrupt enable bit

inline uint32 getHartid()
{
    int hart;
    asm volatile("lw %0, tp" : "=r" hart);

    return hart;
}