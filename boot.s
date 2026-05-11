[bits 16]
[org 0x7c00]

KERNEL_OFFSET equ 0x1000

start:
    mov [BOOT_DRIVE], dl
    ; 1. Reset Disk System (Sometimes needed for reliability)
    mov ah, 0
    int 0x13

    ; 2. Setup the buffer address (ES:BX)
    mov ax, 0
    mov es, ax              ; ES = 0
    mov bx, KERNEL_OFFSET   ; BX = 0x1000. So we read to 0x0000:1000

    ; 3. Setup Read Parameters
    mov ah, 0x02            ; BIOS Read Sector Function
    mov al, 10              ; Read 10 sectors (more than enough for your kernel)
    mov ch, 0x00            ; Cylinder 0
    mov dh, 0x00            ; Head 0
    mov cl, 0x02            ; Sector 2 (Sector 1 is this bootloader)
    mov dl, [BOOT_DRIVE]    ; The drive number we saved at 'start'
    int 0x13

    ; 4. Error Check
    jc disk_error           ; If Carry Flag is set, the BIOS failed  cli
    
    cli
    
    xor ax, ax
    mov ds, ax

    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:init_pm

[bits 32]
init_pm:
  mov ax, DATA_SEG
  mov ds, ax
  mov ss, ax
  mov es, ax
  mov fs, ax
  mov gs, ax

  mov ebp, 0x90000
  mov esp, ebp

  call KERNEL_OFFSET
  jmp $

gdt_start:
  dq 0x0
gdt_code:
  dw 0xffff, 0x0, 0x9a00, 0x00cf
gdt_data:
  dw 0xffff, 0x0, 0x9200, 0x00cf
gdt_end:
gdt_descriptor:
  dw gdt_end - gdt_start - 1
  dd gdt_start

disk_error:
  jmp $

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start
BOOT_DRIVE db 0

times 510-($-$$) db 0
dw 0xaa55
