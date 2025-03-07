#include "uart.h"
#include "riscv.h"
#include "spinlock.h"
#include "pmem.h"

struct spinLock_struct uart_lock;

void initUART(void) 
{
    // disable interrupt
    UARTWriteReg(UART_IER, 0x00);

    // baud rate set mode
    UARTWriteReg(UART_LCR, UART_DIVOSOR_LATCH_ACCESS);

    // Set baud rate. For 38.4k, divisor should be 3
    UARTWriteReg(UART_DIVISOR_LATCH_L, 0x03);
    UARTWriteReg(UART_DIVISOR_LATCH_H, 0x00);

    // Leave baud rate set mode
    UARTWriteReg(UART_LCR, 0);

    // Set word length to 8 bits
    UARTWriteReg(UART_LCR, UART_WORD_LEN_EIGHT);

    // Enable and reset FIFO
    UARTWriteReg(UART_FCR, UART_FIFO_ENABLE | UART_FIFO_RX_RESET | UART_FIFO_TX_RESET);

    // Enable interrupts
    UARTWriteReg(UART_IER, UART_RHR_INTR_ENABLE | UART_THR_INTR_ENABLE);

    initSpinLock(&uart_lock, "uart");
}