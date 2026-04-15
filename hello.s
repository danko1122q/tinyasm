format binary as 'com'
org 100h

start:
    ; 1. Cetak teks "Hello"
    mov dx, msg
    mov ah, 09h
    int 21h

    ; 2. Cetak pesan instruksi biar user gak bingung
    mov dx, msg_pause
    mov ah, 09h
    int 21h

    ; 3. TUNGGU TOMBOL (Ini yang bikin gak langsung close)
    mov ah, 00h     ; Fungsi 00h: Read keyboard character
    int 16h         ; BIOS Keyboard Service

    ; 4. Keluar
    mov ax, 4C00h
    int 21h

; Data
msg       db 'Hello, ini adalah file .com asli!', 0Dh, 0Ah, '$'
msg_pause db 'Tekan tombol apa saja untuk keluar...', '$'