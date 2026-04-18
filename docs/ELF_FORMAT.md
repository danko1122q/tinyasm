# ELF Format — Writing Linux Executables and Object Files by Hand

ELF (Executable and Linkable Format) is what Linux (and most Unix-like systems) use for executables, shared libraries, and object files. This doc covers what tinyasm generates and what you need to write manually.

---

## Table of Contents

- [ELF File Layout Overview](#elf-file-layout-overview)
- [ELF Header](#elf-header)
- [Program Headers (Segments)](#program-headers-segments)
- [Section Headers](#section-headers)
- [ELF64 Executable — How tinyasm Lays It Out](#elf64-executable--how-tinyasm-lays-it-out)
- [ELF32 Executable](#elf32-executable)
- [ELF Object Files](#elf-object-files)
- [Linking Object Files](#linking-object-files)
- [Dynamic Linking — Shared Libraries](#dynamic-linking--shared-libraries)
- [Symbol Table](#symbol-table)
- [Relocations in ELF](#relocations-in-elf)
- [Segment vs Section](#segment-vs-section)
- [Minimal ELF64 Template](#minimal-elf64-template)
- [Minimal ELF32 Template](#minimal-elf32-template)
- [Debugging ELF Structure](#debugging-elf-structure)

---

## ELF File Layout Overview

```
┌────────────────────────────┐  offset 0x00
│  ELF Header (64 bytes)     │  magic, arch, entry point, ph/sh offsets
├────────────────────────────┤
│  Program Headers           │  describes segments (how loader maps to memory)
│  (one per segment)         │
├────────────────────────────┤
│  Section Data              │  actual code and data
│  .text  (code)             │
│  .data  (initialized data) │
│  .bss   (uninitialized)    │
│  .rodata (read-only)       │
│  ...                       │
├────────────────────────────┤
│  Section Headers           │  describes sections (for linker/debugger use)
│  (one per section)         │  usually at end of file
└────────────────────────────┘
```

For a **simple executable**, program headers are what matter — the OS loader uses them to map segments into memory. Section headers are optional for execution but useful for debugging and linking.

For an **object file** (.o), there are no program headers — only section headers. The linker reads those to combine multiple .o files into a final executable.

---

## ELF Header

### ELF64 Header (64 bytes)

| Field | Offset | Size | Description |
|-------|--------|------|-------------|
| `e_ident` | 0 | 16 | magic + class + encoding + version |
| `e_type` | 16 | 2 | 2 = executable, 1 = object, 3 = shared lib |
| `e_machine` | 18 | 2 | `0x3E` = x86-64, `0x03` = x86 |
| `e_version` | 20 | 4 | 1 (current) |
| `e_entry` | 24 | 8 | virtual address of entry point |
| `e_phoff` | 32 | 8 | offset to program header table |
| `e_shoff` | 40 | 8 | offset to section header table |
| `e_flags` | 48 | 4 | 0 for x86/x64 |
| `e_ehsize` | 52 | 2 | size of this header (64 for ELF64) |
| `e_phentsize` | 54 | 2 | size of each program header (56 for ELF64) |
| `e_phnum` | 56 | 2 | number of program headers |
| `e_shentsize` | 58 | 2 | size of each section header (64 for ELF64) |
| `e_shnum` | 60 | 2 | number of section headers |
| `e_shstrndx` | 62 | 2 | index of section name string table |

### `e_ident` breakdown (16 bytes)

| Bytes | Value | Meaning |
|-------|-------|---------|
| 0–3 | `0x7F 'E' 'L' 'F'` | ELF magic |
| 4 | `1` = 32-bit, `2` = 64-bit | Class |
| 5 | `1` = little-endian, `2` = big-endian | Data encoding |
| 6 | `1` | ELF version |
| 7 | `0` = SysV, `3` = Linux | OS/ABI |
| 8–15 | all zeros | padding |

---

## Program Headers (Segments)

Each program header describes a segment — a contiguous region of the file that gets mapped into memory as one chunk.

### ELF64 Program Header (56 bytes)

| Field | Offset | Size | Description |
|-------|--------|------|-------------|
| `p_type` | 0 | 4 | segment type |
| `p_flags` | 4 | 4 | read/write/execute flags |
| `p_offset` | 8 | 8 | file offset of segment data |
| `p_vaddr` | 16 | 8 | virtual address in memory |
| `p_paddr` | 24 | 8 | physical address (usually same as vaddr) |
| `p_filesz` | 32 | 8 | size in file |
| `p_memsz` | 40 | 8 | size in memory (can be larger — BSS) |
| `p_align` | 48 | 8 | alignment (usually 0x1000 or 0x200000) |

### Segment Types (`p_type`)

| Value | Name | Meaning |
|-------|------|---------|
| 1 | `PT_LOAD` | loadable segment — the OS maps this into memory |
| 2 | `PT_DYNAMIC` | dynamic linking info |
| 3 | `PT_INTERP` | path to dynamic linker (`/lib64/ld-linux-x86-64.so.2`) |
| 4 | `PT_NOTE` | auxiliary info |
| 6 | `PT_PHDR` | location of program header table itself |

For a static executable, you typically only have `PT_LOAD` segments.

### Segment Flags (`p_flags`)

| Value | Meaning |
|-------|---------|
| `0x1` | execute |
| `0x2` | write |
| `0x4` | read |

Combinations: `0x5` = read+execute (code), `0x6` = read+write (data).

---

## Section Headers

Section headers are primarily for linkers and debuggers — the OS loader doesn't need them for a static executable. Each section header is 64 bytes in ELF64.

| Field | Offset | Size | Description |
|-------|--------|------|-------------|
| `sh_name` | 0 | 4 | offset into `.shstrtab` (section name string table) |
| `sh_type` | 4 | 4 | section type |
| `sh_flags` | 8 | 8 | section flags |
| `sh_addr` | 16 | 8 | virtual address |
| `sh_offset` | 24 | 8 | file offset |
| `sh_size` | 32 | 8 | section size |
| `sh_link` | 40 | 4 | section index link (type-dependent) |
| `sh_info` | 44 | 4 | extra info (type-dependent) |
| `sh_addralign` | 48 | 8 | alignment |
| `sh_entsize` | 56 | 8 | entry size (for tables like symtab) |

### Common Section Types

| Value | Name | Meaning |
|-------|------|---------|
| 0 | `SHT_NULL` | unused (first section header is always this) |
| 1 | `SHT_PROGBITS` | program data (code, data, rodata) |
| 2 | `SHT_SYMTAB` | symbol table |
| 3 | `SHT_STRTAB` | string table |
| 4 | `SHT_RELA` | relocations with explicit addends |
| 8 | `SHT_NOBITS` | BSS — takes no space in file |
| 11 | `SHT_DYNSYM` | dynamic symbol table |

### Common Section Flags

| Value | Meaning |
|-------|---------|
| `0x1` | `SHF_WRITE` — writable |
| `0x2` | `SHF_ALLOC` — occupies memory during execution |
| `0x4` | `SHF_EXECINSTR` — executable |

---

## ELF64 Executable — How tinyasm Lays It Out

When you write `format elf64 executable 3`, tinyasm generates:

- ELF header with `e_type = ET_EXEC`, `e_machine = 0x3E`
- One or more `PT_LOAD` program headers based on your segments
- Section header table (optional but generated)

Your segments map directly to `PT_LOAD` entries:

```asm
format elf64 executable 3
entry _start

segment readable executable    →  PT_LOAD, flags=0x5 (r-x)
segment readable writeable     →  PT_LOAD, flags=0x6 (rw-)
segment readable               →  PT_LOAD, flags=0x4 (r--)
```

The `3` in `format elf64 executable 3` sets the OS/ABI field in `e_ident[7]` to 3 (Linux). You can use 0 for generic SysV.

**Load address:** tinyasm defaults to `0x400000` as the base virtual address for ELF64 executables. Everything builds from there.

---

## ELF32 Executable

Same structure, different sizes:

- ELF header: 52 bytes
- Program header: 32 bytes
- Section header: 40 bytes
- Pointer/address fields are 4 bytes, not 8

```asm
format elf executable 3     ; 32-bit ELF
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

---

## ELF Object Files

Object files are the intermediate format — they contain code and data but with unresolved symbol references. A linker combines multiple .o files into a final executable.

```asm
format elf64        ; no "executable" → produces .o

public my_function  ; export this symbol
extrn printf        ; declare external dependency

segment readable executable

my_function:
    push rbp
    mov rbp, rsp

    lea rdi, [fmt]
    xor eax, eax
    call printf         ; unresolved — linker fills this in

    pop rbp
    ret

segment readable

fmt u8 'Hello from object file', 10, 0
```

Build:
```sh
./tiny mylib.s mylib.o
ld mylib.o -o myapp -lc          # link with libc
# or with gcc as the linker driver:
gcc mylib.o -o myapp -nostartfiles
```

### `public` and `extrn`

```asm
public my_func      ; make my_func visible to linker (exports the symbol)
extrn printf        ; declare printf as external (linker resolves it)
extrn malloc, free  ; multiple at once
```

In the object file, `public` creates a global symbol with `STB_GLOBAL` binding. `extrn` creates an undefined symbol that the linker must resolve.

---

## Linking Object Files

tinyasm doesn't include a linker — use `ld` directly or via `gcc`.

```sh
# assemble
./tiny main.s main.o
./tiny utils.s utils.o

# link with ld (minimal, no libc)
ld main.o utils.o -o myapp

# link with libc via gcc
gcc main.o utils.o -o myapp

# link with specific start address
ld -Ttext=0x400000 main.o -o myapp

# link a shared library
ld -shared mylib.o -o mylib.so

# static link against libc
gcc main.o -o myapp -static
```

### Linker Script basics (if you need custom layout)

```
SECTIONS {
    . = 0x400000;
    .text : { *(.text) }
    .data : { *(.data) }
    .bss  : { *(.bss)  }
}
```

```sh
ld -T script.ld main.o -o myapp
```

---

## Dynamic Linking — Shared Libraries

For programs that call libc or other shared libraries, the ELF needs additional sections:

- `.interp` — path to the dynamic linker
- `.dynamic` — dynamic linking info
- `.dynsym` — dynamic symbol table
- `.dynstr` — dynamic string table
- `.plt` — Procedure Linkage Table (trampolines for lazy binding)
- `.got` / `.got.plt` — Global Offset Table (holds resolved addresses)

This is complex to write by hand. The practical approach for tinyasm programs that need libc: produce an object file and let `gcc` handle the dynamic linking scaffolding.

```asm
format elf64

extrn puts
extrn exit

public main

segment readable executable

main:
    lea rdi, [msg]
    call puts
    xor edi, edi
    call exit

segment readable

msg u8 'Hello from tinyasm!', 0
```

```sh
./tiny hello.s hello.o
gcc hello.o -o hello       ; gcc adds all the dynamic linking glue
./hello
```

---

## Symbol Table

The `.symtab` section contains all symbols in the file. Each entry is 24 bytes in ELF64:

| Field | Size | Description |
|-------|------|-------------|
| `st_name` | 4 | offset into `.strtab` |
| `st_info` | 1 | type (4 bits) + binding (4 bits) |
| `st_other` | 1 | visibility (usually 0) |
| `st_shndx` | 2 | section index (0xFFF1 = absolute, 0 = undefined) |
| `st_value` | 8 | symbol value / address |
| `st_size` | 8 | symbol size (0 if unknown) |

**Binding (high 4 bits of `st_info`):**
- `0` = `STB_LOCAL` — not visible outside object file
- `1` = `STB_GLOBAL` — visible to linker (`public` in tinyasm)
- `2` = `STB_WEAK` — lower priority than global

**Type (low 4 bits of `st_info`):**
- `0` = `STT_NOTYPE`
- `1` = `STT_OBJECT` — data variable
- `2` = `STT_FUNC` — function

Dump symbols from a tinyasm output:
```sh
./tiny -s symbols.txt main.s main    # tinyasm's own dump
nm main                              # standard tool
readelf -s main
```

---

## Relocations in ELF

Relocations tell the linker "at this offset in this section, patch in the address of this symbol (with this addend)."

### RELA entry (ELF64, 24 bytes)

| Field | Size | Description |
|-------|------|-------------|
| `r_offset` | 8 | offset in section to patch |
| `r_info` | 8 | symbol index (top 32 bits) + reloc type (low 32 bits) |
| `r_addend` | 8 | constant addend |

### Common x86-64 relocation types

| Type | Value | Meaning |
|------|-------|---------|
| `R_X86_64_64` | 1 | absolute 64-bit address |
| `R_X86_64_PC32` | 2 | 32-bit PC-relative |
| `R_X86_64_PLT32` | 4 | 32-bit PC-relative via PLT |
| `R_X86_64_32` | 10 | absolute 32-bit (zero-extend) |
| `R_X86_64_32S` | 11 | absolute 32-bit (sign-extend) |

For static executables built entirely in tinyasm without external symbols, you don't deal with relocations directly — tinyasm resolves everything at assemble time.

---

## Segment vs Section

This trips people up. In ELF they're different things:

| | Segment | Section |
|-|---------|---------|
| Described by | Program Header | Section Header |
| Used by | OS loader | Linker, debugger |
| Granularity | Coarse (whole RX region) | Fine (individual .text, .data, etc.) |
| Required for execution? | Yes | No |
| Required for linking? | No | Yes |

A single segment can contain multiple sections. Typically:
- One `PT_LOAD r-x` segment contains `.text` + `.rodata`
- One `PT_LOAD rw-` segment contains `.data` + `.bss`

In tinyasm, `segment readable executable` = one `PT_LOAD` entry. Within that segment you just write data contiguously — there's no sub-section concept in ELF mode.

---

## Minimal ELF64 Template

```asm
format elf64 executable 3
entry _start

; ── Code ─────────────────────────────────────────────────────────────────────
segment readable executable

_start:
    ; write(1, msg, msglen)
    mov eax, 1
    mov edi, 1
    lea rsi, [msg]
    mov edx, msglen
    syscall

    ; exit(0)
    mov eax, 60
    xor edi, edi
    syscall

; ── Read-only data ────────────────────────────────────────────────────────────
segment readable

msg     u8 'Hello, World!', 10
msglen = $ - msg
```

```sh
./tiny hello.s hello
chmod +x hello
./hello
```

---

## Minimal ELF32 Template

```asm
format elf executable 3
entry _start

segment readable executable

_start:
    mov eax, 4
    mov ebx, 1
    mov ecx, msg
    mov edx, msglen
    int 0x80

    mov eax, 1
    xor ebx, ebx
    int 0x80

segment readable

msg     u8 'Hello, World!', 10
msglen = $ - msg
```

```sh
# needs 32-bit support
./tinyasm32 hello32.s hello32
chmod +x hello32
./hello32
```

---

## Debugging ELF Structure

```sh
# full header info
readelf -h myapp
readelf -l myapp     # program headers (segments)
readelf -S myapp     # section headers
readelf -s myapp     # symbol table
readelf -r myapp     # relocations

# disassemble
objdump -d myapp
objdump -M intel -d myapp   # Intel syntax

# hex dump
xxd myapp | head -8

# check what libraries are needed
ldd myapp

# strace to see syscalls at runtime
strace ./myapp

# quick sanity check
file myapp
```

Common issues:
- **Permission denied** — forgot `chmod +x`
- **No such file or directory** on a valid binary — missing 32-bit support (`apt install libc6-i386` for ELF32)
- **Segfault at start** — entry point is wrong or code segment isn't marked executable
- **Illegal instruction** — using 64-bit instructions in a 32-bit binary or vice versa
- **Symbol not found** — `extrn` declared but not linked against the right library
