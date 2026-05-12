#ifndef M_LINK_H
#define M_LINK_H

#include "types.h"
#include "kernel.h"

// --- RADIUM WRAPPERS ---

static inline void printc(char c) { vga_print_char(c); }

static inline void printstr(char *str) { vga_print_string(str); }

static inline void printcc(char c, U8 color) {
  U8 old = current_term_color;
  current_term_color = color;
  vga_print_char(c);
  current_term_color = old;
}

static inline void printstrc(char *str, U8 color) {
  U8 old = current_term_color;
  current_term_color = color;
  vga_print_string(str);
  current_term_color = old;
}

static inline void clear_zone() { cls_screen(); }

// --- RADIUM WRAPPERS ---

#endif