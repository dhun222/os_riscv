#include "types.h"

// QEMU virt board's CLINT frequency is 10MHz, and timer also follows this frequency.

#define INTR_TICK   10     // Period for timer interrupt. 1 millisecond

volatile static uint64 ticks_val;  // System time in millisecond. Other part of the kernel can only read this value

inline uint64 read_time()
{
    uint64 time;
    asm volatile("csrr %0, stimecmp" : "=r" (time) : );
    return time;
}

inline void set_alarm(uint64 time)
{
    asm volatile("csrw stimecmp, %0" : : "r" (time));
    return;
}

void timer_service()
{
    // get current time
    uint64 time = read_time();

    // set next alarm
    set_alarm(time + INTR_TICK);
    
    // Increase ticks
    ticks_val++;

    // scheduler

    return;
}

// Returns ticks_val to serve system time
uint64 ticks()
{
    return ticks_val;
}

__attribute__((section(".boot.text")))
void timer_init() 
{
    asm volatile("csrw mcounteren, %0" : : "r"(2));
    uint64 time;
    asm volatile ("csrr %0, time" : "=r" (time) :);
    time /= INTR_TICK;  // round down
    asm volatile("csrw stimecmp, %0" : : "r" (time + INTR_TICK));

    return;
}
