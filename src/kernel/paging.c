#include "types.h"
#include "paging.h"
#include "memory.h"
#include "utils.h"
#include "spinlock.h"

// sv48 --------------------------
#define SATP_MODE       (uint64)9
#define PTE_FLAG_OFFSET 10
#define PTE_FLAG_MASK   0x3f
#define V               1
#define R               2
#define W               4
#define X               8
#define U               16
#define G               32
#define MAX_VA          0xffffffffffffffff

#define PTE_MAX_LEVEL   4
#define PAGE_OFFSET     12
#define PAGE_MASK       0xfff

#define VPN_MASK        0x1ff
#define VPN_OFFSET      9
#define PFN_MASK        0xfffffffffff
#define VADDR_MASK      0xffffffffffff

#define pfn2paddr(pfn)      ((paddr_t)((pfn) << PAGE_OFFSET))
#define vpn2vaddr(vpn)      ((vaddr_t)(vpn) << PAGE_OFFSET)
#define paddr2pfn(paddr)    (((pfn_t)paddr) >> PAGE_OFFSET)
#define vaddr2vpn(vaddr)    (~((~((vpn_t)(vaddr))) >> PAGE_OFFSET))
#define pte2pfn(pte)        ((pfn_t)(pte) >> PTE_FLAG_OFFSET)
// --------------------------------

// For booting (referenced by physical address space) -----------------
extern uint64 stack_bot;
extern uint64 stack_top;
extern char _boot_info[];
extern char eboot[];
extern char etext[];
extern char erodata[];
uint64 boot_data;
volatile int paging_on __attribute__((section(".boot.data"))) = 0;
volatile struct boot_info_struct boot_info_src __attribute__((section(".boot.info")));
// --------------------------------------------

extern char _boot_info[];
volatile struct boot_info_struct *boot_info;
struct spinlock_struct ref_cnt_lock;

void write_satp(uint64 asid, pfn_t gpd)
{
    // ---------- satp ----------
    // |63    60|59    44|43    0|
    // |  MODE  |  ASID  |  PFN  |
    // |    4   |   16   |   44  |
    uint64 satp = (SATP_MODE << 60) | (asid << 44) | gpd; 
    asm volatile("csrw satp, %0" : : "r" (satp));
    return;
}

// Only used when first direct mapping while booting. 
// Maps vpn and pfn. 
// Works in the physical address space. 
inline void kmap(vpn_t vpn, pfn_t pfn, char perm, pfn_t *pt_cnt)
{
    int level = PTE_MAX_LEVEL;
    pagetable_t pt = (pagetable_t)(boot_info_src.pt_pool << PAGE_OFFSET);
    while (--level) {
        uint32 offset = (vpn >> (VPN_OFFSET * level)) & VPN_MASK;
        pte_t *pte = &(pt[offset]);
        if ((*pte & V) == 0) {
            *pte = ((*pt_cnt) << PTE_FLAG_OFFSET) | V;
            boot_info_src.ref_cnt[*pt_cnt] = 0;
            boot_info_src.ref_cnt[paddr2pfn(pt)]++;
            (*pt_cnt)++;
        }
        pt = (pagetable_t)pfn2paddr(pte2pfn(*pte));
    }
    boot_info_src.ref_cnt[paddr2pfn(pt)]++;
    uint32 offset = vpn & VPN_MASK;
    pte_t *pte = &pt[offset];
    *pte = pfn << (PTE_FLAG_OFFSET) | perm | V;

    return;
}
// Init for paging
// Setup initial global page directory and other data structures
// for physical allocator and page table allocator
__attribute__((section(".boot.text")))
void paging_init()
{
    // get hardware info
    uint64 ram_size = 512 * 1024 * 1024;   // 512MB 
    uint64 mmio_end = 0x80000000;

    // calculate usable physical memory
    int16 *ref_cnt = (int16 *)(stack_top);
    paddr_t paddr_max = ram_size + mmio_end;
    pfn_t pfn_max = paddr2pfn(paddr_max);
    pfn_t pfn_low = paddr2pfn(ref_cnt); // lowest PFN excluding MMIO, kernel data, stack, text
    pfn_t usable_pages = pfn_max - pfn_low;
    pfn_t ref_cnt_size = usable_pages / 2049;
    if (usable_pages % 2049) {
        ref_cnt_size++;
    }

    pfn_t pt_pool = (pfn_low + ref_cnt_size);
    pfn_t pt_pool_size = paddr_max >> 21;    // in pages. ram size / 512
    pfn_t pfn_min = pt_pool + pt_pool_size;     // can be used by both user and kernel
    ref_cnt -= pt_pool;

    // set 0's in pt_pool
    for (pfn_t pt_i = pt_pool; pt_i < pt_pool + pt_pool_size; pt_i++) {
        pagetable_t pt = (pagetable_t)pfn2paddr(pt_i);
        for (int pte_i = 0; pte_i < 512; pte_i++) {
            pt[pte_i] = 0;
        }
    }
    // set MSB 1's in ref_cnt
    for (uint64 i = pt_pool; i < pfn_max; i++) {
        ref_cnt[i] = -1;
    }

    // save informations that are needed after enabling mmu
    boot_info_src.PFN_MAX = pfn_max;
    boot_info_src.PFN_MIN = pfn_min;
    boot_info_src.ref_cnt = ref_cnt;
    boot_info_src.pt_pool = pt_pool;
    boot_info_src.stack_size = stack_top - stack_bot;
    boot_info_src.stack_bot = stack_bot;
    
    // install initial page tables
    pfn_t pt_cnt = pt_pool + 1;
    vpn_t vpn = vaddr2vpn(KERNEL_BASE);
    pfn_t pfn_s, pfn_e;
    // MMIO
    pfn_s = 0;
    pfn_e = paddr2pfn(mmio_end);
    for (pfn_t pfn = pfn_s; pfn < pfn_e; pfn++) {
        kmap(vpn, pfn, R | W, &pt_cnt);
        vpn++;
    }

    // kernel text
    pfn_s = pfn_e;
    pfn_e = paddr2pfn(etext);
    for (pfn_t pfn = pfn_s; pfn < pfn_e; pfn++) {
        kmap(vpn, pfn, R | X, &pt_cnt);
        vpn++;
    }

    // kernel rodata
    pfn_s = paddr2pfn(etext);
    pfn_e = paddr2pfn(erodata);
    for (pfn_t pfn = pfn_s; pfn < pfn_e; pfn++) {
        kmap(vpn, pfn, R|W, &pt_cnt);
        vpn++;
    }

    // kernel data and bss
    pfn_s = paddr2pfn(erodata);
    pfn_e = paddr2pfn(stack_bot);
    for (pfn_t pfn = pfn_s; pfn < pfn_e; pfn++) {
        kmap(vpn, pfn, R | W, &pt_cnt);
        vpn++;
    }

    // ref_cnt
    pfn_s = paddr2pfn(stack_top);
    pfn_e = pfn_s + ref_cnt_size;
    boot_info_src.map_offset = vpn - pfn_s;
    for (pfn_t pfn = pfn_s; pfn < pfn_e; pfn++) {
        kmap(vpn, pfn, R | W, &pt_cnt);
        vpn++;
    }

    // pt_pool
    pfn_s = pfn_e;
    pfn_e = pfn_min;
    for (pfn_t pfn = pfn_s; pfn < pfn_e; pfn++) {
        kmap(vpn, pfn, R | W, &pt_cnt);
        vpn++;
    }

    // kernel stack
    pfn_s = paddr2pfn(stack_bot);
    pfn_e = paddr2pfn(stack_top);
    vpn = paddr2pfn(MAX_VA - stack_top + stack_bot) + 1;
    for (pfn_t pfn = pfn_s; pfn < pfn_e; pfn++) {
        kmap(vpn, pfn, R | W, &pt_cnt);
        vpn++;
    }

    // temporary mapping for stack
    // enable_mmu() function will unamp this area
    pfn_s = paddr2pfn(stack_bot);
    pfn_e = paddr2pfn(stack_top);
    for (pfn_t pfn = pfn_s; pfn < pfn_e; pfn++) {
        kmap(pfn, pfn, R|W, &pt_cnt);
    }

    // modify some address into vaddr
    boot_info_src.ref_cnt = ((int16 *)((uint64)stack_bot + KERNEL_BASE)) - boot_info_src.pt_pool;

    __sync_synchronize();
    paging_on = 1;

    return;
}

// Allocates new pagetable in pt_pool and returns its pfn
// Must acquire 'ref_cnt_lock' first
pfn_t alloc_pt()
{
    pfn_t pfn = boot_info->pt_pool;
    while ((boot_info->ref_cnt[pfn] != -1) && pfn < boot_info->PFN_MIN) {
        pfn++;
    }
    if (pfn == boot_info->PFN_MIN) {
        // There's no available free page for page table
        // panic
    }
    boot_info->ref_cnt[pfn] = 0;

    return pfn;
}

// Allocates new page in the area between PFN_MIN and PFN_MAX
// Returns its pfn
// Must acquire 'ref_cnt_lock' first
pfn_t alloc_page()
{
    // It takes O(n) time. 
    // ToDo: Make it into buddy system.
    pfn_t pfn = boot_info->PFN_MIN;
    while ((boot_info->ref_cnt[pfn] != -1) && pfn < boot_info->PFN_MAX) {
        pfn++;
    }
    if (pfn == boot_info->PFN_MAX) {
        // There's no available free page
        // panic
    }
    boot_info->ref_cnt[pfn] = 0;

    return pfn;
}

inline vaddr_t get_gpd_vaddr()
{
    uint64 satp;
    asm volatile("csrr %0, satp" : "=r" (satp) :);
    satp &= PFN_MASK;

    return (vaddr_t)(vpn2vaddr(satp + boot_info->map_offset));
}

// Maps vpn to pfn. 
// Used by memory allocator
void map(vpn_t vpn, pfn_t pfn, char perm)
{
    int level = PTE_MAX_LEVEL;
    // get global page directory
    pagetable_t pt = (pagetable_t)get_gpd_vaddr();

    spinlock_acquire(&ref_cnt_lock);
    boot_info->ref_cnt[pfn]++;
    while (--level) {
        uint32 offset = (vpn >> (VPN_OFFSET * level)) & VPN_MASK;
        pte_t *pte = &pt[offset];
        if ((*pte & V) == 0) {
            *pte = (alloc_pt() << PTE_FLAG_OFFSET) | V;
            boot_info->ref_cnt[vaddr2vpn(pt) - boot_info->map_offset]++;
        }
        pt = (pagetable_t)(vpn2vaddr(pte2pfn(*pte)) + boot_info->map_offset);
    }
    boot_info->ref_cnt[vaddr2vpn(pt) - boot_info->map_offset]++;
    spinlock_release(&ref_cnt_lock);
    uint32 offset = vpn & VPN_MASK;
    pte_t *pte = &pt[offset];
    *pte = pfn << (PTE_FLAG_OFFSET) | perm | V;

    return;
}

void unmap(vpn_t vpn, uint64 size)
{
    vpn_t end = vpn + size;
    // get global page directory
    pagetable_t pt[PTE_MAX_LEVEL];
    uint32 offset[PTE_MAX_LEVEL];
    pt[PTE_MAX_LEVEL - 1] = (pagetable_t)get_gpd_vaddr();

    while (vpn < end) {
        // walk and record page table's addresses and offsets
        int level = PTE_MAX_LEVEL - 1;
        while (level) {
            offset[level] = (vpn >> (VPN_OFFSET * level)) & VPN_MASK;
            pte_t *pte = &(pt[level][offset[level]]);
            if ((*pte & V) == 0) {
                // tried to unmap unavailable pte
                // panic or return immediately
                return;
            }
            level--;
            pt[level] = (pagetable_t)(vpn2vaddr(pte2pfn(*pte) + boot_info->map_offset));
        }
        offset[level] = vpn & VPN_MASK;

        // erase pte
        // If can, erase serial pte's in the same page table
        spinlock_acquire(&ref_cnt_lock);
        uint32 i = offset[level];
        pfn_t cur = vaddr2vpn(pt[level]) - boot_info->map_offset;
        while(vpn < end && i < 512) {
            pt[level][i] = 0;
            boot_info->ref_cnt[cur]--;

            i++;
            vpn++;
        }

        while (boot_info->ref_cnt[cur] == 0) {  // There's no any pte in this page table
            boot_info->ref_cnt[cur] = -1;  // free pagetable
            level++;
            pt[level][offset[level]] = 0;
            cur = vaddr2vpn(pt[level]) - boot_info->map_offset;
            boot_info->ref_cnt[cur]--;
        }
        spinlock_release(&ref_cnt_lock);
    }

    return;
}

// Do the rest of the job
// We're in S-mode
// Copies boot_info struct into virtual address space, 
// unmaps temporary mapped pages, 
// move up sp and pc
void enable_mmu()
{
    // get boot info
    boot_info = (struct boot_info_struct *)_boot_info;

    // move up sp 
    volatile int64 sp;
    asm volatile("mv %0, sp" : "=r" (sp));
    int64 offset = MAX_VA - ((uint64)(&(boot_info->ref_cnt[boot_info->pt_pool])) + boot_info->stack_size - KERNEL_BASE) + 1;
    sp += offset;
    asm volatile("mv sp, %0" : : "r" (sp));

    // unmap temporary stack mapping
    unmap(vaddr2vpn(boot_info->stack_bot), vaddr2vpn(boot_info->stack_size));

    return;
}

// copies pagetable
// for internal use
// returns pfn of copied pagetable
pfn_t _copy_pagetable(pfn_t pfn_src) 
{
    pagetable_t src_pt = (pagetable_t)vpn2vaddr(pfn_src + boot_info->map_offset);
    pfn_t pfn_dest = alloc_pt();
    pagetable_t dest = (pagetable_t)(vpn2vaddr(pfn_dest + boot_info->map_offset));
    spinlock_acquire(&ref_cnt_lock);
    boot_info->ref_cnt[pfn_dest] = boot_info->ref_cnt[pfn_src]; // copy ref_cnt value
    spinlock_release(&ref_cnt_lock);
    for (int i = 0; i < 512; i++) {
        if (src_pt[i] & V) {
            if (dest[i] & (W | R | X)) {    // it's leaf pte
                dest[i] = src_pt[i];    // copy exact same thing
            }
            else {
                dest[i] = src_pt[i] & PTE_FLAG_MASK; // copy only flag
                dest[i] |= _copy_pagetable(pte2pfn(src_pt[i])) << PTE_FLAG_OFFSET;  // fill up with new page table's pfn
            }
        }
    }
    
    return pfn_dest;
}

// copies pagetable
// src: satp of source process
// returns pfn of new global page directory
pfn_t copy_pagetable(satp_t src)
{
    pfn_t pfn_src = src & PFN_MASK;
    pagetable_t src_pt = (pagetable_t)vpn2vaddr(pfn_src + boot_info->map_offset);
    pfn_t pfn_dest = alloc_pt();
    pagetable_t dest = (pagetable_t)(vpn2vaddr(pfn_dest + boot_info->map_offset));
    spinlock_acquire(&ref_cnt_lock);
    boot_info->ref_cnt[pfn_dest] = boot_info->ref_cnt[pfn_src];// copy ref_cnt value
    spinlock_release(&ref_cnt_lock);
    for (int i = 0; i < 512; i++) {
        if (src_pt[i] & V) {
            dest[i] = src_pt[i];    // copy only flags
            dest[i] |= _copy_pagetable(pte2pfn(src_pt[i])) << PTE_FLAG_OFFSET;
        }
    }

    return pfn_dest;
}