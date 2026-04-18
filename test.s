format elf64 executable 3
entry _start

segment readable executable

_start:
    mov eax, 1
    mov edi, 1
    lea rsi, [msg]
    mov edx, msglen
    syscall

    mov eax, 60
    xor edi, edi
    syscall

segment readable

msg     u8 'Hello, World!', 10
msglen = $ - msg