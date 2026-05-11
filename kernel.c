#include "io.h"
#include "idt.h"
#include "kernel.h"

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

void update_cursor(int x, int y) {
  unsigned short pos = y * 80 + x;

  // Send High Byte
  outb(0x3D4, 0x0E);
  outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));

  // Send Low Byte
  outb(0x3D4, 0x0F);
  outb(0x3D5, (unsigned char)(pos & 0xFF));
}

void vga_print_char(char c) {
  struct vga_char *buffer = (struct vga_char *)0xB8000;

  // Print at the current cursor position
  int offset = cursor_y * 80 + cursor_x;
  buffer[offset].character = c;
  buffer[offset].style = 0x0A; // Bright Green for URAN

  cursor_x++;
  if (cursor_x >= 80) { // Simple line wrapping
    cursor_x = 0;
    cursor_y++;
  }
  update_cursor(cursor_x, cursor_y);
}

void vga_print_string(char *str){
  for (int i = 0; str[i] != '\0'; i++){
    if(str[i] == '\n'){
      cursor_x = 0;
      cursor_y++;
    } else {
      vga_print_char(str[i]);
    }

    if(cursor_y >= 25){
      cursor_y = 0;
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

void set_idt_gate(int n, unsigned int handler);

    void idt_load() {
  __asm__ volatile("lidt (%0)" : : "r" (&idtp));
}

    void init_idt() {
      idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
      idtp.base = (unsigned int)(unsigned long int)&idt;

      // Clear everything
      for (int i = 0; i < 256; i++) {
        idt[i].type_attr = 0; // "Not Present"
      }

      // Only hook the keyboard
      set_idt_gate(33, (unsigned int)(unsigned long int)keyboard_handler_asm);

      idt_load();
    }

void set_idt_gate(int n, unsigned int handler){
  idt[n].offset_low = handler & 0xFFFF;
  idt[n].selector = 0x08;
  idt[n].zero = 0;
  idt[n].type_attr = 0x8E;
  idt[n].offset_high = (handler >> 16) & 0xFFFF;
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
  unsigned char scancode = inb(0x60);

  // Check if this is a "Break" code (key release)
  if (scancode & 0x80) {
    unsigned char release_code = scancode & 0x7F; // Remove the release bit

    if (release_code == 0x2A || release_code == 0x36) { // Left or Right Shift
      shift_pressed = 0;
    }
  }
  // This is a "Make" code (key press)
  else {
    if (scancode == 0x2A || scancode == 0x36) {
      shift_pressed = 1;
    } else if (scancode == 0x3A) { // Caps Lock
      caps_lock = !caps_lock;
    } else {
      char c = kbd_map[scancode];

      if (c != 0) {
        // Apply modifiers
        if (shift_pressed ^ caps_lock) { // XOR: Shift OR Caps, but not both
          c = to_upper(c);
        }

        // Pass to ATOM's input logic
        process_input(c);
      }
    }
  }
  outb(0x20, 0x20); // Acknowledge the interrupt to the PIC
}

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

void cls_screen()
{
  struct vga_char* buffer = (struct vga_char*)0xB8000;
  for (int i = 0; i < 80 * 25; i++)
  {
    buffer[i].character = ' ';
    buffer[i].style = 0x07; // gray on black
  }
}

void __attribute__((section(".text._start"))) _start()
{
    cls_screen(); // Clear the BIOS junk first

    init_idt();

    pic_remap();

    set_idt_gate(33, (unsigned int)(unsigned long)keyboard_handler_asm);

    __asm__ volatile("sti");

    cursor_x = 0;
    cursor_y = 0;

    extern void atom_main(); // Defined in atom.c
    atom_main();
    while(1); 
}
