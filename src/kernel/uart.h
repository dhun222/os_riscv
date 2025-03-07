// Register address
#define UART_RHR    0x000
#define UART_THR    0x000
#define UART_IER    0x001
#define UART_FCR    0x010
#define UART_ISR    0x010
#define UART_LCR    0x011
#define UART_MCR    0x100
#define UART_LSR    0x101
#define UART_MSR    0x110
#define UART_SPR    0x111
#define UART_DIVISOR_LATCH_L    0x000
#define UART_DIVISOR_LATCH_H    0x001

// Bit maps
#define UART_RHR_INTR_ENABLE        1<<0
#define UART_THR_INTR_ENABLE        1<<1
#define UART_FIFO_ENABLE            1<<0
#define UART_FIFO_RX_RESET          1<<1
#define UART_FIFO_TX_RESET          1<<2
#define UART_WORD_LEN_EIGHT         3<<0
#define UART_DIVOSOR_LATCH_ACCESS   1<<7
#define UART_RX_DATA_READY          1<<0
#define UART_TX_HOLDING_EMPTY       1<<5

#define UARTReadReg(reg) (*((volatile uint8 *)(UART0 + (reg))))
#define UARTWriteReg(reg, val) (*((volatile uint8 *)(UART0 + (reg))) = (val))

void initUART(void);