#include "kernel.h"
#include "m_link.h"
#include "types.h"

typedef void (*uef_entry_t)(void);

void load_uef(U8 *binary_buffer) {
  uef_header_t *header = (uef_header_t *)binary_buffer;

  // Nuke check for container integrity
  if (header->magic != UEF_MAGIC) {
    printstrc("\nRUN: INVALID MAGIC. NUKING CONTAINER.\n", VGA_COLOR_RED);
    return;
  }

  // Calculate jump point via Direct Link
  UINPTR entry_addr = (UINPTR)binary_buffer + header->entry_point;
  uef_entry_t start_node = (uef_entry_t)entry_addr;

  printstrc("\nRUN: Attaching direct link...\n", VGA_COLOR_GREEN);

  // CPU handoff
  start_node();
}