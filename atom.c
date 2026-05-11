#include "kernel.h"

int astrcmp(char *s1, char *s2) {
  int i = 0;
  while (s1[i] != '\0' && s2[i] != '\0') {
    if (s1[i] != s2[i])
      return 1; // Not a match
    i++;
  }
  // Return 0 only if both reached the end at the same time
  return (s1[i] == s2[i]) ? 0 : 1;
}

void atom_main(){
    vga_print_string("Welcome to URAN!\n");
    vga_print_string("ATOM TERMINAL\n");
    vga_print_string("ATOM >");

    while(1){
        if(command_rdy){
          if (astrcmp((char *)atom_buffer, "cls") == 0){
            cls_screen();
            cursor_x = 0;
            cursor_y = 0;
          } else if (astrcmp((char *)atom_buffer, "ver") == 0) {
            vga_print_string("\nURAN Kernel v0.01 - 2026\n");
          } else if (astrcmp((char *)atom_buffer, "exit") == 0){
            vga_print_string("\nShutting down...\n");
            shutdown();
          } 
          else {
            vga_print_string("\nUnknown ATOM command: ");
            vga_print_string((char *)atom_buffer);
          }
          buffer_idx = 0;
          command_rdy = 0;
          vga_print_string("\nATOM >");
        }
        __asm__ volatile("hlt");
    }
}