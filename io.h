#ifndef IO_H
#define IO_H

/**
 * Send a byte to a specific I/O port.
 * 'val' goes into the EAX register ("a")
 * 'port' goes into the EDX register ("d")
 */
static inline void outb(unsigned short port, unsigned char val) 
{
    __asm__ volatile ( "outb %0, %1" : : "a"(val), "Nd"(port));
}

/**
 * Send a word to a specific I/O port.
 * 'val' goes into the EAX register ("a")
 * 'port' goes into the EDX register ("d")
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

#endif
