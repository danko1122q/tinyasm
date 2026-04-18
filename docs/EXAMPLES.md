# Examples & Snippets

Ready-to-copy code for common tasks in tinyasm. All Windows examples are PE64, all Linux examples are ELF64 unless noted.

---

## Table of Contents

- [Hello World](#hello-world)
- [File I/O](#file-io)
- [String Operations](#string-operations)
- [Command Line Arguments](#command-line-arguments)
- [Common Win32 DLLs](#common-win32-dlls)
- [Console Input](#console-input)
- [Math & Bit Operations](#math--bit-operations)
- [Loops & Control Flow](#loops--control-flow)
- [Linux Syscall Patterns](#linux-syscall-patterns)
- [Memory Operations](#memory-operations)
- [Error Handling](#error-handling)

---

## Hello World

### Windows PE64

```asm
format pe64 console
entry start

STD_OUTPUT_HANDLE = 0FFFFFFF5h

section '.code' code readable executable

start:
    sub     rsp, 28h

    mov     ecx, STD_OUTPUT_HANDLE
    call    near qword [GetStdHandle]
    mov     [hOut], eax

    mov     ecx, [hOut]
    lea     rdx, [msg]
    mov     r8d, msglen
    lea     r9, [written]
    mov     qword [rsp+20h], 0
    call    near qword [WriteFile]

    xor     ecx, ecx
    call    near qword [ExitProcess]

section '.data' data readable writeable
    hOut     u32 ?
    written  u32 ?
    msg      u8 'Hello, World!', 13, 10
    msglen = $ - msg

section '.idata' import data readable
    u32 RVA k32_ilt, 0, 0, RVA k32_name, RVA k32_iat
    u32 0, 0, 0, 0, 0
    k32_ilt:
        u64 RVA _GetStdHandle
        u64 RVA _WriteFile
        u64 RVA _ExitProcess
        u64 0
    k32_iat:
        GetStdHandle  u64 RVA _GetStdHandle
        WriteFile     u64 RVA _WriteFile
        ExitProcess   u64 RVA _ExitProcess
        u64 0
    k32_name u8 'KERNEL32.DLL', 0
    _GetStdHandle  u16 0
        u8 'GetStdHandle', 0
    _WriteFile     u16 0
        u8 'WriteFile', 0
    _ExitProcess   u16 0
        u8 'ExitProcess', 0
```

### Linux ELF64

```asm
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
```

---

## File I/O

### Windows — Read a File

```asm
; Opens a file and reads it into a buffer.
; Returns: eax = bytes read, or 0 on error.
;
; rcx = path (null-terminated string pointer)
; rdx = buffer pointer
; r8d = buffer size

read_file:
    sub     rsp, 48h
    mov     [rsp+30h], rdx          ; save buffer
    mov     [rsp+38h], r8d          ; save size

    ; CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)
    ; rcx = path (already set)
    mov     edx, 80000000h          ; GENERIC_READ
    mov     r8d, 1                  ; FILE_SHARE_READ
    xor     r9d, r9d                ; security attrs = NULL
    mov     dword [rsp+20h], 3      ; OPEN_EXISTING
    mov     dword [rsp+28h], 0      ; flags = 0
    mov     dword [rsp+30h], 0      ; template = NULL  ← careful: overwrites saved rdx!
    ; NOTE: save your buffer pointer somewhere else before this call

    call    near qword [CreateFileA]
    cmp     eax, -1                 ; INVALID_HANDLE_VALUE
    je      .fail

    mov     [file_handle], eax

    ; ReadFile(handle, buffer, size, &bytes_read, NULL)
    mov     ecx, [file_handle]
    mov     rdx, [rsp+30h]          ; buffer
    mov     r8d, [rsp+38h]          ; size
    lea     r9, [bytes_read]
    mov     qword [rsp+20h], 0      ; overlapped = NULL
    call    near qword [ReadFile]

    ; CloseHandle
    mov     ecx, [file_handle]
    call    near qword [CloseHandle]

    mov     eax, [bytes_read]
    add     rsp, 48h
    ret
  .fail:
    xor     eax, eax
    add     rsp, 48h
    ret
```

### Windows — Write a File

```asm
; CreateFileA + WriteFile + CloseHandle
;
; rcx = path
; rdx = data buffer
; r8d = byte count

write_file:
    sub     rsp, 48h
    mov     [rsp+30h], rdx
    mov     [rsp+38h], r8d

    ; CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0x80, NULL)
    mov     edx, 40000000h          ; GENERIC_WRITE
    xor     r8d, r8d                ; share = none
    xor     r9d, r9d                ; security = NULL
    mov     dword [rsp+20h], 2      ; CREATE_ALWAYS
    mov     dword [rsp+28h], 80h    ; FILE_ATTRIBUTE_NORMAL
    mov     dword [rsp+30h], 0      ; NOTE: overwrites saved rdx — save in a register instead
    call    near qword [CreateFileA]
    cmp     eax, -1
    je      .fail
    mov     [file_handle], eax

    ; WriteFile(handle, buffer, count, &written, NULL)
    mov     ecx, [file_handle]
    mov     rdx, [rsp+30h]
    mov     r8d, [rsp+38h]
    lea     r9, [written]
    mov     qword [rsp+20h], 0
    call    near qword [WriteFile]

    mov     ecx, [file_handle]
    call    near qword [CloseHandle]

    add     rsp, 48h
    ret
  .fail:
    add     rsp, 48h
    ret

section '.data' data readable writeable
    file_handle  u32 ?
    bytes_read   u32 ?
    written      u32 ?
```

### Linux — Read a File (syscalls)

```asm
; open + read + close
; rdi = path, rsi = buffer, rdx = count
; returns: rax = bytes read

read_file_linux:
    push    rbx
    push    r12
    push    r13

    mov     r12, rsi        ; save buffer
    mov     r13, rdx        ; save count

    ; open(path, O_RDONLY=0)
    mov     eax, 2
    ; rdi = path already
    xor     esi, esi        ; O_RDONLY
    xor     edx, edx        ; mode (unused for read)
    syscall
    test    eax, eax
    js      .fail
    mov     ebx, eax        ; save fd

    ; read(fd, buffer, count)
    mov     eax, 0
    mov     edi, ebx
    mov     rsi, r12
    mov     rdx, r13
    syscall
    push    rax             ; save bytes read

    ; close(fd)
    mov     eax, 3
    mov     edi, ebx
    syscall

    pop     rax
    pop     r13
    pop     r12
    pop     rbx
    ret
  .fail:
    xor     eax, eax
    pop     r13
    pop     r12
    pop     rbx
    ret
```

### Linux — Write to a File

```asm
; rdi = path, rsi = data, rdx = count

write_file_linux:
    push    rbx
    push    r12
    push    r13
    mov     r12, rsi
    mov     r13, rdx

    ; open(path, O_WRONLY|O_CREAT|O_TRUNC, 0644)
    mov     eax, 2
    mov     esi, 0x241      ; O_WRONLY(1)|O_CREAT(0x40)|O_TRUNC(0x200)
    mov     edx, 0o644
    syscall
    test    eax, eax
    js      .fail
    mov     ebx, eax

    ; write(fd, data, count)
    mov     eax, 1
    mov     edi, ebx
    mov     rsi, r12
    mov     rdx, r13
    syscall

    ; close(fd)
    mov     eax, 3
    mov     edi, ebx
    syscall

    pop     r13
    pop     r12
    pop     rbx
    ret
  .fail:
    pop     r13
    pop     r12
    pop     rbx
    ret
```

---

## String Operations

### String Length (null-terminated)

```asm
; rdi = string pointer
; returns: rax = length (not including null)

strlen:
    xor     eax, eax
  .loop:
    cmp     byte [rdi + rax], 0
    je      .done
    inc     eax
    jmp     .loop
  .done:
    ret
```

### String Copy

```asm
; rdi = dest, rsi = src
; copies until null terminator, including null

strcpy:
  .loop:
    mov     al, [rsi]
    mov     [rdi], al
    inc     rsi
    inc     rdi
    test    al, al
    jnz     .loop
    ret
```

### String Compare

```asm
; rdi = str1, rsi = str2
; returns: eax = 0 if equal, nonzero if not

strcmp:
  .loop:
    mov     al, [rdi]
    mov     bl, [rsi]
    cmp     al, bl
    jne     .notequal
    test    al, al
    jz      .equal
    inc     rdi
    inc     rsi
    jmp     .loop
  .equal:
    xor     eax, eax
    ret
  .notequal:
    movsx   eax, al
    movsx   ebx, bl
    sub     eax, ebx
    ret
```

### Integer to String (decimal)

```asm
; eax = unsigned integer to convert
; rdi = output buffer (at least 11 bytes for u32)
; returns: rdi points past the last digit (not null-terminated)

u32_to_dec:
    push    rbx
    push    rsi
    mov     esi, edi        ; save start
    add     edi, 10         ; work backwards from end
    mov     byte [rdi], 0   ; null terminate
    dec     edi
    mov     ebx, 10
  .digit:
    xor     edx, edx
    div     ebx             ; eax = quotient, edx = remainder
    add     dl, '0'
    mov     [rdi], dl
    dec     edi
    test    eax, eax
    jnz     .digit
    inc     edi             ; edi now points to first digit
    ; reverse the string from edi to esi+10
    pop     rsi
    pop     rbx
    ret
```

### Hex Dump (one byte)

```asm
; dl = byte to convert to hex
; rdi = output (2 bytes written, not null-terminated)

byte_to_hex:
    mov     al, dl
    shr     al, 4
    call    .nibble
    mov     al, dl
    and     al, 0Fh
    call    .nibble
    ret
  .nibble:
    cmp     al, 10
    jb      .digit
    add     al, 'A' - 10
    mov     [rdi], al
    inc     rdi
    ret
  .digit:
    add     al, '0'
    mov     [rdi], al
    inc     rdi
    ret
```

---

## Command Line Arguments

### Windows — Parsing argv via GetCommandLineA

```asm
; Windows doesn't pass argc/argv through registers.
; You call GetCommandLineA to get the raw command line string,
; then parse it yourself — or use GetCommandLine + CommandLineToArgvW.

start:
    sub     rsp, 28h

    call    near qword [GetCommandLineA]
    ; rax = pointer to command line string
    ; format: "program.exe arg1 arg2 ..."
    mov     [cmdline], eax

    ; simple: skip past the program name (first token)
    mov     esi, eax
  .skip_prog:
    mov     al, [esi]
    inc     esi
    test    al, al
    jz      .no_args
    cmp     al, ' '
    jne     .skip_prog
    ; esi now points past the first space — first argument starts here
    ; ...

  .no_args:
    xor     ecx, ecx
    call    near qword [ExitProcess]

section '.data' data readable writeable
    cmdline  u32 ?
```

### Linux — argc/argv from stack

On Linux, at program entry the stack layout is:
```
[rsp]     = argc
[rsp+8]   = argv[0]  (program name)
[rsp+16]  = argv[1]
...
[rsp+8*(argc+1)] = NULL
```

```asm
format elf64 executable 3
entry _start

segment readable executable

_start:
    mov     eax, [rsp]          ; argc
    lea     rsi, [rsp+8]        ; argv

    ; print each argument
    mov     ecx, eax
  .loop:
    test    ecx, ecx
    jz      .done
    mov     rdi, [rsi]          ; argv[i]
    ; find length
    push    rsi
    push    rcx
    mov     rsi, rdi
    xor     edx, edx
  .len:
    cmp     byte [rsi + rdx], 0
    je      .print
    inc     edx
    jmp     .len
  .print:
    ; write(1, argv[i], len)
    mov     eax, 1
    mov     edi, 1
    ; rsi = arg pointer, rdx = length
    syscall
    ; newline
    mov     eax, 1
    mov     edi, 1
    lea     rsi, [nl]
    mov     edx, 1
    syscall
    pop     rcx
    pop     rsi
    add     rsi, 8
    dec     ecx
    jmp     .loop
  .done:
    mov     eax, 60
    xor     edi, edi
    syscall

segment readable
nl u8 10
```

---

## Common Win32 DLLs

### msvcrt.dll — C Runtime (printf, malloc, etc.)

```asm
section '.idata' import data readable

    ; kernel32
    u32 RVA k32_ilt, 0, 0, RVA k32_name, RVA k32_iat
    ; msvcrt
    u32 RVA crt_ilt, 0, 0, RVA crt_name, RVA crt_iat
    u32 0, 0, 0, 0, 0

    k32_ilt:
        u64 RVA _ExitProcess
        u64 0
    k32_iat:
        ExitProcess    u64 RVA _ExitProcess
        u64 0
    k32_name u8 'KERNEL32.DLL', 0
    _ExitProcess u16 0
        u8 'ExitProcess', 0

    crt_ilt:
        u64 RVA _printf
        u64 RVA _malloc
        u64 RVA _free
        u64 RVA _strlen
        u64 0
    crt_iat:
        printf         u64 RVA _printf
        malloc         u64 RVA _malloc
        free           u64 RVA _free
        strlen         u64 RVA _strlen
        u64 0
    crt_name u8 'MSVCRT.DLL', 0
    _printf  u16 0
        u8 'printf', 0
    _malloc  u16 0
        u8 'malloc', 0
    _free    u16 0
        u8 'free', 0
    _strlen  u16 0
        u8 'strlen', 0
```

Usage:
```asm
    ; printf("Value: %d\n", 42)
    sub     rsp, 28h
    lea     rcx, [fmt]
    mov     edx, 42
    call    near qword [printf]
    add     rsp, 28h

section '.data' data readable writeable
    fmt u8 'Value: %d', 13, 10, 0
```

### user32.dll — MessageBox

```asm
    ; MessageBoxA(NULL, text, caption, MB_OK)
    sub     rsp, 28h
    xor     ecx, ecx            ; hWnd = NULL
    lea     rdx, [msg_text]
    lea     r8, [msg_title]
    xor     r9d, r9d            ; MB_OK = 0
    call    near qword [MessageBoxA]
    add     rsp, 28h
```

Import entry:
```asm
    u32_ilt:
        u64 RVA _MessageBoxA
        u64 0
    u32_iat:
        MessageBoxA    u64 RVA _MessageBoxA
        u64 0
    u32_name u8 'USER32.DLL', 0
    _MessageBoxA u16 0
        u8 'MessageBoxA', 0
```

### VirtualAlloc — Allocate Memory

```asm
    ; VirtualAlloc(NULL, size, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE)
    MEM_COMMIT    = 1000h
    MEM_RESERVE   = 2000h
    PAGE_READWRITE = 4

    sub     rsp, 28h
    xor     ecx, ecx                    ; lpAddress = NULL
    mov     edx, 65536                  ; dwSize = 64KB
    mov     r8d, MEM_COMMIT + MEM_RESERVE
    mov     r9d, PAGE_READWRITE
    call    near qword [VirtualAlloc]
    add     rsp, 28h
    ; rax = allocated pointer, or NULL on failure
    mov     [my_buffer], eax
```

---

## Console Input

### Windows — ReadConsole

```asm
    STD_INPUT_HANDLE = 0FFFFFFF6h

    ; get stdin handle
    sub     rsp, 28h
    mov     ecx, STD_INPUT_HANDLE
    call    near qword [GetStdHandle]
    mov     [hIn], eax

    ; ReadConsoleA(hIn, buffer, maxChars, &charsRead, NULL)
    mov     ecx, [hIn]
    lea     rdx, [input_buf]
    mov     r8d, 256
    lea     r9, [chars_read]
    mov     qword [rsp+20h], 0
    call    near qword [ReadConsoleA]
    add     rsp, 28h
    ; input_buf now contains the input, chars_read = count

section '.data' data readable writeable
    hIn        u32 ?
    input_buf  rb 256
    chars_read u32 ?
```

Import for `ReadConsoleA`:
```asm
    _ReadConsoleA u16 0
        u8 'ReadConsoleA', 0
```

### Linux — read from stdin

```asm
    ; read(0, buffer, 255)
    mov     eax, 0
    xor     edi, edi        ; fd = 0 (stdin)
    lea     rsi, [buf]
    mov     edx, 255
    syscall
    ; rax = bytes read

segment readable writeable
buf rb 256
```

---

## Math & Bit Operations

### Multiply without MUL

```asm
    ; eax * 5
    lea     eax, [eax + eax*4]

    ; eax * 3
    lea     eax, [eax + eax*2]

    ; eax * 10
    lea     eax, [eax + eax*4]
    add     eax, eax
```

### Divide by power of 2

```asm
    shr     eax, 3      ; eax / 8 (unsigned)
    sar     eax, 3      ; eax / 8 (signed)
```

### Check if power of 2

```asm
    ; eax = value to test
    ; result: ZF set if power of 2 (including 0 — check separately)
    mov     ebx, eax
    dec     ebx
    test    eax, ebx    ; if (n & (n-1)) == 0, it's a power of 2
    jz      .is_power_of_2
```

### Bit scan (find highest set bit)

```asm
    bsr     eax, ecx    ; eax = index of highest set bit in ecx
    ; ZF set if ecx was 0

    bsf     eax, ecx    ; eax = index of lowest set bit in ecx
```

### Popcount (count set bits)

```asm
    popcnt  eax, ecx    ; eax = number of set bits in ecx
    ; requires SSE4.2 — check CPUID if targeting old hardware
```

### Rotate

```asm
    rol     eax, 8      ; rotate left by 8 bits
    ror     eax, 8      ; rotate right by 8 bits
    rol     eax, cl     ; rotate by amount in cl
```

---

## Loops & Control Flow

### Counted loop with `ecx`

```asm
    mov     ecx, 10
  .loop:
    ; ... loop body ...
    dec     ecx
    jnz     .loop
```

### Loop with index

```asm
    xor     eax, eax        ; i = 0
  .loop:
    cmp     eax, 10
    jge     .done
    ; ... body using eax as index ...
    inc     eax
    jmp     .loop
  .done:
```

### Loop over array

```asm
    lea     rsi, [arr]
    mov     ecx, arr_count
  .loop:
    mov     eax, [rsi]      ; load element
    ; ... process eax ...
    add     rsi, 4          ; next dword
    dec     ecx
    jnz     .loop

section '.data' data readable writeable
    arr       u32 10, 20, 30, 40, 50
    arr_count = ($ - arr) / 4
```

### String loop (until null)

```asm
    lea     rsi, [str]
  .loop:
    lodsb                   ; al = [rsi], rsi++
    test    al, al
    jz      .done
    ; ... process al ...
    jmp     .loop
  .done:
```

### Switch/case via jump table

```asm
    ; eax = case value (0-3)
    cmp     eax, 3
    ja      .default
    lea     rbx, [jump_table]
    jmp     qword [rbx + rax*8]

  .case0:
    ; ...
    jmp     .end
  .case1:
    ; ...
    jmp     .end
  .case2:
    ; ...
    jmp     .end
  .case3:
    ; ...
    jmp     .end
  .default:
    ; ...
  .end:

section '.data' data readable
    jump_table:
        u64 .case0
        u64 .case1
        u64 .case2
        u64 .case3
```

---

## Linux Syscall Patterns

### Exit with code

```asm
    mov     eax, 60
    mov     edi, exit_code
    syscall
```

### Write to stdout / stderr

```asm
    mov     eax, 1          ; sys_write
    mov     edi, 1          ; 1=stdout, 2=stderr
    lea     rsi, [msg]
    mov     edx, len
    syscall
```

### mmap — allocate memory

```asm
    ; mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0)
    PROT_READ   = 1
    PROT_WRITE  = 2
    MAP_PRIVATE = 2
    MAP_ANON    = 20h

    xor     edi, edi                    ; addr = NULL
    mov     esi, 4096                   ; length
    mov     edx, PROT_READ + PROT_WRITE
    mov     r10d, MAP_PRIVATE + MAP_ANON
    mov     r8d, -1                     ; fd = -1
    xor     r9d, r9d                    ; offset = 0
    mov     eax, 9                      ; sys_mmap
    syscall
    ; rax = mapped address, or negative errno on error
```

### getpid

```asm
    mov     eax, 39         ; sys_getpid
    syscall
    ; rax = PID
```

### nanosleep (sleep for N seconds)

```asm
    ; struct timespec: u64 seconds, u64 nanoseconds
    lea     rdi, [sleep_time]
    xor     rsi, rsi        ; no remainder struct
    mov     eax, 35         ; sys_nanosleep
    syscall

segment readable writeable
sleep_time:
    u64 1       ; 1 second
    u64 0       ; 0 nanoseconds
```

---

## Memory Operations

### memset — fill memory

```asm
    ; rdi = dest, al = fill byte, ecx = count
memset:
    push    rdi
    rep     stosb
    pop     rax             ; return original dest
    ret
```

### memcpy — copy memory

```asm
    ; rdi = dest, rsi = src, ecx = count (bytes)
memcpy:
    push    rdi
    rep     movsb
    pop     rax
    ret
```

### memcpy aligned dwords (faster for large copies)

```asm
    ; rdi = dest, rsi = src, ecx = count (must be multiple of 4)
memcpy_dwords:
    push    rdi
    shr     ecx, 2          ; count / 4
    rep     movsd
    pop     rax
    ret
```

### memcmp — compare memory

```asm
    ; rdi = ptr1, rsi = ptr2, ecx = count
    ; returns: eax = 0 if equal
memcmp:
    repe    cmpsb
    je      .equal
    movzx   eax, byte [rdi-1]
    movzx   ebx, byte [rsi-1]
    sub     eax, ebx
    ret
  .equal:
    xor     eax, eax
    ret
```

---

## Error Handling

### Windows — GetLastError

```asm
    call    near qword [SomeFunc]
    test    eax, eax
    jnz     .ok

    ; get error code
    sub     rsp, 28h
    call    near qword [GetLastError]
    add     rsp, 28h
    ; eax = error code

    ; format error message
    ; ... FormatMessageA / write to stderr ...

  .ok:
```

### Windows — Write to stderr

```asm
    STD_ERROR_HANDLE = 0FFFFFFF4h

    sub     rsp, 28h
    mov     ecx, STD_ERROR_HANDLE
    call    near qword [GetStdHandle]
    mov     [hErr], eax

    mov     ecx, [hErr]
    lea     rdx, [err_msg]
    mov     r8d, err_msglen
    lea     r9, [written]
    mov     qword [rsp+20h], 0
    call    near qword [WriteFile]
    add     rsp, 28h
```

### Linux — Write error to stderr

```asm
    mov     eax, 1
    mov     edi, 2          ; fd = 2 (stderr)
    lea     rsi, [err_msg]
    mov     edx, err_msglen
    syscall
```

### Linux — Exit with error code from errno

```asm
    ; after a failed syscall, rax contains -errno
    neg     eax             ; eax = errno
    mov     edi, eax
    mov     eax, 60         ; sys_exit
    syscall
```
