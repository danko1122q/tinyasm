; hello.asm
org 100h

start:
    mov ah, 01h
    int 21h
    
    cmp al, 'q'
    je exit
    
    xchg dx, ax
    mov ah, 02h
    int 21h
    jmp start

exit:
    ret