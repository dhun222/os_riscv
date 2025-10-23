#pragma once
#include "types.h"

#define KERNEL_BASE 0xffff800000000000
#define SATP_MODE       (uint64)9
#define PTE_V               1
#define PTE_R               2
#define PTE_W               4
#define PTE_X               8
#define PTE_U               16
#define PTE_G               32

typedef uint64 pfn_t;
typedef int64 vpn_t;
typedef uint64 pte_t;
typedef pte_t *pagetable_t;

struct boot_info_struct{
    uint64 ram_size;
    uint64 mmio_end;
    int16 *ref_cnt; // MSB 0: unused, MSB 1: used
    pfn_t PFN_MAX;
    pfn_t PFN_MIN;
    pfn_t pt_pool;
    uint8 N_HART;
};

static inline void write_satp(uint64 asid, pfn_t gpd)
{
    // ---------- satp ----------
    // |63    60|59    44|43    0|
    // |  MODE  |  ASID  |  PFN  |
    // |    4   |   16   |   44  |
    uint64 satp = (SATP_MODE << 60) | (asid << 44) | gpd; 
    asm volatile("  csrw satp, %0       \n\
                    sfence.vma          \n\
                    fence.i             \n\
                    " : : "r" (satp));
    return;
}

void stack_init();
void paging_init();
void enable_mmu();
pfn_t copy_pagetable(pagetable_t src);
void map(vpn_t vpn, pfn_t pfn, char perm);
void unmap(vpn_t vpn, uint64 size);
pfn_t alloc_page();
