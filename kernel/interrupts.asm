[bits 32]

extern keyboard_handler_c
global keyboard_handler_asm
global timer_handler_asm
extern timer_handler_c

; jak nie daję tego section .text to mi irq nie działa
section .text

keyboard_handler_asm:
    pushad
    call keyboard_handler_c
    mov al, 0x20
    out 0x20, al
    popad
    iret

timer_handler_asm:
    pusha
    call timer_handler_c
    popa
    iretd