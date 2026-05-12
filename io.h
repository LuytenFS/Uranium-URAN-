#ifndef IO_H
#define IO_H

#include "types.h"

/**
 * Send a byte to a specific I/O port.
 */
static inline void outb(unsigned short port, unsigned char val) 
{
    __asm__ volatile ( "outb %0, %1" : : "a"(val), "Nd"(port));
}

/**
 * Send a word to a specific I/O port.
 */
static inline void outw(unsigned short port, unsigned short data) {
  __asm__ volatile("outw %1, %0" : : "dN"(port), "a"(data));
}

/**
 * Receive a byte from a specific I/O port.
 */
static inline unsigned char inb(unsigned short port)
{
    unsigned char ret;
    __asm__ volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void cpuGetMSR(U32 msr, U32 *lo, U32 *hi){
    __asm__ volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}

static inline void cpuSetMSR(U32 msr, U32 lo, U32 hi){
    __asm__ volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}

static inline U32 mmio_read(UINPTR addr){
    return *(volatile U32*)(addr);
}

static inline void mmio_write(UINPTR addr, U32 val){
    *(volatile U32*)(addr) = val;
}

#endif
