load_kernel:
    mov bx, KERNEL_OFFSET ; Gdzie zapisać dane
    mov dh, 0x2F             ; to są sektory
    mov dl, [BOOT_DRIVE]
    
    mov ah, 0x02 ; czytaj
    mov al, dh
    mov ch, 0x00 ; cylinder 0
    mov dh, 0x00 ; głowica 0
    mov cl, 0x02 ; zaczyna od drugiego
    int 0x13
    ret