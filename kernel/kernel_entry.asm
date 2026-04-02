[bits 32]
[extern _main]   
global _start
global _memory_copy

section .text
_start:
    mov esp, 0x7FFFFF
    and esp, 0xFFFFFFF0
    call _main       
    jmp $       
_memory_copy:
    push ebp
    mov ebp, esp
    push esi
    push edi
    push ecx

    mov esi, [ebp + 8]
    mov edi, [ebp + 12]   
    mov ecx, [ebp + 16]  
    
    shr ecx, 2            
    
    cld                   
    rep movsd             

    mov ecx, [ebp + 16]
    and ecx, 3
    rep movsb           

    pop ecx
    pop edi
    pop esi
    pop ebp
    ret
