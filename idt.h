#ifndef IDT_H
#define IDT_H

struct idt_entry {
  unsigned short offset_low;  // Offset bits 0..15 (Bottom box, right side)
  unsigned short selector;    // Segment Selector (Bottom box, left side)
  unsigned char zero;         // Reserved/Zero (Top box, bits 0..7)
  unsigned char type_attr;    // Flags/Attributes (Top box, bits 8..15)
  unsigned short offset_high; // Offset bits 16..31 (Top box, left side)
} __attribute__((packed));


struct idt_ptr{
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

#endif
