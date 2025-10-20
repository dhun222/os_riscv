#include "riscv.h"
#include "types.h"
#include "spinlock.h"
#include "uart.h"
#include "plic.h"

#define IRQ_ID 10

// Register address offset
#define UART0       (uint8 *)(0xffff800010000000)  // uart0 base address
#define UART_RHR    0b000       // RX holding register
#define UART_THR    0b000       // TX holding register
#define UART_IER    0b001       // Interrupt enable regsiter
#define UART_FCR    0b010       // FIFO control register
#define UART_ISR    0b010       // Interrupt status register
#define UART_LCR    0b011       // Line control register
#define UART_MCR    0b100       // Modem status register
#define UART_LSR    0b101       // Line status register
#define UART_MSR    0b110       // Modem status register
#define UART_SPR    0b111       // Scratchpad register
#define UART_DLL    0b000       // Divisor latch low
#define UART_DLH    0b001       // Divisor latch high

// Bit maps
#define UART_RHR_INTR_ENABLE        (1<<0)
#define UART_THR_INTR_ENABLE        (1<<1)
#define UART_RLS_INTR_ENABLE        (1<<2)
#define UART_FIFO_ENABLE            (1<<0)
#define UART_FIFO_RX_RESET          (1<<1)
#define UART_FIFO_TX_RESET          (1<<2)
#define UART_FIFO_TRG_LVL14         (3<<6)
#define UART_8N1                    (3<<0)
#define UART_DLAB                   (1<<7)
#define UART_RX_DATA_READY          (1<<0)
#define UART_TX_HOLDING_EMPTY       (1<<5)

#define read_reg(addr)            (*(uint8 *)(UART0 + (addr)))
#define write_reg(addr, val)      *((uint8 *)(UART0 + (addr))) = (val)

struct spinlock_struct uart_lock;

char in_buffer[128];
char out_buffer[128];
uint8 in_buffer_top;
uint8 in_buffer_bot;
uint8 out_buffer_top;
uint8 out_buffer_bot;

inline char in_buffer_pop() {
    char ret = in_buffer[in_buffer_bot++];
    if (in_buffer_bot == 128) {
        in_buffer_bot = 0;
    }
    return ret;
}

inline void in_buffer_push(char val) {
    in_buffer[in_buffer_top++] = val;
    if (in_buffer_top == 128) {
        in_buffer_top = 0;
    }
    return;
}

inline char out_buffer_pop() {
    char ret = out_buffer[out_buffer_bot++];
    if (out_buffer_bot == 128) {
        out_buffer_bot = 0;
    }
    return ret;
}

inline void out_buffer_push(char val) {
    out_buffer[out_buffer_top++] = val;
    if (out_buffer_top == 128) {
        out_buffer_top = 0;
    }
    return;
}

inline int out_buffer_isempty() {
    return out_buffer_bot == out_buffer_top;
}

void uart_service(void) 
{
    // read ISR
    int id = read_reg(UART_ISR);
    id = id & 0b1110;

    int end = 14;
    switch (id) {
        case 2:
        // THR empty
        while (!out_buffer_isempty() && (read_reg(UART_LSR) & 0x20)) {
            write_reg(UART_THR, out_buffer_pop());
        }
        break;
        case 6:
        // Receiver data ready, time out
        end = 1;
        case 4:
        // Receiver data ready, flush rx FIFO
        for (int i = 0; i < end; i++) {
            in_buffer_push(read_reg(UART_RHR));
        }
        break;
    }
    
    return;
}

void uart_init(void) 
{
    // For MMIO
    // Disable interrupt
    write_reg(UART_IER, 0);

    // Set baud rate. In qemu virt board, clock is 1.8432MHz. For 38.4k, divisor should be 3
    write_reg(UART_LCR, UART_DLAB);
    write_reg(UART_DLL, 0x03);
    write_reg(UART_DLH, 0x00);
    write_reg(UART_LCR, 0);

    // Set word length to 8 bits, no parity, no stop bit.
    write_reg(UART_LCR, UART_8N1);

    // Enable and reset FIFO
    write_reg(UART_FCR, UART_FIFO_TRG_LVL14 | UART_FIFO_ENABLE | UART_FIFO_RX_RESET | UART_FIFO_TX_RESET);

    // MCR setting. Set OUT2 high. 
    write_reg(UART_MCR, 1<<3);

    // Set kernel's buffer
    in_buffer_top = 0;
    in_buffer_bot = 0;
    out_buffer_top = 0;
    out_buffer_bot = 0;

    spinlock_init(&uart_lock, "uart");

    plic_register_source(IRQ_ID, uart_service, 1, "uart");

    // Enable interrupts
    write_reg(UART_IER, UART_RHR_INTR_ENABLE | UART_THR_INTR_ENABLE | UART_RLS_INTR_ENABLE);
}

// It must be used only when uart_lock is acquired
inline void putch(char c)
{
    out_buffer_push(c);
}

void puts(char *s)
{
    spinlock_acquire(&uart_lock);
    while (*s) {
        putch(*s);
        s++;
    }
    spinlock_release(&uart_lock);
    return;
}