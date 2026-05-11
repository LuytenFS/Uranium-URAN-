[bits 16]
[org 0x7c00]

KERNEL_OFFSET equ 0x1000

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov [BOOT_DRIVE], dl  
    
    ; 1. Reset Disk System
    mov ah, 0
    int 0x13

    ; 2. Read Kernel into memory at 0x1000
    mov ax, 0
    mov es, ax
    mov bx, KERNEL_OFFSET
    mov ah, 0x02
    mov al, 50              
    mov ch, 0x00
    mov dh, 0x00
    mov cl, 0x02
    mov dl, [BOOT_DRIVE]
    int 0x13

    jc disk_error

    ; 3. Transition to 32-bit Protected Mode
    cli                     
    lgdt [gdt32_descriptor]
    mov eax, cr0
    or eax, 0x1             
    mov cr0, eax
    
    jmp CODE_SEG32:init_pm    

[bits 32]
init_pm:
    mov ax, DATA_SEG32        
    mov ds, ax
    mov ss, ax
    mov es, ax

    ; --- Set up Paging (Identity map first 2MB) ---
    mov edi, 0x10000
    xor eax, eax
    mov ecx, 4096
    rep stosd

    mov edi, 0x10000
    mov dword [edi], 0x11003      
    mov dword [edi + 0x1000], 0x12003
    mov dword [edi + 0x2000], 0x13003
    
    mov edi, 0x13000
    mov ebx, 0x00000003           
    mov ecx, 512
.map_loop:
    mov [edi], ebx
    add ebx, 0x1000
    add edi, 8
    loop .map_loop

    ; --- Enable PAE and Long Mode ---
    mov eax, 0x10000
    mov cr3, eax                  
    mov eax, cr4
    or eax, 1 << 5                
    mov cr4, eax

    mov ecx, 0xC0000080           
    rdmsr
    or eax, 1 << 8                
    wrmsr

    mov eax, cr0
    or eax, 1 << 31               
    mov cr0, eax

    ; --- Jump to 64-bit Code ---
    lgdt [gdt64_descriptor]
    jmp CODE_SEG64:init_lm

[bits 64]
init_lm:
    mov ax, DATA_SEG64
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov rsp, 0x90000              
    call KERNEL_OFFSET            
    jmp $

; --- GDT Structures ---

; 32-bit GDT (Used for the initial jump)
gdt32_start:
    dq 0x0
gdt32_code: 
    dw 0xffff, 0x0000
    db 0x00, 10011010b, 11001111b, 0x00
gdt32_data:
    dw 0xffff, 0x0000
    db 0x00, 10010010b, 11001111b, 0x00
gdt32_end:

gdt32_descriptor:
    dw gdt32_end - gdt32_start - 1
    dd gdt32_start

; 64-bit GDT (Used for Long Mode)
gdt64_start:
    dq 0x0
gdt64_code:
    dq (1<<43) | (1<<44) | (1<<47) | (1<<53) 
gdt64_data:
    dq (1<<44) | (1<<47) | (1<<41)           
gdt64_end:

gdt64_descriptor:
    dw gdt64_end - gdt64_start - 1
    dq gdt64_start

CODE_SEG32 equ gdt32_code - gdt32_start
DATA_SEG32 equ gdt32_data - gdt32_start
DATA_SEG64 equ gdt64_data - gdt64_start
CODE_SEG64 equ gdt64_code - gdt64_start

disk_error:
    jmp $

BOOT_DRIVE db 0

times 510-($-$$) db 0
dw 0xaa55