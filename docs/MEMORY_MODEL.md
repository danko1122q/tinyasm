# Memory Model — How tinyasm Addresses Memory

Understanding how tinyasm thinks about memory will save you a lot of confusion, especially when mixing PE and ELF targets or writing position-dependent code.

---

## Table of Contents

- [Flat Memory Model](#flat-memory-model)
- [Virtual Address vs File Offset](#virtual-address-vs-file-offset)
- [How tinyasm Tracks Position](#how-tinyasm-tracks-position)
- [$, $$, and org](#--and-org)
- [Segments (ELF)](#segments-elf)
- [Sections (PE)](#sections-pe)
- [Image Base](#image-base)
- [RVA — Relative Virtual Address](#rva--relative-virtual-address)
- [Stack Memory](#stack-memory)
- [Addressing Modes](#addressing-modes)
- [32-bit vs 64-bit Addressing](#32-bit-vs-64-bit-addressing)
- [Common Addressing Mistakes](#common-addressing-mistakes)

---

## Flat Memory Model

tinyasm uses a **flat memory model** — there are no separate code/data/stack segments in the x86 segmentation sense. Everything lives in one continuous virtual address space. Segment registers (`cs`, `ds`, `es`, `ss`) are all set to the same flat segment by the OS at load time.

This means:
- You address everything with a single linear address
- No `far` jumps, no `far` calls, no segment overrides needed (except for `fs`/`gs` which the OS uses for TLS)
- `[rip + offset]` or `[label]` just works

---

## Virtual Address vs File Offset

The same byte can have two "addresses":

| Concept | Meaning |
|---------|---------|
| **File offset** | Where the byte sits in the `.exe`/`.elf` file on disk |
| **Virtual address (VA)** | Where the byte lives in memory after the loader maps it in |

These are different because the loader maps sections into memory at page-aligned addresses, which usually differ from where they sit in the file.

Example for a PE64 executable:
```
File:
  offset 0x200 → .code section starts
  offset 0x400 → .data section starts

Memory (loaded at ImageBase 0x140000000):
  0x140001000 → .code section
  0x140002000 → .data section
```

tinyasm works in **virtual addresses** — all labels, `$`, and `$$` give virtual addresses. The file offsets are computed internally.

---

## How tinyasm Tracks Position

tinyasm maintains a **current address counter** as it processes each instruction and data directive. Every instruction advances the counter by its encoded size. Every data directive advances it by the data size.

This counter is what `$` gives you.

```asm
segment readable executable

start:                  ; $ = 0x400000 (base load address)
    mov eax, 1          ; 5 bytes → $ now 0x400005
    syscall             ; 2 bytes → $ now 0x400007

segment readable

msg:                    ; $ = 0x401000 (new segment, page-aligned)
    u8 'Hi', 10         ; 3 bytes → $ now 0x401003
```

Since it's multi-pass, in the first pass some addresses might be unknown (forward references). tinyasm re-runs until everything stabilizes.

---

## $, $$, and org

### `$` — Current Position

`$` is the virtual address of the **current position** — the next byte that would be emitted.

```asm
msg     u8 'Hello, World'
msglen = $ - msg            ; $ is now right after the string → length = 12
```

```asm
here:
    ; ...
    jmp here                ; jumps to the address of 'here'
    ; same as:
    jmp $                   ; jumps to current position (infinite loop)
```

### `$$` — Start of Current Section/Segment

`$$` is the virtual address of the **beginning of the current section or segment**.

```asm
segment readable executable

    ; some code
    times 16-($-$$) u8 0x90   ; NOP-pad until 16 bytes into this segment
```

In a bootloader:
```asm
format binary
org 0x7C00

    ; code here...
    times 510-($-$$) u8 0    ; pad to byte 510 (from start of this section)
    u16 0xAA55               ; boot signature at bytes 510-511
```

`$-$$` = "how many bytes have been emitted in this section so far."

### `org` — Set the Origin Address

`org` tells tinyasm "assume this code will be loaded at this address." It doesn't emit any bytes — it just changes what `$` reports.

```asm
format binary
org 0x7C00          ; MBR is loaded at 0x7C00 by BIOS

start:
    ; $ = 0x7C00 here
    mov ax, start   ; = 0x7C00

    times 510-($-$$) u8 0
    u16 0xAA55
```

Without `org`, references to labels would compute wrong absolute addresses for position-dependent code.

`org` is mainly for `format binary`. For ELF and PE, the format directive sets the base address automatically.

**Changing `org` mid-file** is valid — useful for multi-region binary images:

```asm
format binary

org 0x7C00
    ; first stage at 0x7C00
    ; ...

org 0x8000
    ; second stage at 0x8000
    ; ...
```

---

## Segments (ELF)

In ELF mode, `segment` creates a new loadable region of memory. Each segment becomes a `PT_LOAD` entry in the program header.

```asm
format elf64 executable 3

segment readable executable     ; code goes here, mapped r-x
segment readable writeable      ; data goes here, mapped rw-
segment readable                ; read-only data, mapped r--
```

**Alignment:** each segment starts on a page boundary (0x1000 = 4096 bytes). tinyasm pads between segments automatically.

**What happens in memory:**
```
0x400000  ┌─────────────────────┐  segment readable executable
          │  ELF header         │
          │  program headers    │
          │  your .text         │
0x401000  ├─────────────────────┤  segment readable writeable
          │  your .data         │
          │  (padded to page)   │
0x402000  └─────────────────────┘
```

There's no concept of separate `.text`/`.data` named sections in ELF mode tinyasm — you just write into segments and tinyasm handles the program headers.

---

## Sections (PE)

In PE mode, `section` creates a named region with specific attributes. Each section becomes a section table entry in the PE.

```asm
format pe64 console

section '.code'  code readable executable
section '.data'  data readable writeable
section '.rdata' data readable
section '.idata' import data readable
```

**Alignment:** PE sections are aligned to `SectionAlignment` in memory (default 0x1000) and `FileAlignment` in the file (default 0x200). tinyasm handles padding.

**What happens in memory (ImageBase = 0x140000000):**
```
0x140001000  .code    (RVA 0x1000)
0x140002000  .data    (RVA 0x2000)
0x140003000  .rdata   (RVA 0x3000)
0x140004000  .idata   (RVA 0x4000)
```

Each section starts at a multiple of 0x1000 from the image base. The exact RVA depends on how many bytes the preceding sections take up (rounded up to the alignment).

---

## Image Base

The **image base** is where the OS loader places the executable in virtual memory.

| Format | Default Image Base |
|--------|--------------------|
| PE64 executable | `0x140000000` |
| PE32 executable | `0x400000` |
| PE64 DLL | `0x180000000` |
| ELF64 | `0x400000` |
| ELF32 | `0x8048000` |
| raw binary | wherever `org` says |

With ASLR, the actual load address might differ from the image base. PE handles this via the base relocation table (`.reloc`). tinyasm generates this automatically for PE.

For ELF, position-independent executables (PIE) use `R_X86_64_PC32` relative relocations so they work at any load address. Static executables with a fixed base don't need this.

---

## RVA — Relative Virtual Address

An RVA is an offset from the image base — used throughout PE format for internal references.

```
VA  = ImageBase + RVA
RVA = VA - ImageBase
```

In tinyasm, `RVA label` computes the RVA of a label and writes it as a value. Used in the import table, export table, and data directories:

```asm
; data directory entry pointing to import table:
u32 RVA idata_start   ; RVA of .idata section
u32 idata_size        ; size of .idata section

; IAT entry:
GetStdHandle   u64 RVA _GetStdHandle   ; RVA to hint/name entry
```

`RVA` is only valid in PE mode. In ELF mode, just use labels directly.

---

## Stack Memory

The stack grows downward in x86/x64. The OS allocates stack space (default 1MB reserved, 4KB committed for PE64) and sets RSP/ESP to point to the top.

```
high address  ──────────────────
              │  arg n          │
              │  ...            │
              │  arg 1          │
              │  return addr    │  ← [rsp] after call
              │  saved regs     │
              │  local vars     │
              │  shadow space   │  ← [rsp] after sub rsp, 28h
low address   ──────────────────
```

**Stack alignment rule (x64):** RSP must be 16-byte aligned at the point of a `call` instruction. Since `call` pushes 8 bytes (return address), RSP mod 16 = 8 just before the call.

At `_start` / `start` in a PE64 program:
- RSP is 16-byte aligned by the OS
- After `sub rsp, 28h` (40 bytes = 0x28), RSP is still aligned: `aligned - 0x28 mod 16 = aligned - 8 mod 16 = 8` → correct for a call

```asm
start:
    sub rsp, 28h     ; 0x20 shadow + 0x8 alignment pad → RSP now 16-aligned - 8
    call near qword [SomeFunc]   ; call pushes 8 → RSP 16-aligned inside callee ✓
    add rsp, 28h
    ret
```

If you push an odd number of 8-byte values before a call, you need an extra 8-byte pad to restore alignment.

---

## Addressing Modes

tinyasm supports the full x86/x64 addressing syntax:

### Register Indirect

```asm
mov eax, [rbx]          ; load dword at address in rbx
mov [rcx], al           ; store byte at address in rcx
```

### Base + Displacement

```asm
mov eax, [rbx + 8]      ; rbx + 8
mov eax, [rbx - 4]      ; rbx - 4
mov eax, [rsp + 20h]    ; 5th argument (shadow space)
```

### Base + Index * Scale

```asm
mov eax, [rbx + rcx*4]      ; array of dwords: rbx[rcx]
mov eax, [rbx + rcx*8 + 16] ; base + scaled index + displacement
```

Scale must be 1, 2, 4, or 8.

### RIP-Relative (64-bit only)

```asm
lea rax, [msg]           ; tinyasm emits this as RIP-relative automatically
mov eax, [data_table + rcx*4]
```

In 64-bit mode, label references in memory operands are RIP-relative unless you explicitly use a 32-bit register as base.

### Absolute (32-bit address zero-extended)

```asm
mov eax, [0x1234]        ; absolute address — rare in 64-bit mode
```

### Size Overrides

```asm
mov byte  [rbx], 0      ; write 1 byte
mov word  [rbx], 0      ; write 2 bytes
mov dword [rbx], 0      ; write 4 bytes
mov qword [rbx], 0      ; write 8 bytes
```

Size overrides are required when the assembler can't infer the size from the operands:
```asm
mov [rbx], 0            ; ERROR: size missing
mov dword [rbx], 0      ; OK
```

---

## 32-bit vs 64-bit Addressing

In 64-bit code, using 32-bit registers as addresses causes a **address-size prefix** (0x67) in the encoding, which works but is slightly inefficient.

```asm
; 64-bit clean — no prefix:
mov esi, [rip+msg]
lea rsi, [msg]

; 32-bit base in 64-bit code — gets 0x67 prefix:
mov esi, [esi]      ; 32-bit address size in 64-bit mode
```

tinyasm's `core/platform.tny` has `promote_esi` etc. macros that do `mov esi, esi` to explicitly zero-extend 32-bit pointer values to 64-bit, eliminating the prefix in subsequent addressing.

If you're writing user programs (not tinyasm internals), just use 64-bit registers for all addresses and you won't hit this.

---

## Common Addressing Mistakes

### Two memory operands

x86 doesn't allow both source and destination to be memory references in one instruction:

```asm
mov [dest], [src]       ; INVALID — two memory operands
; solution:
mov rax, [src]
mov [dest], rax
```

### Forgetting size in ambiguous context

```asm
cmp [rbx], 0            ; ERROR: size missing
cmp dword [rbx], 0      ; OK
cmp byte  [rbx], 0      ; OK
```

### Stack misalignment before call

```asm
push rcx                ; RSP -= 8
call SomeFunc           ; RSP not aligned! (was aligned, pushed 8, now misaligned)

; fix: push an even number of 8-byte values, or pad:
push rcx
sub rsp, 8              ; alignment pad
call SomeFunc
add rsp, 8
pop rcx
```

### Using 32-bit label addresses in 64-bit code

```asm
mov eax, msg            ; truncates 64-bit address to 32 bits — wrong if address > 0xFFFFFFFF
lea rax, [msg]          ; correct — full 64-bit RIP-relative address
```

### Off-by-one with $ and $$

```asm
msg u8 'Hi'
len = $ - msg       ; correct — 2

; wrong:
len = $$ - msg      ; $$ is start of section, not start of msg
```
