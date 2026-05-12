[bits 64]
extern keyboard_handler_main
global keyboard_handler_asm

keyboard_handler_asm:
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    call keyboard_handler_main

    ; Restore in reverse order
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax

    iretq    ; 64-bit Interrupt Return

global spurious_handler_asm

spurious_handler_asm:
    ; Spurious interrupts don't usually push an error code,
    ; and we don't need to save registers if we aren't calling C.
    iretq