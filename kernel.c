#include "io.h"
#include "idt.h"
#include "kernel.h"
#include "types.h"
#include "atomf.h"

#define LAPIC_SVR 0x0F0
#define LAPIC_APIC_BASE_MSR 0x1B
#define IOAPIC_REGSEL 0x00
#define IOAPIC_IOWIN 0x10

struct idt_entry idt[256];
struct idt_ptr idtp;
int shift_pressed = 0;
int caps_lock = 0;
int cursor_x = 0;
int cursor_y = 0;
volatile char atom_buffer[256];
volatile int buffer_idx = 0;
volatile int command_rdy = 0;
extern void keyboard_handler_asm();
extern void spurious_handler_asm();
static unsigned char last_scancode = 0;
U8 current_term_color = 0x09;

void safe_print(char *str) {
  if (cursor_y < 28) {
    cursor_y = 28;
  }
  vga_print_string(str);
}

void vga_print_char(char c) {
  struct vga_char *buffer = (struct vga_char *)0xB8000;

  if (cursor_y >= 25) {
    cls_screen();
    cursor_x = 0;
    cursor_y = 0;
  }

  int offset = cursor_y * 80 + cursor_x;
  buffer[offset].character = c;
  buffer[offset].style = current_term_color; 

  cursor_x++;
  if (cursor_x >= 80) {
    cursor_x = 0;
    cursor_y++;
  }
  update_cursor(cursor_x, cursor_y);
}

void init_apic() {
  U32 lo, hi;
  cpuGetMSR(LAPIC_APIC_BASE_MSR, &lo, &hi);

  UINPTR apic_base = (lo & 0xfffff000);

  lo |= (1 << 11);
  cpuSetMSR(LAPIC_APIC_BASE_MSR, lo, hi);

  UINPTR svr_val = mmio_read(apic_base + LAPIC_SVR);
  svr_val |= (1 << 8) | 0xFF;
  mmio_write(apic_base + LAPIC_SVR, svr_val);
}

void ioapic_write(UINPTR base, U32 reg, U32 data) {
  mmio_write(base + IOAPIC_REGSEL, reg);
  mmio_write(base + IOAPIC_IOWIN, data);
}

void init_ioapic(UINPTR ioapic_base) {
  U32 low = 33;

  U32 high = 0; // Destination APIC ID 0

  // Map Pin 1 (Standard Keyboard)
  ioapic_write(ioapic_base, 0x12, low);
  ioapic_write(ioapic_base, 0x13, high);

  // Map Pin 2 (Common Redirected Keyboard)
  ioapic_write(ioapic_base, 0x14, low);
  ioapic_write(ioapic_base, 0x15, high);
}

void update_cursor(int x, int y) {
  unsigned short pos = y * 80 + x;

  // Send High Byte
  outb(0x3D4, 0x0E);
  outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));

  // Send Low Byte
  outb(0x3D4, 0x0F);
  outb(0x3D5, (unsigned char)(pos & 0xFF));
}

void vga_print_string(char *str) {
  for (int i = 0; str[i] != '\0'; i++) {
    if (str[i] == '\n') {
      cursor_x = 0;
      cursor_y++;
    } else {
      // Manually placing the char to handle scrolling logic before printing
      if (cursor_x >= 80) {
        cursor_x = 0;
        cursor_y++;
      }

      // Perform the scroll if we hit the 25-line limit
      if (cursor_y >= 25) {
        struct vga_char *buffer = (struct vga_char *)0xB8000;

        // Shift lines 1-24 up to lines 0-23
        for (int y = 0; y < 24; y++) {
          for (int x = 0; x < 80; x++) {
            buffer[y * 80 + x] = buffer[(y + 1) * 80 + x];
          }
        }

        // Clear the bottom line (Line 24) with the current theme color
        for (int x = 0; x < 80; x++) {
          buffer[24 * 80 + x].character = ' ';
          buffer[24 * 80 + x].style = current_term_color;
        }

        cursor_y = 24; // Stay on the bottom line
      }

      // Now safely print the character
      struct vga_char *buffer = (struct vga_char *)0xB8000;
      int offset = cursor_y * 80 + cursor_x;
      buffer[offset].character = str[i];
      buffer[offset].style = current_term_color;
      cursor_x++;
    }
  }
  update_cursor(cursor_x, cursor_y);
}

void update_cursor_backwards() {
  if (cursor_x > 0) {
    cursor_x--;
  } else if (cursor_y > 0) {
    cursor_y--;
    cursor_x = 79;
  }

  struct vga_char *buffer = (struct vga_char *)0xB8000;
  int offset = cursor_y * 80 + cursor_x;
  buffer[offset].character = ' ';
  buffer[offset].style = 0x07;

  update_cursor(cursor_x, cursor_y);
}

void process_input(char c){
  if (c == '\b'){
    if(buffer_idx > 0){
      buffer_idx--;
      update_cursor_backwards();
    }
  } else if(c == '\n'){
    atom_buffer[buffer_idx] = '\0';
    command_rdy = 1;
  } else {
    if (buffer_idx < 254){
      atom_buffer[buffer_idx++] = c;
      vga_print_char(c);
    }
  }
}

char to_upper(char c){
  if(c >= 'a' && c <= 'z'){
    return c -32;
  }
  if (c >= '0' && c <= '9') {
    switch (c) {
    case '1':
      return '!';
    case '2':
      return '@';
    case '3':
      return '#';
    case '4':
      return '$';
    case '5':
      return '%';
    case '6':
      return '^';
    case '7':
      return '&';
    case '8':
      return '*';
    case '9':
      return '(';
    case '0':
      return ')';
    }
  }
  return c;
}

void set_idt_gate(int n, unsigned long handler);

    void idt_load() {
  __asm__ volatile("lidt (%0)" : : "r" (&idtp));
}

void init_idt() {
  idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
  idtp.base = (unsigned long int)&idt;

  for (int i = 0; i < 256; i++) {
    idt[i].type_attr = 0;
  }

  set_idt_gate(33, (unsigned long int)keyboard_handler_asm);
  set_idt_gate(255, (unsigned long int)spurious_handler_asm);

  idt_load();
}

void set_idt_gate(int n, unsigned long handler) {
  idt[n].offset_low = (unsigned short)(handler & 0xFFFF);
  idt[n].selector = 0x08;
  idt[n].ist = 0;
  idt[n].type_attr = 0x8E; // 64-bit Interrupt Gate, Present, Ring 0
  idt[n].offset_mid = (unsigned short)((handler >> 16) & 0xFFFF);
  idt[n].offset_high = (unsigned int)((handler >> 32) & 0xFFFFFFFF);
  idt[n].reserved = 0; // 32-bit reserved field
}

// A simplified US-QWERTY layout
unsigned char kbd_map[128] = {
    0,    27,  '1',  '2', '3', '4',  '5',  '6',
    '7',  '8', '9',  '0', '-', '=',  '\b', /* Backspace */
    '\t', 'q', 'w',  'e', 'r', 't',  'y',  'u',
    'i',  'o', 'p',  '[', ']', '\n', /* Enter */
    0,                               /* Control */
    'a',  's', 'd',  'f', 'g', 'h',  'j',  'k',
    'l',  ';', '\'', '`', 0, /* Left Shift */
    '\\', 'z', 'x',  'c', 'v', 'b',  'n',  'm',
    ',',  '.', '/',  0,   '*', 0,    ' ' /* Space */
};

void keyboard_handler_main() {
  mmio_write(0xFEE000B0, 0); // EOI
  unsigned char scancode = inb(0x60);
  if (scancode == last_scancode) {
    return;
  }
  last_scancode = scancode;
  if (scancode & 0x80) {
    last_scancode = 0;
    unsigned char release_code = scancode & 0x7F;
    if (release_code == 0x2A || release_code == 0x36)
      shift_pressed = 0;
    return;
  } else {
    // KEY PRESS LOGIC
    if (scancode == 0x2A || scancode == 0x36) {
      shift_pressed = 1;
    } else if (scancode == 0x3A) {
      caps_lock = !caps_lock;
    } else {
      char c = kbd_map[scancode];
      if (c != 0) {
        if (shift_pressed ^ caps_lock)
          c = to_upper(c);
        process_input(c);
      }
    }
  }
}

/* old PIC logic, possibly may be reutilized for legacy hardware releases of URAN
void pic_remap() {
  outb(0x20, 0x11);
  outb(0xA0, 0x11);
  outb(0x21, 0x20);
  outb(0xA1, 0x28);
  outb(0x21, 0x04);
  outb(0xA1, 0x02);
  outb(0x21, 0x01);
  outb(0xA1, 0x01);

  // MASK: 0xFD is 11111101 binary.
  // This disables the Timer (Bit 0) but keeps the Keyboard (Bit 1) ON.
  outb(0x21, 0xFD);
  outb(0xA1, 0xFF); // Disable all slave interrupts
}
*/

void disable_pic() {
  outb(0x21, 0xFF);
  outb(0xA1, 0xFF);
}

void shutdown() {
  // This works on many older chipsets and emulators
  outw(0x604, 0x2000);

  // If that fails, we enter an infinite halt so the CPU stops
  // consuming cycles while the user manually flips the switch.
  vga_print_string("\nIt is now safe to turn off your computer.");
  while (1) {
    __asm__ volatile("cli; hlt");
  }
}

void cls_screen() {
  struct vga_char *buffer = (struct vga_char *)0xB8000;
  for (int i = 0; i < 80 * 25; i++) {
    buffer[i].character = ' ';
    buffer[i].style = current_term_color; // Match the new background/foreground
  }
}

UINPTR find_ioapic_address() {
  // Search BIOS memory from 0xE0000 to 0xFFFFF for "RSD PTR "
  for (char *p = (char *)0x000E0000; p < (char *)0x000FFFFF; p += 16) {
    if (astrcmp_n(p, "RSD PTR ", 8) == 0) {
      return 0xFEC00000;
    }
  }
  return 0xFEC00000; // Fallback
}

void set_keyboard_rate() {
  // 0xF3 is the 'Set Typematic Rate/Delay' command
  while (inb(0x64) & 2)
    ;
  outb(0x60, 0xF3);

  while (inb(0x64) & 2)
    ;
  // 0x7F = 01111111b
  // Bits 5-6 (11): 1000ms delay before repeating
  // Bits 0-4 (11111): ~2.0 characters per second (slowest)
  outb(0x60, 0x7F);
}

void __attribute__((section(".text._start"))) _start() {
  cls_screen();

  // 1. Setup Interrupts
  init_idt();

  // 2. Setup Local APIC
  init_apic();

  // 3. Setup I/O APIC (Hardcoded for now to match boot.s mapping)
  UINPTR ioapic = 0xFEC00000;
  init_ioapic(ioapic);

  // 4. Switch from PIC to APIC
  disable_pic();

  // 5. Enable Keyboard Controller (8042)
  while (inb(0x64) & 2);               // Wait for not busy
  outb(0x64, 0xAE); // Enable keyboard port

  set_keyboard_rate();
  
  // 6. Final Prep
  __asm__ volatile("sti"); // Enable interrupts now that setup is done

  // 7. Enter ATOM
  extern void atom_main();
  atom_main();

  // Catch-all
  while (1) {
    __asm__ volatile("hlt");
  }
}