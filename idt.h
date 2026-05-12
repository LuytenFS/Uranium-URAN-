#ifndef IDT_H
#define IDT_H

struct idt_entry {
  unsigned short offset_low; // 0-15
  unsigned short selector;   // 16-31
  unsigned char ist;         // 32-39 (Interrupt Stack Table)
  unsigned char type_attr;   // 40-47 (Gate Type, DPL, Present)
  unsigned short offset_mid; // 48-63
  unsigned int offset_high;  // 64-95
  unsigned int reserved;     // 96-127 (Must be zero)
} __attribute__((packed));

struct idt_ptr {
  unsigned short limit;
  unsigned long base;
} __attribute__((packed));

#endif
