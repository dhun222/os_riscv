#pragma once
#include "types.h"

#define KERNEL_BASE 0xffff800000000000

typedef uint64 paddr_t;
typedef int64 vaddr_t;
typedef uint64 pfn_t;
typedef int64 vpn_t;
typedef uint64 pte_t;
typedef pte_t *pagetable_t;
typedef uint64 satp_t;

struct boot_info_struct{
    int16 *ref_cnt; // MSB 1: unused, MSB 0: used
    pfn_t PFN_MAX;
    pfn_t PFN_MIN;
    pfn_t pt_pool;
    uint64 stack_size;
    uint64 stack_bot;
    vpn_t map_offset;
};

void paging_init();
void enable_mmu();
void write_satp(uint64 asid, pfn_t gpd);
pfn_t copy_pagetable(satp_t src);
