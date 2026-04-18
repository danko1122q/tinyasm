# tinyasm Technical Reference

> A no-fluff deep-dive into writing real programs with tinyasm — syntax, directives, PE64/ELF structure, macros, and all the stuff README.md doesn't cover.


---

## Table of Contents

- [How tinyasm Actually Works](#how-tinyasm-actually-works)
- [CLI Options](#cli-options)
- [Output Formats](#output-formats)
- [Source File Structure](#source-file-structure)
- [Data Directives](#data-directives)
- [Labels & Symbols](#labels--symbols)
- [Expressions & Operators](#expressions--operators)
- [Macros](#macros)
- [Preprocessor Directives](#preprocessor-directives)
- [Sections & Segments](#sections--segments)
- [Writing a PE64 Windows Program](#writing-a-pe64-windows-program)
- [Writing an ELF64 Linux Program](#writing-an-elf64-linux-program)
- [Writing a Raw Binary](#writing-a-raw-binary)
- [Win64 ABI Cheatsheet](#win64-abi-cheatsheet)
- [Linux Syscall Cheatsheet](#linux-syscall-cheatsheet)
- [Error Reference](#error-reference)
- [What core/*.tny Actually Is](#what-coretny-actually-is)

---

## How tinyasm Actually Works

tinyasm is a **multi-pass assembler** — it re-scans the source up to 100 times (configurable) until all forward references resolve. It outputs the final binary directly: no linker, no intermediate object files. Everything — code, data, import tables, ELF/PE headers — goes into the output in a single shot.

This is why writing a Windows PE64 program feels verbose: you're doing what a linker would normally do for you.

**Build pipeline:**
```
source.s  →  [pass 1]  →  [pass 2]  →  ...  →  [pass N]  →  output binary
              (symbols undefined ok)   (resolve forward refs)   (all resolved)
```

Exit code = number of passes performed. A clean 2-pass build exits with `2`. Scripts should check for output file existence, not exit code, to determine success.

---

## CLI Options

```
./tiny [options] <source.s> [output]
```

| Flag | Description |
|------|-------------|
| `-m <kb>` | Memory limit in KB (default: 16384 = 16 MB)  |
| `-p <n>` | Max passes (default: 100) |
| `-d <name>=<value>` | Define a symbol from the command line |
| `-s <file>` | Dump symbol table to a file after assembly |
| `-i <path>` | Add directory to include search path |

Flags are case-insensitive: `-M` and `-m` are the same.

**Output filename** — if you omit it, tinyasm derives it from the source name + format directive:

| Format | Auto extension |
|--------|---------------|
| `format elf64 executable` | *(no extension)* |
| `format elf64` (object) | `.o` |
| `format pe64` (exe) | `.exe` |
| `format pe64` (dll) | `.dll` |
| `format pe64` (driver, subsystem 1) | `.sys` |
| `format pe64` (EFI, subsystem ≥ 10) | `.efi` |
| `format binary` | *(source extension replaced, nothing appended)* |

**Include search order:**
1. Directory of the file containing the `include`
2. Paths from `TNYASM_INCLUDE` env variable (semicolon-separated)
3. Paths from `-i` flags (in order given)

**Examples:**
```sh
# basic
./tiny hello.s hello.exe

# with defines and include paths
./tiny -d WIN32=1 -d VERSION=2 -i ./lib -i ./arch main.s main.exe

# let tinyasm pick the output name
./tiny hello.s

# dump symbols
./tiny -s symbols.txt main.s main

# increase memory for large projects
./tiny -m 65536 big.s big
```

---

## Output Formats

The `format` directive must be the **first line** of your source file, and must be lowercase.

```asm
format elf64 executable 3      ; Linux 64-bit executable
format elf executable 3        ; Linux 32-bit executable
format elf64                   ; Linux 64-bit object file
format elf                     ; Linux 32-bit object file
format pe64 console 5.0        ; Windows 64-bit console app
format pe64 GUI 5.0            ; Windows 64-bit GUI app (no console window)
format pe console 5.0          ; Windows 32-bit console app
format pe64 dll 5.0            ; Windows 64-bit DLL
format binary                  ; raw flat binary, zero headers
```

> **Note on `GUI` subsystem:** this just tells Windows not to allocate a console window — it doesn't give you any UI framework. You still wire up all Win32 GUI calls manually.

> **Note on version numbers:** `5.0` in `pe64 console 5.0` is the minimum Windows version (5.0 = Windows 2000+). You can omit it and tinyasm will use a default.

---

## Source File Structure

### Linux ELF64

```asm
format elf64 executable 3
entry _start

segment readable executable

_start:
    mov eax, 1          ; sys_write
    mov edi, 1          ; stdout
    lea rsi, [msg]
    mov edx, msglen
    syscall

    mov eax, 60         ; sys_exit
    xor edi, edi
    syscall

segment readable

msg     u8 'Hello, World', 10
msglen = $ - msg
```

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
    mov     [hStdOut], eax

    mov     ecx, [hStdOut]
    lea     rdx, [msg]
    mov     r8d, msglen
    lea     r9, [written]
    mov     qword [rsp+20h], 0
    call    near qword [WriteFile]

    xor     ecx, ecx
    call    near qword [ExitProcess]

section '.data' data readable writeable
    hStdOut  u32 ?
    written  u32 ?
    msg      u8 'Hello, World', 13, 10
    msglen = $ - msg

section '.idata' import data readable
    ; ... import table (see PE64 section below)
```

### Raw Binary (Bootloader)

```asm
format binary
org 0x7C00

start:
    mov ah, 0x0E
    mov al, 'H'
    int 0x10
    jmp $

times 510-($-$$) u8 0
u16 0xAA55
```

---

## Data Directives

tinyasm uses its own data directives (`u8`, `u16`, `u32`, etc.) — **not** NASM/FASM's `db/dw/dd/dq/dt`. Those directives do not exist in tinyasm and will cause `unknown instruction` errors.

### Defining Data (`u*`)

| Directive | Size | Example |
|-----------|------|---------|
| `u8` | 1 byte | `u8 0x41, 'A', "hello"` |
| `u16` | 2 bytes | `u16 1234, 0xFF00` |
| `u32` | 4 bytes | `u32 0xDEADBEEF` |
| `u64` | 8 bytes | `u64 0x123456789ABCDEF0` |
| `u80` | 10 bytes | `u80 0x1234567890ABCDEF0011` |
| `u128` | 16 bytes | `u128 0x0` |
| `u256` | 32 bytes | `u256 0x0` |
| `u512` | 64 bytes | `u512 0x0` |

String literals go inside single or double quotes:
```asm
msg u8 'Hello, World', 13, 10, 0   ; string + CRLF + null terminator
msg u8 "Hello", 0                  ; double quotes also work
```

Mixing values and strings in one line:
```asm
u8 'TI'             ; 2 bytes: 0x54, 0x49
u8 0x00, 0xFF, 'A'  ; mixed bytes
```

### Reserving Uninitialized Space (`r*`)

These reserve space without initializing it (like BSS):

| Directive | Reserves |
|-----------|----------|
| `rb <n>` | n bytes |
| `rw <n>` | n × 2 bytes |
| `rd <n>` | n × 4 bytes |
| `rq <n>` | n × 8 bytes |

```asm
buffer    rb 256        ; reserve 256 bytes
table     rd 64         ; reserve 64 dwords (256 bytes)
```

### Uninitialized Single Variable (`?`)

```asm
hStdOut  u32 ?    ; one uninitialized dword
written  u32 ?
flag     u8 ?
```

### `times` / `repeat` — Repeat Data

```asm
times 510-($-$$) u8 0   ; fill to offset 510 with zeros (bootloader padding)

; or use repeat block:
repeat 16
    u8 0
end repeat
```

### `$` and `$$`

| Symbol | Meaning |
|--------|---------|
| `$` | Current address (position in output) |
| `$$` | Start address of current section/segment |

```asm
msg     u8 'Hello'
msglen = $ - msg        ; length of msg = 5
```

### `align`

```asm
align 4     ; pad to next 4-byte boundary
align 16    ; pad to next 16-byte boundary
```

---

## Labels & Symbols

### Regular Labels

```asm
my_label:
    mov eax, 1
    jmp my_label
```

### Anonymous Labels

```asm
@@:                 ; anonymous label
    dec ecx
    jnz @b          ; jump back to nearest @@
    jmp @f          ; jump forward to nearest @@
@@:
    ret
```

`@b` = look backward, `@f` = look forward.

### Local Labels (inside macros)

```asm
macro my_loop count {
    local .start, .end
    mov ecx, count
  .start:
    dec ecx
    jnz .start
}
```

Labels prefixed with `.` inside a macro are locally scoped.

### Constant Definitions (`=` and `equ`)

```asm
; numeric constant (evaluated immediately)
PAGE_SIZE = 4096
STD_OUTPUT_HANDLE = 0FFFFFFF5h

; equ — text substitution (lazy, like a macro)
CRLF equ 13, 10
msg u8 'Hello', CRLF, 0
```

The difference: `=` creates a numeric symbol. `equ` does text-level substitution — it can expand to anything including commas, register names, etc.

### `RVA` — Relative Virtual Address

Used exclusively in PE import/export tables:
```asm
u32 RVA kernel32_name    ; RVA to a label in the PE image
u64 RVA _GetStdHandle    ; 64-bit RVA entry for IAT
```

---

## Expressions & Operators

### Numeric Literals

```asm
255         ; decimal
0xFF        ; hex (C style)
0FFh        ; hex (Intel style)
11111111b   ; binary
0377o       ; octal
```

### Arithmetic

```asm
mov eax, 4 * 1024          ; 4096
mov eax, (offset + 3) and -4   ; align down to 4
mov eax, 1 shl 8           ; 256
mov eax, value xor 0xFF
```

| Operator | Meaning |
|----------|---------|
| `+` `-` `*` `/` | basic arithmetic |
| `mod` | modulo |
| `shl` `shr` | bit shift |
| `and` `or` `xor` `not` | bitwise |
| `=` `<>` `<` `>` `<=` `>=` | comparison (returns 0 or -1) |

### Size Overrides

```asm
mov byte [ebx], 0       ; explicit byte
mov word [ebx], 0       ; explicit word
mov dword [ebx], 0      ; explicit dword
mov qword [rbx], 0      ; explicit qword
```

---

## Macros

Macros in tinyasm are powerful — they're closer to FASM macros than NASM's `%macro`. They support variadic args, pattern matching, and nested scoping.

### Basic Macro

```asm
macro zero_reg reg {
    xor reg, reg
}

zero_reg eax    ; expands to: xor eax, eax
zero_reg rbx    ; expands to: xor rbx, rbx
```

### Multiple Parameters

```asm
macro mov32_mem addr, val {
    mov dword [addr], val
}

mov32_mem [myVar], 42
```

### Variadic Macros (`[arg]`)

Square brackets make the last parameter variadic — it collects all remaining arguments:

```asm
macro push_all [reg] {
    common
        ; 'common' block runs once for all args
    forward
        push reg    ; 'forward' block runs once per argument
}

push_all rax, rbx, rcx
; expands to: push rax / push rbx / push rcx
```

**Iteration sections inside variadic macros:**

| Keyword | Runs |
|---------|------|
| `common` | once, before all args |
| `forward` | once per arg, left-to-right |
| `reverse` | once per arg, right-to-left |

```asm
macro push_all [reg] {
    forward
        push reg
}

macro pop_all [reg] {
    reverse
        pop reg     ; pops in reverse order — correct for stack discipline
}

push_all rax, rbx, rcx     ; push rax, rbx, rcx
pop_all  rax, rbx, rcx     ; pop rcx, rbx, rax  ✓
```

### `local` — Local Labels in Macros

```asm
macro spin_until_zero reg {
    local .loop
  .loop:
    test reg, reg
    jnz .loop
}

spin_until_zero ecx   ; each call gets its own unique .loop label
spin_until_zero eax   ; no collision
```

### `if` Inside Macros

```asm
macro safe_mov dst, src {
    if dst eq src
        ; do nothing — same register
    else
        mov dst, src
    end if
}
```

### Condition operators for `if` inside macros

| Operator | Meaning |
|----------|---------|
| `eq` | symbol equality |
| `eqtype` | same type (e.g., both registers) |
| `defined <sym>` | symbol is defined |
| `<` `>` `=` | numeric comparison |

### Overriding Instructions with Macros

tinyasm lets you redefine existing mnemonics. `platform.tny` does this to bridge 32/64-bit addressing:

```asm
macro mov dest, src {
    if src eq esp
        mov dest, ESP   ; promote 32-bit ESP to full RSP value
    else
        mov dest, src
    end if
}
```

> **Watch out:** once you redefine `mov`, every `mov` in scope goes through your macro. Use `purge mov` to remove your override if needed.

### `purge` — Remove a Macro

```asm
purge mov       ; restore original mov instruction
purge push, pop ; restore multiple at once
```

### Nested Macros

Inside a macro body, backslashes escape the nesting level:

```asm
macro outer {
    macro inner arg \{
        mov eax, arg
    \}
}
```

Each nesting level needs one more backslash per `{` and `}`.

### `irps` — Iterate Over Token List

```asm
macro init_regs [reg] {
    forward
        irps r, reg {
            xor r, r
        }
}
```

### `match` — Pattern Matching

```asm
macro try_mov dst, src {
    match =dword, src {
        ; src is literally the token 'dword'
    }
    match [mem], src {
        ; src is a memory reference like [ebx]
    }
}
```

---

## Preprocessor Directives

### `include`

```asm
include 'mylib.tny'
include 'arch/x86_extras.tny'
```

Paths are relative to the including file, or searched via `-i` paths.

### `if defined` / Conditional Assembly

```asm
if defined WIN32
    ; Windows-specific code
    section '.idata' import data readable
end if

if defined DEBUG
    ; debug output
end if
```

### `if` with Expressions

```asm
if VERSION >= 2
    ; newer code path
else
    ; fallback
end if
```

### `-d` Defines from CLI

```sh
./tiny -d WIN32=1 -d VERSION=3 main.s main.exe
```

Inside source:
```asm
if defined WIN32
    ; ...
end if

if VERSION >= 3
    ; ...
end if
```

### `repeat` / `end repeat`

```asm
repeat 8
    u8 0
end repeat

; with counter variable (1-based)
repeat 4
    u32 % * 100     ; % = current iteration: 100, 200, 300, 400
end repeat
```

### `org`

Sets the virtual origin address (mainly for raw binary):
```asm
format binary
org 0x7C00      ; assume this code loads at 0x7C00
```

---

## Sections & Segments

### Linux ELF — `segment`

```asm
segment readable executable     ; code segment
segment readable writeable      ; data segment (initialized)
segment readable                ; read-only data
```

### Windows PE — `section`

```asm
section '.code'  code readable executable
section '.data'  data readable writeable
section '.rdata' data readable              ; read-only data
section '.idata' import data readable       ; import table
section '.edata' export data readable       ; export table (DLLs)
```

Section names are arbitrary strings up to 8 chars. The flags that matter:

| Flag | Meaning |
|------|---------|
| `code` | marks as code section |
| `data` | marks as data section |
| `readable` | page is readable |
| `writeable` | page is writable |
| `executable` | page is executable |
| `import` | PE import section hint |
| `export` | PE export section hint |

---

## Writing a PE64 Windows Program

This is the full boilerplate you need for a standalone PE64 program. No `core/win32.tny` — that file is internal to tinyasm's own build.

```asm
format pe64 console
entry start

; ── Constants ────────────────────────────────────────────────────────────────
STD_OUTPUT_HANDLE = 0FFFFFFF5h
STD_INPUT_HANDLE  = 0FFFFFFF6h
STD_ERROR_HANDLE  = 0FFFFFFF4h

; ── Code ─────────────────────────────────────────────────────────────────────
section '.code' code readable executable

start:
    sub     rsp, 28h            ; shadow space + alignment

    ; get stdout handle
    mov     ecx, STD_OUTPUT_HANDLE
    call    near qword [GetStdHandle]
    mov     [hOut], eax

    ; write message
    mov     ecx, [hOut]
    lea     rdx, [msg]
    mov     r8d, msglen
    lea     r9, [written]
    mov     qword [rsp+20h], 0  ; lpOverlapped = NULL
    call    near qword [WriteFile]

    ; exit
    xor     ecx, ecx
    call    near qword [ExitProcess]

; ── Data ─────────────────────────────────────────────────────────────────────
section '.data' data readable writeable
    hOut     u32 ?
    written  u32 ?
    msg      u8 'Hello from tinyasm!', 13, 10
    msglen = $ - msg

; ── Import Table ─────────────────────────────────────────────────────────────
section '.idata' import data readable

    ; IMAGE_IMPORT_DESCRIPTOR (5 DWORDs = 20 bytes per DLL)
    u32 RVA kernel32_ilt    ; OriginalFirstThunk (ILT RVA)
    u32 0                   ; TimeDateStamp
    u32 0                   ; ForwarderChain
    u32 RVA kernel32_name   ; Name RVA
    u32 RVA kernel32_iat    ; FirstThunk (IAT RVA)
    u32 0, 0, 0, 0, 0       ; null terminator descriptor

    ; ILT — Import Lookup Table (parallel to IAT, read-only)
    kernel32_ilt:
        u64 RVA _GetStdHandle
        u64 RVA _WriteFile
        u64 RVA _ExitProcess
        u64 0

    ; IAT — Import Address Table (filled by Windows loader at runtime)
    kernel32_iat:
        GetStdHandle  u64 RVA _GetStdHandle
        WriteFile     u64 RVA _WriteFile
        ExitProcess   u64 RVA _ExitProcess
        u64 0

    ; DLL name string
    kernel32_name u8 'KERNEL32.DLL', 0

    ; Hint/Name entries: u16 hint (0 = don't care), then null-terminated name
    _GetStdHandle  u16 0
        u8 'GetStdHandle', 0
    _WriteFile     u16 0
        u8 'WriteFile', 0
    _ExitProcess   u16 0
        u8 'ExitProcess', 0
```

### Calling Win32 Functions

Always use `near qword [FuncName]` — the label in the IAT holds the actual function pointer filled by the loader:

```asm
call near qword [WriteFile]     ; correct
call WriteFile                  ; will NOT work — WriteFile is a data label, not code
```

### Adding More DLL Imports

Each DLL needs its own `IMAGE_IMPORT_DESCRIPTOR`. Stack them before the null terminator:

```asm
section '.idata' import data readable

    ; kernel32 descriptor
    u32 RVA k32_ilt, 0, 0, RVA k32_name, RVA k32_iat
    ; user32 descriptor
    u32 RVA u32_ilt, 0, 0, RVA u32_name, RVA u32_iat
    ; null terminator
    u32 0, 0, 0, 0, 0

    k32_ilt:
        u64 RVA _ExitProcess
        u64 0
    k32_iat:
        ExitProcess u64 RVA _ExitProcess
        u64 0
    k32_name u8 'KERNEL32.DLL', 0
    _ExitProcess u16 0
        u8 'ExitProcess', 0

    u32_ilt:
        u64 RVA _MessageBoxA
        u64 0
    u32_iat:
        MessageBoxA u64 RVA _MessageBoxA
        u64 0
    u32_name u8 'USER32.DLL', 0
    _MessageBoxA u16 0
        u8 'MessageBoxA', 0
```

### Useful Win32 Constants

```asm
; Standard handles
STD_INPUT_HANDLE  = 0FFFFFFF6h   ; -10
STD_OUTPUT_HANDLE = 0FFFFFFF5h   ; -11
STD_ERROR_HANDLE  = 0FFFFFFF4h   ; -12

; File access
GENERIC_READ      = 80000000h
GENERIC_WRITE     = 40000000h

; File share
FILE_SHARE_READ   = 1

; Creation disposition
CREATE_ALWAYS     = 2
OPEN_EXISTING     = 3

; Memory
MEM_COMMIT        = 1000h
MEM_RESERVE       = 2000h
MEM_RELEASE       = 8000h
PAGE_READWRITE    = 4

; File pointer
FILE_BEGIN        = 0
FILE_CURRENT      = 1
FILE_END          = 2

INVALID_HANDLE_VALUE = -1
```

---

## Writing an ELF64 Linux Program

```asm
format elf64 executable 3
entry _start

; ── Code ─────────────────────────────────────────────────────────────────────
segment readable executable

_start:
    ; write(1, msg, msglen)
    mov eax, 1          ; syscall: sys_write
    mov edi, 1          ; fd: stdout
    lea rsi, [msg]      ; buf
    mov edx, [msglen]   ; count
    syscall

    ; exit(0)
    mov eax, 60         ; syscall: sys_exit
    xor edi, edi        ; status: 0
    syscall

; ── Data ─────────────────────────────────────────────────────────────────────
segment readable

msg     u8 'Hello from tinyasm!', 10
msglen  u32 $ - msg
```

### ELF32 (32-bit Linux)

```asm
format elf executable 3
entry _start

segment readable executable

_start:
    mov eax, 4          ; sys_write
    mov ebx, 1          ; stdout
    mov ecx, msg
    mov edx, msglen
    int 0x80

    mov eax, 1          ; sys_exit
    xor ebx, ebx
    int 0x80

segment readable

msg     u8 'Hello!', 10
msglen = $ - msg
```

### ELF Object File (for linking)

```asm
format elf64            ; no "executable" — produces .o

public my_function      ; export symbol

extrn printf            ; import external symbol

segment readable executable

my_function:
    ; ... your code ...
    ret
```

---

## Writing a Raw Binary

Raw binary mode has no headers — just bytes at a fixed address. Great for bootloaders, firmware, shellcode.

```asm
format binary
org 0x7C00              ; MBR loads at 0x7C00

; ── VBR / Bootloader ─────────────────────────────────────────────────────────

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    ; print 'H'
    mov ah, 0x0E
    mov al, 'H'
    xor bh, bh
    int 0x10

    jmp $               ; hang

; ── Padding & Boot Signature ─────────────────────────────────────────────────
times 510-($-$$) u8 0
u16 0xAA55
```

---

## Win64 ABI Cheatsheet

When calling Win64 API (or any Win64 function), follow these rules:

**Register passing (first 4 args):**

| Arg | Integer/Pointer | Float |
|-----|----------------|-------|
| 1st | `rcx` | `xmm0` |
| 2nd | `rdx` | `xmm1` |
| 3rd | `r8` | `xmm2` |
| 4th | `r9` | `xmm3` |
| 5th+ | `[rsp+20h]`, `[rsp+28h]`, ... | same stack slots |

**Stack rules:**
- Allocate **32 bytes of shadow space** before any call (`sub rsp, 20h` minimum)
- Stack must be **16-byte aligned** at the point of the `call` instruction
- The `call` itself pushes 8 bytes (return address), so before the call, RSP mod 16 should be 8

**Volatile vs non-volatile registers:**

| Volatile (caller-saved) | Non-volatile (callee-saved) |
|-------------------------|-----------------------------|
| `rax`, `rcx`, `rdx`, `r8`, `r9`, `r10`, `r11` | `rbx`, `rbp`, `rsi`, `rdi`, `r12`–`r15` |
| `xmm0`–`xmm5` | `xmm6`–`xmm15` |

**Return value:** `rax` (integer), `xmm0` (float).

**Typical function prologue/epilogue:**
```asm
myfunc:
    sub rsp, 28h        ; 20h shadow space + 8h alignment padding

    ; ... do stuff ...

    add rsp, 28h
    ret
```

**Calling with 5+ args:**
```asm
    sub rsp, 38h                ; 20h shadow + space for 5th arg + alignment
    mov ecx, arg1
    mov edx, arg2
    mov r8d, arg3
    mov r9d, arg4
    mov dword [rsp+20h], arg5   ; 5th arg on stack
    call near qword [SomeFunc]
    add rsp, 38h
```

---

## Linux Syscall Cheatsheet

### x86-64 (64-bit)

Syscall number in `rax`, args in `rdi`, `rsi`, `rdx`, `r10`, `r8`, `r9`. Return in `rax`.

| Syscall | Number | Args |
|---------|--------|------|
| `read` | 0 | fd, buf, count |
| `write` | 1 | fd, buf, count |
| `open` | 2 | path, flags, mode |
| `close` | 3 | fd |
| `exit` | 60 | status |
| `exit_group` | 231 | status |
| `mmap` | 9 | addr, len, prot, flags, fd, offset |
| `brk` | 12 | addr |

```asm
; write("Hello\n", 6) to stdout
mov rax, 1
mov rdi, 1
lea rsi, [msg]
mov rdx, 6
syscall
```

### x86 (32-bit)

Syscall number in `eax`, args in `ebx`, `ecx`, `edx`, `esi`, `edi`. Use `int 0x80`.

| Syscall | Number |
|---------|--------|
| `exit` | 1 |
| `write` | 4 |
| `read` | 3 |
| `open` | 5 |
| `close` | 6 |

```asm
mov eax, 4      ; sys_write
mov ebx, 1      ; stdout
mov ecx, msg
mov edx, len
int 0x80
```

---

## Error Reference

| Error | Meaning & Fix |
|-------|---------------|
| `undefined symbol 'X'` | `X` was used but never defined. Check spelling, include order, or add a `-d X=value` define. |
| `symbol redefined` | Same label declared more than once in the same scope. |
| `unknown instruction` | Typo in mnemonic, or using a NASM/FASM directive that doesn't exist here (e.g., `db`, `dw`, `dd`, `dq`). Use `u8`, `u16`, `u32`, `u64` instead. |
| `invalid operand` | Operand type not valid here (e.g., two memory operands). |
| `operand size missing` | Size is ambiguous — add `byte`/`word`/`dword`/`qword` override. |
| `operand size mismatch` | Source and dest sizes differ and can't be inferred. |
| `immediate value too large` | Constant doesn't fit in the instruction encoding. |
| `jump target out of range` | Short jump can't reach target — use a near jump or restructure code. |
| `code generation not possible` | A forward reference didn't resolve after max passes. Usually a circular dependency. |
| `out of memory` | Exceeded `-m` limit — use `-m 65536` or higher. |
| `file not found` | An `include`'d file can't be located — check `-i` paths. |
| `source file not found` | Input `.s` file doesn't exist. |
| `output write failed` | Disk full or permission denied on output path. |
| `unterminated macro` | `macro { }` block missing closing `}`. |
| `bad macro arguments` | Wrong number or type of args to a macro call. |
| `malformed expression` | Syntax error in an expression — check parentheses and operators. |
| `assertion failed` | An `assert` directive evaluated to false. |
| `output format limit exceeded` | Output binary too large for the chosen format. |

---

## What `core/*.tny` Actually Is

This trips up a lot of people (including the `STD_OUTPUT_HANDLE` error you probably just hit).

**`core/*.tny` is NOT a standard library. It is the internal implementation of tinyasm itself.**

| File | What it actually is |
|------|---------------------|
| `core/win32.tny` | tinyasm's own Win64 I/O layer — `ta_display_string`, `ta_init_memory`, etc. Depends on 100+ variables in `state.tny`. |
| `core/linux.tny` | tinyasm's own Linux syscall layer. Same deal. |
| `core/platform.tny` | Macro shims that let tinyasm's own code run in both 32-bit and 64-bit mode. |
| `core/state.tny` | ~100 internal assembler state variables. Meaningless outside tinyasm's build. |
| `core/expand.tny` | The macro preprocessor engine. |
| `core/scan.tny` | The source scanner/parser. |
| `core/emit.tny` | The instruction emitter. |
| `core/structs.tny` | Opcode tables. |

**The only file that's sometimes safe to use in a user program is `core/platform.tny`** — and only for the `use32`/`use64` mode macros and the `promote_esi` helpers. Everything else in core assumes the entire tinyasm codebase is present.

**The correct approach for user programs:**
- Need Win32 constants? Define them yourself: `STD_OUTPUT_HANDLE = 0FFFFFFF5h`
- Need Win32 functions? Write your own `.idata` section (see the PE64 section above)
- Need Linux syscalls? Use `syscall` directly with the numbers from the cheatsheet
- Need string helpers? Write them yourself or copy the relevant code

---

*Generated from source analysis of the tinyasm codebase.*
