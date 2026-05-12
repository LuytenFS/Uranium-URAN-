#include "atomf.h"
#include "kernel.h"
#include "m_link.h"
#include "types.h"

void load_uef(U8 *binary_buffer);

char node_msg[] = "\n[NODE] Hello from the Application Space!\n";

// The test node successfully executed in your screenshot
U8 test_node[] = {
    0x21, 0x46, 0x45, 0x55, // Magic: !FEU
    0x10, 0x00, 0x00, 0x00, // Entry point at offset 16
    0x00, 0x00, 0x00, 0x00, // Data size
    0x00, 0x00, 0x00, 0x00, // Code size

    // --- MACHINE CODE ---
    // 1. movabs rdi, <address_of_node_msg> (48 bf followed by 8-byte address)
    0x48, 0xBF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, // We must fill this!

    // 2. mov rax, 0x1220 (The address you found)
    0x48, 0xC7, 0xC0, 0x20, 0x12, 0x00, 0x00,

    // 3. call rax
    0xFF, 0xD0,

    // 4. ret
    0xC3};

void run_command(char *app_name) {
  if (astrcmp(app_name, "test") == 0) {
    // Dynamically patch the string address into the machine code
    U64 msg_addr = (U64)node_msg;
    for (int i = 0; i < 8; i++) {
      test_node[18 + i] = (msg_addr >> (i * 8)) & 0xFF;
    }

    load_uef(test_node);
  }
}