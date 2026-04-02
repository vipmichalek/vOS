[bits 32]

extern _keyboard_handler_c
global _keyboard_handler_asm

; jak nie daję tego section .text to mi irq nie działa
section .text

_keyboard_handler_asm:
    pushad
    call _keyboard_handler_c
    
    mov al, 0x20
    out 0x20, al
    
    popad
    iret
