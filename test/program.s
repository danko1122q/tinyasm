format binary
org 0x7C00

; --- CETAK BIRU FORMAT .TIN v2 ---
u8 'TI'                ; [00] Signature
u8 start - $$          ; [02] Offset ke kode
u8 00000101b           ; [03] Flags: (1=Bootable, 0=Reserved, 1=VGA)
u16 0x9000              ; [04] Stack Pointer awal
u8 "DAVA-ARCH", 0      ; [06] Metadata Author

start:
    ; 1. Baca Flags dari Header sendiri (0x7C03)
    mov al, [0x7C03]
    test al, 00000100b ; Cek apakah flag VGA aktif?
    jz no_vga
    
    ; 2. Jika aktif, masuk ke Mode VGA
    mov ax, 0x13
    int 0x10

    ; 3. Gambar Kotak Menggunakan Stack dari Header
    mov sp, [0x7C04]   ; Set stack otomatis dari cetak biru
    
    mov ax, 0xA000
    mov es, ax
    mov di, 320*50 + 100
    mov al, 0x0E       ; Warna Kuning
    mov cx, 50
.loop:
    stosb
    loop .loop

    jmp $

no_vga:
    mov ah, 0x0E
    mov al, 'E'
    int 0x10
    hlt

; --- FOOTER ---
times 510-($-$$) db 0
u16 0xAA55