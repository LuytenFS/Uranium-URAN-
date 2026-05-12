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

    ; --- Set up Paging ---
    ; We move tables to 0x70000 to avoid overwriting a large kernel
    %define PML4_ADDR 0x70000
    %define PDPT_ADDR 0x71000
    %define PDT_ADDR  0x72000

    mov edi, PML4_ADDR
    xor eax, eax
    mov ecx, 3072             ; Clear 12KB
    rep stosd

    ; 1. PML4 -> PDPT
    mov eax, PDPT_ADDR | 0x03 ; Present + Writable
    mov [PML4_ADDR], eax

    ; 2. PDPT -> PDT
    ; Map entry 0 (0-1GB) and entry 3 (3-4GB)
    mov eax, PDT_ADDR | 0x03      
    mov [PDPT_ADDR], eax        ; Entry 0
    mov [PDPT_ADDR + 24], eax    ; Entry 3 (3 * 8 bytes)

    ; 3. PDT -> 2MB Huge Pages
    ; Entry 0: Identity maps 0.0MB to 2.0MB
    mov eax, 0x00000083       ; Huge + Writable + Present
    mov [PDT_ADDR], eax

    ; Entry 502: Map 0xFEC00000 range (I/O APIC) 
    ; 0x18 = PCD (Cache Disable) + PWT (Write Through) - Essential for MMIO
    mov eax, 0xFEC0009B       ; Huge + Writable + Present + PCD + PWT
    mov [PDT_ADDR + (502 * 8)], eax

    ; Entry 503: Map 0xFEE00000 range (Local APIC)
    mov eax, 0xFEE0009B
    mov [PDT_ADDR + (503 * 8)], eax

    ; --- Enable Long Mode ---
    mov eax, PML4_ADDR
    mov cr3, eax

    mov eax, cr4
    or eax, 1 << 5            ; PAE
    mov cr4, eax

    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8            ; LME
    wrmsr

    mov eax, cr0
    or eax, 0x80000001        ; PG + PE
    mov cr0, eax

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

    ; Place stack at 0x90000 (Safe from kernel and page tables)
    mov rsp, 0x90000          
    call KERNEL_OFFSET        
    jmp $

; --- GDT Structures ---
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