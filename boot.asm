[org 0x7c00]
KERNEL_OFFSET equ 0x1000 

mov [BOOT_DRIVE], dl    

mov bp, 0x9000          
mov sp, bp

; czyta dane o pamięci
call get_memory_info

call load_kernel        

set_vga_mode:
    mov ax, 0x4F02
    mov bx, 0x411B   ;(1280x720x24-bit + LFB)
    int 0x10
    cmp ax, 0x004F   
    jne error_no_vbe
    jmp switch_to_pm

    
get_memory_info:
    ; le czytanie danych o pamięci
    xor ax, ax
    mov ax, 0xE801
    int 0x15
    jc mem_error     ; flaga gdy błąd
    
    ; megabajty poniżej 16 na 0x7000
    mov [0x7000], ax
    ; bloki po 64k powyżej 16mb na 0x7004
    mov [0x7004], bx
    ret

mem_error:
    mov byte [0x7000], 0 ; błąd
    ret

error_no_vbe:
    ; jeżeli coś pierdolnie i nie ma vbe to wywala "E"
    mov ah, 0x0e
    mov al, 'E'      
    int 0x10
    jmp $

%include "disk_load.asm"
%include "gdt.asm"
%include "switch_to_pm.asm"

[bits 32]
BEGIN_PM:
    call KERNEL_OFFSET  
    jmp $

BOOT_DRIVE db 0

times 510-($-$$) db 0
dw 0xaa55