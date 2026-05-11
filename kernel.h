#ifndef KERNEL_H
#define KERNEL_H

struct vga_char {
  char character;
  char style;
};

extern int shift_pressed;
extern int caps_lock;
extern int cursor_x;
extern int cursor_y;

extern volatile char atom_buffer[256];
extern volatile int buffer_idx;
extern volatile int command_rdy;

void vga_print_char(char c);
void vga_print_string(char* str);
void update_cursor(int x, int y);
void update_cursor_backwards();
void process_input(char c);
char to_upper(char c);
void cls_screen();
void shutdown();

#endif