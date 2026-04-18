# Macro Cookbook

Useful macro patterns for tinyasm — from the basics up to more advanced things like struct emulation and inline strings.

---

## Table of Contents

- [Macro Basics Recap](#macro-basics-recap)
- [Struct Emulation](#struct-emulation)
- [Enum / Named Constants](#enum--named-constants)
- [Scoped Locals & Stack Frames](#scoped-locals--stack-frames)
- [Inline Strings](#inline-strings)
- [Debug Helpers](#debug-helpers)
- [Assertions](#assertions)
- [Conditional Wrappers](#conditional-wrappers)
- [Function Call Helpers](#function-call-helpers)
- [Bit Field Helpers](#bit-field-helpers)
- [Safe Memory Access](#safe-memory-access)
- [Loop Macros](#loop-macros)

---

## Macro Basics Recap

Quick reminder of the syntax before diving in:

```asm
; fixed args
macro name arg1, arg2 {
    ; body
}

; variadic args (square bracket)
macro name [arg] {
    forward
        ; runs once per argument
    reverse
        ; runs once per argument, backwards
    common
        ; runs once
}

; local labels (unique per invocation)
macro name {
    local .lbl
  .lbl:
    jmp .lbl
}

; conditional inside macro
macro name val {
    if val > 0
        ; ...
    else
        ; ...
    end if
}
```

---

## Struct Emulation

tinyasm doesn't have a `struct` keyword, but you can fake it cleanly with macros.

### Basic Struct Definition

```asm
; Define a struct layout as constants
macro struct name {
    virtual at 0
        name#._base:
}

macro ends {
    end virtual
}

; Define a field inside a struct
macro field type, name {
    name:
        type ?
}
```

Usage:
```asm
; Define POINT struct
POINT.x = 0        ; offset of x: 0
POINT.y = 4        ; offset of y: 4
POINT.size = 8     ; total size

; Or use a cleaner approach with = and $ math:
POINT.x    = 0
POINT.y    = 4
POINT.size = 8
```

### Cleaner Struct Macro

```asm
macro struct_begin name {
    name#._ofs = 0
}

macro struct_field name, fname, fsize {
    name#.fname = name#._ofs
    name#._ofs  = name#._ofs + fsize
}

macro struct_end name {
    name#.size = name#._ofs
}
```

Usage:
```asm
struct_begin POINT
    struct_field POINT, x, 4
    struct_field POINT, y, 4
struct_end POINT

; now you have:
;   POINT.x    = 0
;   POINT.y    = 4
;   POINT.size = 8

; allocate one in data section
section '.data' data readable writeable
    my_point rb POINT.size

; access fields
    mov eax, [my_point + POINT.x]
    mov [my_point + POINT.y], edx
```

### Nested Structs

```asm
struct_begin RECT
    struct_field RECT, left,   4
    struct_field RECT, top,    4
    struct_field RECT, right,  4
    struct_field RECT, bottom, 4
struct_end RECT

struct_begin WINDOW
    struct_field WINDOW, id,     4
    struct_field WINDOW, bounds, RECT.size   ; embed RECT
    struct_field WINDOW, flags,  4
struct_end WINDOW

; access nested field:
; WINDOW.bounds + RECT.top = offset of top inside window
mov eax, [wnd + WINDOW.bounds + RECT.top]
```

### Array of Structs

```asm
    ; array of 10 POINTs
    points rb POINT.size * 10

    ; access points[3].y
    mov eax, [points + POINT.size * 3 + POINT.y]
```

---

## Enum / Named Constants

### Simple Enum

```asm
macro enum_begin {
    _enum_val = 0
}

macro enum_val name {
    name = _enum_val
    _enum_val = _enum_val + 1
}

macro enum_val_n name, n {
    name = n
    _enum_val = n + 1
}
```

Usage:
```asm
enum_begin
    enum_val  COLOR_RED
    enum_val  COLOR_GREEN
    enum_val  COLOR_BLUE
    enum_val_n COLOR_ALPHA, 255

; COLOR_RED = 0, COLOR_GREEN = 1, COLOR_BLUE = 2, COLOR_ALPHA = 255
```

### Bitmask Enum

```asm
macro flags_begin {
    _flag_bit = 0
}

macro flag_val name {
    name = 1 shl _flag_bit
    _flag_bit = _flag_bit + 1
}
```

Usage:
```asm
flags_begin
    flag_val  FLAG_READABLE
    flag_val  FLAG_WRITEABLE
    flag_val  FLAG_EXECUTABLE
    flag_val  FLAG_CACHED

; FLAG_READABLE   = 1  (bit 0)
; FLAG_WRITEABLE  = 2  (bit 1)
; FLAG_EXECUTABLE = 4  (bit 2)
; FLAG_CACHED     = 8  (bit 3)

    mov eax, FLAG_READABLE or FLAG_EXECUTABLE    ; = 5
```

---

## Scoped Locals & Stack Frames

### Standard Stack Frame

```asm
; manually set up frame for a function with 3 local dwords
; total local space: 3*4 = 12 bytes, padded to 16 = 16 bytes

MY_FUNC_locals = 16     ; space for locals, 16-byte aligned
local_a = -4            ; offsets from rbp
local_b = -8
local_c = -12

my_func:
    push    rbp
    mov     rbp, rsp
    sub     rsp, MY_FUNC_locals

    mov     dword [rbp + local_a], 0
    mov     dword [rbp + local_b], 0
    mov     dword [rbp + local_c], 0

    ; ... code ...

    mov     rsp, rbp
    pop     rbp
    ret
```

### Macro-Based Function Prologue/Epilogue

```asm
macro proc name, local_bytes {
    name:
    push    rbp
    mov     rbp, rsp
    if local_bytes > 0
        sub     rsp, (local_bytes + 15) and -16    ; round up to 16
    end if
}

macro endproc {
    mov     rsp, rbp
    pop     rbp
    ret
}
```

Usage:
```asm
proc my_function, 32    ; 32 bytes of locals
    mov     dword [rbp - 4], 42
    ; ...
endproc
```

### Saving/Restoring Callee-Saved Registers

```asm
macro save_regs [reg] {
    forward
        push reg
}

macro restore_regs [reg] {
    reverse
        pop reg
}
```

Usage:
```asm
my_func:
    save_regs rbx, r12, r13, r14
    ; ... use rbx, r12, r13, r14 freely ...
    restore_regs rbx, r12, r13, r14
    ret
```

---

## Inline Strings

### Emit a String at Call Site and Jump Over It

```asm
macro inline_str label {
    jmp     .past_#label
    label:
    .past_#label:
}
```

Usage:
```asm
    inline_str greeting
        u8 'Hello, World!', 0
    ; execution continues here, 'greeting' label points to the string
    lea     rsi, [greeting]
```

### Counted String Inline

```asm
macro cstr label, text {
    jmp .after_#label
    label:
        u8 text, 0
    label#_len = $ - label - 1
  .after_#label:
}
```

Usage:
```asm
    cstr my_msg, 'Operation failed'
    ; my_msg     = pointer to string
    ; my_msg_len = 16 (length without null)
```

### String Table

```asm
; Emit a table of null-terminated strings + an index table
; (just data, not a macro — but a useful pattern)

section '.rdata' data readable

strings_base:
    str0:  u8 'first string', 0
    str1:  u8 'second string', 0
    str2:  u8 'third string', 0

string_table:
    u32 RVA str0
    u32 RVA str1
    u32 RVA str2

string_count = ($ - string_table) / 4
```

---

## Debug Helpers

### Print a String Literal (Windows, inline)

```asm
macro dbg_print text {
    local .msg, .msg_end, .hOut, .written
    jmp     .skip_#.msg

    .msg:        u8 text, 13, 10
    .msg_end:

  .skip_#.msg:
    push    rax
    push    rcx
    push    rdx
    push    r8
    push    r9
    push    r10
    push    r11
    sub     rsp, 28h

    mov     ecx, 0FFFFFFF5h         ; STD_OUTPUT_HANDLE
    call    near qword [GetStdHandle]
    mov     ecx, eax
    lea     rdx, [.msg]
    mov     r8d, .msg_end - .msg
    lea     r9, [.written]
    mov     qword [rsp+20h], 0
    call    near qword [WriteFile]

    add     rsp, 28h
    pop     r11
    pop     r10
    pop     r9
    pop     r8
    pop     rdx
    pop     rcx
    pop     rax

    jmp     .after_print_#.msg
    .written: u32 ?
    .after_print_#.msg:
}
```

Usage:
```asm
    dbg_print 'entering my_function'
    ; ... code ...
    dbg_print 'leaving my_function'
```

### Print a String Literal (Linux, inline)

```asm
macro dbg_print text {
    local .msg, .msg_end
    jmp     .skip_#.msg
    .msg:    u8 text, 10
    .msg_end:
  .skip_#.msg:
    push    rax
    push    rdi
    push    rsi
    push    rdx
    push    rcx
    push    r11
    mov     eax, 1
    mov     edi, 1
    lea     rsi, [.msg]
    mov     edx, .msg_end - .msg
    syscall
    pop     r11
    pop     rcx
    pop     rdx
    pop     rsi
    pop     rdi
    pop     rax
}
```

### Breakpoint (for debuggers)

```asm
macro brk {
    int3
}

macro brk_if_zero reg {
    test    reg, reg
    jnz     .no_brk
    int3
  .no_brk:
}
```

### Register Dump (print eax as hex to stdout, Linux)

```asm
macro dump_eax {
    local .buf
    push    rax
    push    rbx
    push    rcx
    push    rdx
    push    rsi
    push    rdi

    ; convert eax to 8-char hex string in .buf
    lea     rdi, [.buf + 8]
    mov     byte [rdi], 10      ; newline
    dec     rdi
    mov     ecx, 8
    mov     ebx, eax
  .hex_loop:
    mov     eax, ebx
    and     eax, 0Fh
    cmp     al, 10
    jb      .dig
    add     al, 'A' - 10
    jmp     .store
  .dig:
    add     al, '0'
  .store:
    mov     [rdi], al
    dec     rdi
    shr     ebx, 4
    dec     ecx
    jnz     .hex_loop

    mov     eax, 1
    mov     edi, 1
    lea     rsi, [.buf]
    mov     edx, 9
    syscall

    pop     rdi
    pop     rsi
    pop     rdx
    pop     rcx
    pop     rbx
    pop     rax
    jmp     .after_buf_#.buf
    .buf rb 9
    .after_buf_#.buf:
}
```

---

## Assertions

### Compile-Time Assert

Fails assembly if condition is false:

```asm
macro static_assert condition, msg {
    if ~(condition)
        ; force an error — tinyasm will report the line
        err msg
    end if
}
```

Usage:
```asm
    static_assert POINT.size = 8, 'POINT struct size mismatch'
    static_assert PAGE_SIZE = 4096, 'wrong page size'
```

Or just use the built-in `assert`:
```asm
    assert POINT.size = 8
    assert sizeof_header = 64
```

### Runtime Assert (debug build)

```asm
macro assert_nz reg, errmsg {
    if defined DEBUG
        test    reg, reg
        jnz     .ok_#errmsg
        int3    ; break to debugger in DEBUG builds
      .ok_#errmsg:
    end if
}
```

Usage:
```asm
    call    near qword [malloc]
    assert_nz rax, alloc_failed     ; only active when -d DEBUG=1
```

---

## Conditional Wrappers

### Platform Guard

```asm
macro win64_only {
    if ~defined WIN32
        ; not a compile error, but skip on non-Windows builds
        ; (just a no-op wrapper for clarity)
    end if
}
```

More useful — wrapping platform-specific code:
```asm
if defined WIN32

section '.code' code readable executable

platform_write:
    ; Windows WriteFile impl
    ret

else

segment readable executable

platform_write:
    ; Linux write syscall impl
    ret

end if
```

### Debug-Only Code Block

```asm
macro debug_begin { if defined DEBUG }
macro debug_end   { end if }
```

Usage:
```asm
debug_begin
    ; this whole block is only assembled when -d DEBUG=1
    dbg_print 'debug checkpoint'
    int3
debug_end
```

---

## Function Call Helpers

### Win64 Call with Shadow Space

```asm
; call a function with auto shadow space management
macro win_call func {
    sub     rsp, 20h
    call    near qword [func]
    add     rsp, 20h
}

macro win_call_1 func, a {
    mov     ecx, a
    sub     rsp, 20h
    call    near qword [func]
    add     rsp, 20h
}

macro win_call_2 func, a, b {
    mov     ecx, a
    mov     edx, b
    sub     rsp, 20h
    call    near qword [func]
    add     rsp, 20h
}
```

Usage:
```asm
    win_call_1 ExitProcess, 0
    win_call_2 MessageBoxA, 0, 0    ; partial — just an example
```

### Linux Syscall Wrapper

```asm
macro syscall1 num, a {
    mov     eax, num
    mov     edi, a
    syscall
}

macro syscall3 num, a, b, c {
    mov     eax, num
    mov     edi, a
    mov     esi, b
    mov     edx, c
    syscall
}
```

Usage:
```asm
    syscall1 60, 0          ; exit(0)
    syscall3 1, 1, msg, len ; write(1, msg, len)
```

---

## Bit Field Helpers

### Extract a bit field

```asm
; extract bits [hi:lo] from eax into eax
macro bitfield_extract reg, lo, hi {
    local width
    width = hi - lo + 1
    if lo > 0
        shr reg, lo
    end if
    and reg, (1 shl width) - 1
}
```

Usage:
```asm
    mov     eax, 0b11010110
    bitfield_extract eax, 2, 5   ; extract bits [5:2] = 0b0101 = 5
```

### Set a bit field

```asm
macro bitfield_set reg, lo, hi, val {
    local width, mask
    width = hi - lo + 1
    mask  = ((1 shl width) - 1) shl lo
    and reg, ~mask              ; clear the field
    or  reg, (val shl lo)       ; set the new value
}
```

---

## Loop Macros

### Repeat N times with counter

```asm
macro times_n n, body {
    local .i
    .i = n
    repeat n
        body
    end repeat
}
```

Or using `repeat` directly (which tinyasm supports natively):
```asm
    repeat 8
        u8 0            ; emit 8 zero bytes
    end repeat

    repeat 4
        u32 % * 10      ; % = 1,2,3,4 → emit 10, 20, 30, 40
    end repeat
```

### For-each over a list at assemble time

```asm
macro for_each [item] {
    forward
        ; process each item
        u8 item, 0      ; e.g., emit each as a null-terminated byte
}

for_each 1, 2, 3, 4, 5
; emits: 01 00 02 00 03 00 04 00 05 00
```

### Generate a lookup table

```asm
; generate a 256-entry byte table where table[i] = i XOR 0x55
repeat 256
    u8 (% - 1) xor 55h
end repeat

; generate sin table (scaled) — using expression math
; (tinyasm doesn't have float math in expressions, but you can precompute)
```
