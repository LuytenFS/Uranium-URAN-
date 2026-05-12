#include "atomf.h"
#include "kernel.h"
#include "m_link.h"
#include "types.h"

// Forward declaration of the bridge in run.c
void run_command(char *app_name);

// --- String Utilities ---

int astrcmp(char *s1, char *s2) {
  int i = 0;
  while (s1[i] != '\0' && s2[i] != '\0') {
    if (s1[i] != s2[i])
      return 1;
    i++;
  }
  return (s1[i] == s2[i]) ? 0 : 1;
}

int astrcmp_n(const char *s1, const char *s2, int n) {
  for (int i = 0; i < n; i++) {
    if (s1[i] != s2[i]) {
      return 1;
    }
  }
  return 0;
}

// --- Main Hub ---

void atom_main() {
  current_term_color = vga_entry_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
  cls_screen();
  cursor_x = 0;
  cursor_y = 0;

  vga_print_string("ATOM > ");

  while (1) {
    if (command_rdy) {
      if (astrcmp((char *)atom_buffer, "cls") == 0) {
        cls_screen();
        cursor_x = 0;
        cursor_y = 0;
      } else if (astrcmp((char *)atom_buffer, "ver") == 0) {
        printstrc(KERNEL_VER, VGA_COLOR_LIGHT_GREEN);
        printstrc(ATOM_VER, VGA_COLOR_LIGHT_BLUE);
      } else if (astrcmp((char *)atom_buffer, "exit") == 0) {
        printstrc("\nShutting down...\n", VGA_COLOR_LIGHT_GREEN);
        shutdown();
      } else if (astrcmp_n((char *)atom_buffer, "run", 3) == 0) {
        if (atom_buffer[3] == ' ') {
          run_command((char *)atom_buffer + 4);
        } else if (atom_buffer[3] == '\0') {
          printstrc("\nUsage: run <node_name>", VGA_COLOR_LIGHT_BROWN);
        } else {
          goto unknown;
        }
      } else if (astrcmp((char *)atom_buffer, "ver") == 0) {
        printstrc(KERNEL_VER, VGA_COLOR_LIGHT_GREEN);
      } else {
      unknown:
        printstrc("\nUnknown command: ", VGA_COLOR_LIGHT_RED);
        printstr((char *)atom_buffer);
      }

      // Cleanup
      for (int i = 0; i < 256; i++)
       atom_buffer[i] = 0;
      buffer_idx = 0;
      command_rdy = 0;
      printstr("\nATOM > ");
    }
    __asm__ volatile("hlt");
  }
}