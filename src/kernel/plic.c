#include "riscv.h"
#include "plic.h"
#include "spinlock.h"
#include "string.h"

#define PLIC_SOURCE_ID_MAX      1024    // Common PLIC can have 1024 sources

#define PLIC_BASE               (uint8 *)0xffff80000c000000  // Base address of PLIC registers. 
#define PLIC_PRIO               (0x0)                       // Source priority setting regsiter. 4Byte/source
#define PLIC_PENDING            (0x1000)                    // Interrupt pending registers. 1bit/source
#define PLIC_ENABLE(hart)       (0x2080 + (hart)*0x100)     // Interrupt enable registers. Add additional bit/bytes to access each sources. 1bit/source, 1kB/target
// ------ These registers makes block per target ------
#define PLIC_THRESHOLD(hart)    (0x201000 + (hart)*0x2000)  // Threshold register. 4B/target
#define PLIC_CLAIM(hart)        (0x201004 + (hart)*0x2000)  // Claim register. 4B/target
#define PLIC_COMPLETE(hart)     (PLIC_CLAIM(hart))          // Complete register. 4B/target. Shares same address with claim regitser
// ------  4KB aligned per target -------

#define read_reg(addr)  (*((uint32 *)(PLIC_BASE + (addr))))
#define write_reg(addr, val)    *((uint32 *)(PLIC_BASE + (addr))) = (val)

struct plic_source_struct plic_source[PLIC_SOURCE_ID_MAX];
struct spinlock_struct plic_source_lock;

void plic_init(int N_HART)
{
    // Init the lock for plic source managing
    spinlock_init(&plic_source_lock, "plic_src");

    for (int hartid = 0; hartid < N_HART; hartid++) {
        // Set threshold
        write_reg(PLIC_THRESHOLD(hartid), 0);

        // Enable all sources
        for (int i = 0; i < 32; i++) {
            write_reg(PLIC_ENABLE(hartid) + i * 4, 0xffffffff);
        }
    }

    return;
}

void plic_register_source(uint32 id, void *service, uint32 prio, char *name)
{
    spinlock_acquire(&plic_source_lock);
    strcpy(plic_source[id].name, name);
    plic_source[id].service = service;
    plic_source[id].prio = prio;
    write_reg(PLIC_PRIO + 4 * id, prio);
    spinlock_release(&plic_source_lock);
}

// Claim for interrupt. Returns id of claimed interrupt.
inline uint32 plic_claim(void)
{
    // Read claim register
    int hart = get_hartid();
    int id = read_reg(PLIC_CLAIM(hart));

    return id;
}

// Send completion msg to PLIC
inline void plic_complete(int intr_id)
{
    // Write complete reigster
    int hart = get_hartid();
    write_reg(PLIC_CLAIM(hart), intr_id);
    return;
}

inline void plic_do_service(int intr_id) 
{
    if (plic_source[intr_id].service) {
        plic_source[intr_id].service();
    }
    return;
}