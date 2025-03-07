/* 
*   0x8000_0000 ~           DRAM
*   0x1000_0000, 0x100      uart0
*   0x1000_1000, 0x1000 * 8 virtio disk, which is plugged into virtio bus 0
*   0x0C00_0000,            PLIC
*   0x0200_0000,            CLINT
*/

// uart
#define UART0 0x10000000

// virtio disk
#define VIRTIO0 0x10001000

// PLIC
#define PLIC 0x0c000000                                         // Base address of PLIC registers
#define PLIC_PRIORITY (PLIC + 0x0)                              // Priority setting regsiter
#define PLIC_PENDING (PLIC + 0x1000)                            // Interrupt pending registers
#define PLIC_ENABLE(hart) (PLIC + 0x2080 + (hart)*0x100)        // Interrupt enable registers. Add additional bit/bytes to access each sources. .... always enable..?
#define PLIC_THRESHOLD(hart) (PLIC + 0x201000 + (hart)*0x2000)  // Threshold register
#define PLIC_CLAIM(hart) (PLIC + 0x201004 + (hart)*0x2000)      // Claim register
#define PLIC_COMPLETE(hart) (PLIC_CLAIM(hart))                  // Complete register

