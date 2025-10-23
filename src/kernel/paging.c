#include "riscv.h"
#include "types.h"
#include "paging.h"
#include "memory.h"
#include "utils.h"
#include "spinlock.h"

// sv48 --------------------------
#define PTE_FLAG_OFFSET 10
#define PTE_FLAG_MASK   0x3f
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
extern char eboot[];
extern char etext[];
extern char erodata[];
extern char _kernel_end[];
volatile int paging_on __attribute__((section(".boot.data"))) = 0;
volatile struct boot_info_struct boot_info_src __attribute__((section(".boot.info")));
// --------------------------------------------

#define _N_HART 2
extern char _boot_info[];
volatile struct boot_info_struct *boot_info = (struct boot_info_struct *)_boot_info;
struct spinlock_struct ref_cnt_lock;

// Only used when first direct mapping while booting. 
// Maps vpn and pfn. 
// Works in the physical address space. 
__attribute__((section(".boot.text")))
void kmap(pagetable_t pt, vpn_t vpn, pfn_t pfn, char perm, pfn_t *pt_cnt)
{
    uint8 level = PTE_MAX_LEVEL;
    while (--level) {
        uint32 offset = (vpn >> (VPN_OFFSET * level)) & VPN_MASK;
        pte_t *pte = &(pt[offset]);
        if ((*pte & PTE_V) == 0) {
            *pte = ((*pt_cnt) << PTE_FLAG_OFFSET) | PTE_V;
            boot_info_src.ref_cnt[*pt_cnt] = 0;
            boot_info_src.ref_cnt[paddr2pfn(pt)]++;
            (*pt_cnt)++;
        }
        pt = (pagetable_t)pfn2paddr(pte2pfn(*pte));
    }
    boot_info_src.ref_cnt[paddr2pfn(pt)]++;
    uint32 offset = vpn & VPN_MASK;
    pte_t *pte = &pt[offset];
    *pte = (pfn << (PTE_FLAG_OFFSET)) | PTE_G | perm | PTE_V;

    return;
}

// copies the first page table. 
// If mid-level pt's can be shared, it makes them share.
__attribute__((section(".boot.text")))
void kcopy_pagetable(pagetable_t src, pagetable_t dest, pfn_t *pt_cnt)
{
    boot_info_src.ref_cnt[paddr2pfn(dest)] = boot_info_src.ref_cnt[paddr2pfn(src)];
    for (int i = 0; i < 512; i++) {
        if (src[i] & PTE_V) {
            if (boot_info_src.ref_cnt[pte2pfn(src[i])] == 512 || (src[i] & (PTE_R | PTE_W | PTE_X))) {
                // If next level's page table is full
                // Copies pte exactly same
                // So dest pagetable can point to same next level pagetable as src.
                // Or if it's leaf pte, 
                // Just copies pte.
                dest[i] = src[i] | PTE_G;
                // If mid-level pagetable's pte has PTE_G flag, it means that the pagetable that 
                // pointed by the pte can be shared by all processes.
            }
            else {
                // If not, it means dest pt needs seperate next level pagetable.
                // Allocate new one and copy the contents.
                dest[i] = (*pt_cnt << PTE_FLAG_OFFSET) | PTE_V;
                pagetable_t dest_pt = (pagetable_t)pfn2paddr(*pt_cnt);
                pagetable_t src_pt = (pagetable_t)pfn2paddr(pte2pfn(src[i]));
                (*pt_cnt)++;
                kcopy_pagetable(src_pt, dest_pt, pt_cnt);
            }
        }
    }

    return;
}

static inline void kmemcpy(char *src, char *dest, size_t size)
{
    while (size--) {
        dest[size] = src[size];
    }
    return;
}

__attribute__((section(".boot.text")))
void stack_init()
{
    int hartid = get_hartid();
    if (hartid == 0) {
        // get hardware info
        uint64 ram_size = 512 * 1024 * 1024;   // 512MB 
        uint64 mmio_end = 0x80000000;
        
        // calculate usable physical memory
        int16 *ref_cnt = (int16 *)(_kernel_end);
        paddr_t paddr_max = ram_size + mmio_end;
        pfn_t pfn_max = paddr2pfn(paddr_max);
        pfn_t pfn_low = paddr2pfn(_kernel_end); // lowest PFN excluding MMIO, kernel data, stack, text
        pfn_t usable_pages = pfn_max - pfn_low;
        pfn_t ref_cnt_size = usable_pages / 2049;
        if (usable_pages % 2049) {
            ref_cnt_size++;
        }

        pfn_t pt_pool = (pfn_low + ref_cnt_size);
        pfn_t pt_pool_size = paddr_max >> 19;    // in pages. ram size in page / 128
        pfn_t pfn_min = pt_pool + pt_pool_size;     // can be used by both user and kernel
        ref_cnt -= pt_pool;

        // save informations that are needed after enabling mmu
        boot_info_src.N_HART = _N_HART;
        boot_info_src.ram_size = ram_size;
        boot_info_src.mmio_end = mmio_end;;
        boot_info_src.PFN_MAX = pfn_max;
        boot_info_src.PFN_MIN = pfn_min;
        boot_info_src.ref_cnt = ref_cnt;
        boot_info_src.pt_pool = pt_pool;

        paging_on = 1;
    }
    __sync_synchronize();
    int x;
    do {
        asm volatile("lw %0, %1" : "=r" (x) : "m" (paging_on));
    } while (x == 0);

    // Allocate a page for kernel stack
    char *kernel_stack = (char *)pfn2paddr(boot_info_src.pt_pool + hartid);
    boot_info_src.ref_cnt[boot_info_src.pt_pool + hartid] = 0;

    // copy all content in the stack and move to new stack
    char *stack_top = _kernel_end + (hartid + 1) * 2048;
    char *stack_bot;
    asm volatile("mv %0, sp" : "=r" (stack_bot) : );
    size_t size = stack_top - stack_bot;
    kmemcpy(stack_bot, kernel_stack + 4096 - size, size); 
    stack_bot = kernel_stack + 4096 - size;
    asm volatile("mv sp, %0" : : "r" (stack_bot));

    __sync_synchronize();
    asm volatile("amoadd.d %0, %1, %2" : "=r" (x) : "r" (1), "m" (paging_on));
    do {
        asm volatile("lw %0, %1" : "=r" (x) : "m" (paging_on));
    } while (x <= boot_info_src.N_HART);
    paging_on = -1; // paging_on < 0 -> already acquired by another hart

    return;
}

// Init for paging
// Setup initial global page directory and other data structures
// for physical allocator and page table allocator
__attribute__((section(".boot.text")))
void paging_init()
{
    // Now, all hart's moved to new stack. 
    // We can modify ref_cnt and pt_pool
    int hartid = get_hartid();
    int x;
    pfn_t pt_cnt = boot_info_src.pt_pool + boot_info_src.N_HART;
    pagetable_t gpd = (pagetable_t)(pfn2paddr(pt_cnt));
    if (hartid == 0) {
        // init ref_cnt and pt_pool
        for (pfn_t pfn = pt_cnt; pfn < boot_info_src.PFN_MAX; pfn++) {
            boot_info_src.ref_cnt[pfn] = 1 << 15;
        }
        for (pfn_t pfn_pt = pt_cnt; pfn_pt < boot_info_src.PFN_MIN; pfn_pt++) {
            pagetable_t pt = (pagetable_t)pfn2paddr(pfn_pt);
            for (int i = 0; i < 512; i++) {
                pt[i] = 0;
            }
        }

        // Make initial page table
        boot_info_src.ref_cnt[pt_cnt] = 0;
        pt_cnt++;

        // MMIO
        pfn_t pfn_s = 0;
        pfn_t pfn_e = paddr2pfn(boot_info_src.mmio_end);
        vpn_t vpn = vaddr2vpn(KERNEL_BASE);
        for (pfn_t pfn = pfn_s; pfn < pfn_e; pfn++) {
            kmap(gpd, vpn, pfn, PTE_R | PTE_W, &pt_cnt);
            vpn++;
        }

        // kernel text
        pfn_s = pfn_e;
        pfn_e = paddr2pfn(etext);
        for (pfn_t pfn = pfn_s; pfn < pfn_e; pfn++) {
            kmap(gpd, vpn, pfn, PTE_R | PTE_X, &pt_cnt);
            vpn++;
        }

        // kernel rodata
        pfn_s = pfn_e;
        pfn_e = paddr2pfn(erodata);
        for (pfn_t pfn = pfn_s; pfn < pfn_e; pfn++) {
            kmap(gpd, vpn, pfn, PTE_R, &pt_cnt);
            vpn++;
        }

        // kernel data, bss
        pfn_s = pfn_e;
        pfn_e = paddr2pfn(_kernel_end);
        for (pfn_t pfn = pfn_s; pfn < pfn_e; pfn++) {
            kmap(gpd, vpn, pfn, PTE_R | PTE_W, &pt_cnt);
            vpn++;
        }

        // ref_cnt and pt_pool
        pfn_s = pfn_e;
        pfn_e = boot_info_src.PFN_MIN;
        for (pfn_t pfn = pfn_s; pfn < pfn_e; pfn++) {
            kmap(gpd, vpn, pfn, PTE_R | PTE_W, &pt_cnt);
            vpn++;
        }

        // map stack
        pfn_t kernel_stack = boot_info_src.pt_pool + hartid;
        kmap(gpd, vaddr2vpn(MAX_VA), kernel_stack, PTE_R | PTE_W, &pt_cnt);
        // map temp stack
        kmap(gpd, kernel_stack, kernel_stack, PTE_R | PTE_W, &pt_cnt);

        paging_on = 1;
    }
    else {
        // copy first pagetable
        // Because all kernels reference for same physical frames except it's stack page, 
        // Many pagetables can be shared along process
        // try to acquire simple spinlock
        do {
            asm volatile("amoswap.w %0, %1, %2" : "=r" (x) : "r" (-1), "m" (paging_on));
        } while (x < 0);

        pt_cnt = boot_info_src.pt_pool;
        while (boot_info_src.ref_cnt[pt_cnt] >= 0) {
            pt_cnt++;
        }
        pagetable_t new_gpd = (pagetable_t)pfn2paddr(pt_cnt);
        pt_cnt++;
        kcopy_pagetable(gpd, new_gpd, &pt_cnt);
        gpd = new_gpd;

        // map stack
        pfn_t kernel_stack = boot_info_src.pt_pool + hartid;
        kmap(gpd, vaddr2vpn(MAX_VA), kernel_stack, PTE_R | PTE_W, &pt_cnt);
        // map temp stack
        kmap(gpd, kernel_stack, kernel_stack, PTE_R | PTE_W, &pt_cnt);

        // release simple spinlock
        x++;
        paging_on = x;
    }

    do {
        asm volatile("lw %0, %1" : "=r" (x) : "m" (paging_on));
    } while (x < boot_info_src.N_HART);
    if (hartid == 0) {
        boot_info_src.ref_cnt = (int16 *)((uint64)(boot_info_src.ref_cnt) + KERNEL_BASE);
    }

    write_satp(hartid, paddr2pfn(gpd));
    
    return;
}

// Do the rest of the init
// Pull up sp, unmap temporarily mapped stack
void enable_mmu()
{
    int hartid = get_hartid();
    paddr_t stack_top = pfn2paddr(boot_info->pt_pool + hartid + 1);
    paddr_t stack_bot;
    asm volatile("mv %0, sp" : "=r" (stack_bot) : );
    size_t size = stack_top - stack_bot - 1;
    asm volatile("sub sp, %0, %1" : : "r" (MAX_VA), "r" (size));

    unmap(boot_info->pt_pool + hartid, 1);

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
    while ((boot_info->ref_cnt[pfn] > 0) && pfn < boot_info->PFN_MAX) {
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

    return (vaddr_t)(vpn2vaddr(satp) + KERNEL_BASE);
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
        if ((*pte & PTE_V) == 0) {
            *pte = (alloc_pt() << PTE_FLAG_OFFSET) | PTE_V;
            boot_info->ref_cnt[vaddr2vpn((uint64)pt - KERNEL_BASE)]++;
        }
        pt = (pagetable_t)(vpn2vaddr(pte2pfn(*pte)) + KERNEL_BASE);
    }
    boot_info->ref_cnt[vaddr2vpn((uint64)pt - KERNEL_BASE)]++;
    spinlock_release(&ref_cnt_lock);
    uint32 offset = vpn & VPN_MASK;
    pte_t *pte = &pt[offset];
    *pte = pfn << (PTE_FLAG_OFFSET) | perm | PTE_V;

    return;
}

// unmaps mapping from vpn. 
// If some pagetable or frames should be freed, 
// it also frees them.
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
            if ((*pte & PTE_V) == 0) {
                // tried to unmap unavailable pte
                // panic or return immediately
                return;
            }
            level--;
            pt[level] = (pagetable_t)(vpn2vaddr(pte2pfn(*pte)) + KERNEL_BASE);
        }
        offset[level] = vpn & VPN_MASK;

        // erase pte
        // If can, erase serial pte's in the same page table
        spinlock_acquire(&ref_cnt_lock);
        uint32 i = offset[level];
        pfn_t cur = vaddr2vpn((uint64)pt[level] - KERNEL_BASE);
        while(vpn < end && i < 512) {
            pt[level][i] = 0;
            boot_info->ref_cnt[cur]--;

            i++;
            vpn++;
        }

        while (boot_info->ref_cnt[cur] == 0) {  // There's no any pte in this page table
            boot_info->ref_cnt[cur] = 1 << 15;  // free pagetable
            level++;
            pt[level][offset[level]] = 0;
            boot_info->ref_cnt[cur]--;
        }
        spinlock_release(&ref_cnt_lock);
    }

    return;
}

// copies pagetable
// src: pagetable of source process
// returns pfn of new global page directory
// Must acquire ref_cnt_lock first.
pfn_t copy_pagetable(pagetable_t src)
{
    pfn_t dest_pt = alloc_pt();
    boot_info->ref_cnt[dest_pt] = boot_info->ref_cnt[paddr2pfn(src)];
    pagetable_t dest = (pagetable_t)pfn2paddr(dest_pt);
    for (int i = 0; i < 512; i++) {
        if (src[i] & PTE_V) {
            if (src[i] & PTE_G || src[i] & (PTE_R | PTE_W | PTE_X)) {
                // pointint shared pagetable or leaf pte
                dest[i] = src[i];
            }
            else {
                dest[i] = (copy_pagetable((pagetable_t)pfn2paddr(pte2pfn(src[i]))) << PTE_FLAG_OFFSET) | PTE_V;
            }
        }
    }

    return dest_pt;
}