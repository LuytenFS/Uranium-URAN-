#ifndef TYPES_H
#define TYPES_H

typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;
typedef unsigned long long U64;

typedef signed char IN8;
typedef signed short IN16;
typedef signed int INt32;
typedef signed long long IN64;

typedef U64 UINPTR;

typedef enum {
  VGA_COLOR_BLACK = 0,
  VGA_COLOR_BLUE = 1,
  VGA_COLOR_GREEN = 2,
  VGA_COLOR_CYAN = 3,
  VGA_COLOR_RED = 4,
  VGA_COLOR_MAGENTA = 5,
  VGA_COLOR_BROWN = 6,
  VGA_COLOR_LIGHT_GREY = 7,
  VGA_COLOR_DARK_GREY = 8,
  VGA_COLOR_LIGHT_BLUE = 9,
  VGA_COLOR_LIGHT_GREEN = 10,
  VGA_COLOR_LIGHT_CYAN = 11,
  VGA_COLOR_LIGHT_RED = 12,
  VGA_COLOR_LIGHT_MAGENTA = 13,
  VGA_COLOR_LIGHT_BROWN = 14,
  VGA_COLOR_WHITE = 15,
} vga_color;

static inline U8 vga_entry_color(vga_color foreground, vga_color background){
  return foreground | background << 4;
}

#define UEF_MAGIC 0x55454621

typedef struct{
  U32 magic;
  U32 entry_point;
  U32 data_size;
  U32 code_size;
} __attribute__((packed)) uef_header_t;

#endif