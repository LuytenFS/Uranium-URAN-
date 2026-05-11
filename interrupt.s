[bits 32]
extern keyboard_handler_main
global keyboard_handler_asm

keyboard_handler_asm:
    pushad                ; Save all General Purpose Registers
    call keyboard_handler_main
    popad                 ; Restore all General Purpose Registers
    iretd                 ; Interrupt Return (32-bit)