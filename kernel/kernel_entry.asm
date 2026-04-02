[bits 32]
[extern main]   
global start
global memory_copy

section .text
start:
    mov esp, 0x7FFFFF
    and esp, 0xFFFFFFF0
    call main       
    jmp $       
memory_copy:
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