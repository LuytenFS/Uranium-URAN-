#ifndef KERNEL_H
#define KERNEL_H

#include "types.h"

struct vga_char {
  char character;
  char style;
};

#define KERNEL_VER "\nURAN Kernel v0.30 X86_64 - 2026\n"

extern int shift_pressed;
extern int caps_lock;
extern int cursor_x;
extern int cursor_y;

extern volatile char atom_buffer[256];
extern volatile int buffer_idx;
extern volatile int command_rdy;
extern U8 current_term_color;

void vga_print_char_color(char c, U8 color);
void vga_print_char(char c);
void vga_print_string(char* str);
void update_cursor(int x, int y);
void update_cursor_backwards();
void process_input(char c);
char to_upper(char c);
void cls_screen();
void shutdown();

void run_command(char *app_name);

#endif