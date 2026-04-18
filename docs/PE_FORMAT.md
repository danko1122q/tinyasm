# PE64 Format — Writing Windows Executables by Hand

When you write a PE64 program in tinyasm, there's no linker doing the heavy lifting. You're laying out every byte of the executable format manually. This doc explains exactly what that format is and why each piece exists.

---

## Table of Contents

- [PE64 File Layout Overview](#pe64-file-layout-overview)
- [DOS Stub](#dos-stub)
- [PE Signature](#pe-signature)
- [COFF File Header](#coff-file-header)
- [Optional Header](#optional-header)
- [Section Table](#section-table)
- [Sections in Practice](#sections-in-practice)
- [Import Table (.idata)](#import-table-idata)
- [Export Table (.edata)](#export-table-edata)
- [Relocations](#relocations)
- [RVA — Relative Virtual Address](#rva--relative-virtual-address)
- [How tinyasm Handles All This](#how-tinyasm-handles-all-this)
- [Minimal PE64 Template](#minimal-pe64-template)
- [Debugging PE Structure](#debugging-pe-structure)

---

## PE64 File Layout Overview

```
┌─────────────────────────────┐  offset 0x00
│  DOS Header (64 bytes)      │  IMAGE_DOS_HEADER
│  DOS Stub (small program)   │  "This program cannot be run..."
├─────────────────────────────┤  offset from e_lfanew (usually 0x80)
│  PE Signature (4 bytes)     │  "PE\0\0"
├─────────────────────────────┤
│  COFF File Header (20 bytes)│  machine, sections, flags
├─────────────────────────────┤
│  Optional Header (112 bytes)│  entry point, image base, sizes
│  Data Directories (128 bytes│  RVAs to import/export/reloc tables
├─────────────────────────────┤
│  Section Table              │  one 40-byte entry per section
├─────────────────────────────┤
│  Section Data               │
│  .code  (your code)         │
│  .data  (your data)         │
│  .idata (import table)      │
│  ...                        │
└─────────────────────────────┘
```

tinyasm's `format pe64` directive generates the DOS header, PE signature, COFF header, and Optional Header automatically. You define the sections and their content.

---

## DOS Stub

The first 64 bytes are `IMAGE_DOS_HEADER`. The only fields that matter:

| Field | Offset | Value | Meaning |
|-------|--------|-------|---------|
| `e_magic` | 0x00 | `MZ` (0x5A4D) | DOS magic number |
| `e_lfanew` | 0x3C | usually 0x40–0x80 | offset to PE signature |

Everything in between is the DOS stub — a tiny 16-bit program that prints "This program cannot be run in DOS mode" if you somehow run it under DOS. Modern tools often set this to all zeros except the two fields above.

tinyasm handles all of this for you with `format pe64`.

---

## PE Signature

Four bytes at the offset pointed to by `e_lfanew`:

```
50 45 00 00   →   "PE\0\0"
```

---

## COFF File Header

Immediately follows the PE signature. 20 bytes.

| Field | Size | Description |
|-------|------|-------------|
| `Machine` | 2 | `0x8664` = AMD64, `0x014C` = i386 |
| `NumberOfSections` | 2 | how many sections follow the optional header |
| `TimeDateStamp` | 4 | unix timestamp (can be 0) |
| `PointerToSymbolTable` | 4 | 0 for executables |
| `NumberOfSymbols` | 4 | 0 for executables |
| `SizeOfOptionalHeader` | 2 | 0xF0 for PE64 (240 bytes) |
| `Characteristics` | 2 | flags — see below |

**Characteristics flags:**

| Flag | Value | Meaning |
|------|-------|---------|
| `IMAGE_FILE_EXECUTABLE_IMAGE` | 0x0002 | it's an executable |
| `IMAGE_FILE_LARGE_ADDRESS_AWARE` | 0x0020 | can use >2GB addresses |
| `IMAGE_FILE_32BIT_MACHINE` | 0x0100 | 32-bit (PE32 only) |
| `IMAGE_FILE_DLL` | 0x2000 | it's a DLL |

A typical PE64 exe has `Characteristics = 0x0022`.

---

## Optional Header

Despite the name, this is required for executables. For PE64 it's 112 bytes of header + 128 bytes of data directories = 240 bytes total.

### Standard Fields

| Field | Offset | Size | Description |
|-------|--------|------|-------------|
| `Magic` | 0 | 2 | `0x020B` for PE64, `0x010B` for PE32 |
| `MajorLinkerVersion` | 2 | 1 | can be 0 |
| `MinorLinkerVersion` | 3 | 1 | can be 0 |
| `SizeOfCode` | 4 | 4 | total size of code sections |
| `SizeOfInitializedData` | 8 | 4 | total size of data sections |
| `SizeOfUninitializedData` | 12 | 4 | total BSS size |
| `AddressOfEntryPoint` | 16 | 4 | **RVA** of entry point |
| `BaseOfCode` | 20 | 4 | RVA of first code section |

### Windows-Specific Fields

| Field | Offset | Size | Description |
|-------|--------|------|-------------|
| `ImageBase` | 24 | 8 | preferred load address (usually `0x140000000` for PE64) |
| `SectionAlignment` | 32 | 4 | section alignment in memory (usually 0x1000 = 4KB) |
| `FileAlignment` | 36 | 4 | section alignment in file (usually 0x200 = 512 bytes) |
| `MajorOSVersion` | 40 | 2 | min Windows version |
| `MinorOSVersion` | 42 | 2 | |
| `SizeOfImage` | 56 | 4 | total size of image in memory, aligned to SectionAlignment |
| `SizeOfHeaders` | 60 | 4 | size of all headers, aligned to FileAlignment |
| `Subsystem` | 68 | 2 | 3 = console, 2 = GUI |
| `DllCharacteristics` | 70 | 2 | ASLR, DEP, etc. |
| `SizeOfStackReserve` | 72 | 8 | stack size to reserve (default 1MB) |
| `SizeOfStackCommit` | 80 | 8 | stack to commit upfront (default 4KB) |
| `SizeOfHeapReserve` | 88 | 8 | heap reserve |
| `SizeOfHeapCommit` | 96 | 8 | heap commit |
| `NumberOfRvaAndSizes` | 104 | 4 | number of data directory entries (usually 16) |

### Data Directories

16 entries follow, each being an (RVA, Size) pair of 8 bytes:

| Index | Name |
|-------|------|
| 0 | Export Table |
| 1 | **Import Table** ← most important for regular programs |
| 2 | Resource Table |
| 3 | Exception Table |
| 4 | Certificate Table |
| 5 | Base Relocation Table |
| 6 | Debug |
| 12 | Import Address Table |
| 14 | CLR Runtime Header |

tinyasm fills these in automatically based on your section flags (`import`, `export`, etc.).

---

## Section Table

Each section has a 40-byte descriptor:

| Field | Size | Description |
|-------|------|-------------|
| `Name` | 8 | null-padded ASCII name (e.g., `.code\0\0\0`) |
| `VirtualSize` | 4 | actual size of section data in memory |
| `VirtualAddress` | 4 | RVA where section loads in memory |
| `SizeOfRawData` | 4 | size in file (aligned to FileAlignment) |
| `PointerToRawData` | 4 | file offset of section data |
| `PointerToRelocations` | 4 | 0 for executables |
| `PointerToLinenumbers` | 4 | 0 (deprecated) |
| `NumberOfRelocations` | 2 | 0 for executables |
| `NumberOfLinenumbers` | 2 | 0 |
| `Characteristics` | 4 | section flags |

**Section Characteristics:**

| Flag | Value | Meaning |
|------|-------|---------|
| `IMAGE_SCN_CNT_CODE` | 0x00000020 | contains code |
| `IMAGE_SCN_CNT_INITIALIZED_DATA` | 0x00000040 | contains initialized data |
| `IMAGE_SCN_CNT_UNINITIALIZED_DATA` | 0x00000080 | BSS |
| `IMAGE_SCN_MEM_EXECUTE` | 0x20000000 | executable |
| `IMAGE_SCN_MEM_READ` | 0x40000000 | readable |
| `IMAGE_SCN_MEM_WRITE` | 0x80000000 | writable |

A typical `.code` section: `0x60000020` (code + execute + read).
A typical `.data` section: `0xC0000040` (initialized data + read + write).

In tinyasm you express this as:
```asm
section '.code'  code readable executable
section '.data'  data readable writeable
section '.rdata' data readable
section '.idata' import data readable
```

---

## Sections in Practice

### `.code` — Executable Code

```asm
section '.code' code readable executable

start:
    sub rsp, 28h
    ; ... your code ...
    xor ecx, ecx
    call near qword [ExitProcess]
```

### `.data` — Initialized Read/Write Data

```asm
section '.data' data readable writeable
    counter  u32 0
    buffer   rb 256
    msg      u8 'Hello', 13, 10
    msglen = $ - msg
```

### `.rdata` — Read-Only Data

```asm
section '.rdata' data readable
    version_str u8 'v1.0', 0
    pi_val      u64 4614253070214989087  ; IEEE 754 for 3.14159...
```

### `.bss` — Uninitialized Data

tinyasm doesn't have a dedicated BSS section directive — just use `rb`/`rd`/`rq` in `.data`:
```asm
section '.data' data readable writeable
    big_buffer rb 65536   ; 64KB of zeros
```

---

## Import Table (.idata)

This is the part you write entirely by hand in tinyasm. Here's exactly what the Windows loader expects.

### Structure Overview

```
.idata section:
  ┌─────────────────────────────────────────┐
  │  IMAGE_IMPORT_DESCRIPTOR (20 bytes)     │  ← one per DLL
  │  IMAGE_IMPORT_DESCRIPTOR (20 bytes)     │  ← another DLL
  │  IMAGE_IMPORT_DESCRIPTOR (zeros)        │  ← null terminator
  ├─────────────────────────────────────────┤
  │  ILT entries (Import Lookup Table)      │  ← RVAs to hint/name
  │  ILT null terminator (8 bytes zero)     │
  ├─────────────────────────────────────────┤
  │  IAT entries (Import Address Table)     │  ← loader fills these at runtime
  │  IAT null terminator (8 bytes zero)     │
  ├─────────────────────────────────────────┤
  │  DLL name string                        │
  ├─────────────────────────────────────────┤
  │  Hint/Name entries                      │  ← u16 hint + function name string
  └─────────────────────────────────────────┘
```

### IMAGE_IMPORT_DESCRIPTOR (20 bytes)

| Field | Size | Description |
|-------|------|-------------|
| `OriginalFirstThunk` | 4 | RVA to ILT (can be 0 — loader uses FirstThunk instead) |
| `TimeDateStamp` | 4 | 0 |
| `ForwarderChain` | 4 | 0 |
| `Name` | 4 | RVA to null-terminated DLL name |
| `FirstThunk` | 4 | RVA to IAT (loader overwrites these with actual addresses) |

### ILT vs IAT

Both tables have the same initial layout — parallel arrays of 8-byte entries pointing to hint/name entries. The difference:

- **ILT** (OriginalFirstThunk) — read-only reference, never modified
- **IAT** (FirstThunk) — the loader overwrites each entry with the actual function address at load time

When your code does `call near qword [WriteFile]`, it's reading from the IAT — by that point the loader has already filled in the real address.

### Hint/Name Entry

```
u16  hint       ; ordinal hint (0 = don't care, loader looks up by name)
u8   'FunctionName', 0
```

If `hint` matches the function's ordinal in the DLL's export table, the loader can skip the name comparison and bind faster. Just use 0 — the performance difference is negligible.

### Full Single-DLL Example

```asm
section '.idata' import data readable

    ; ── IMAGE_IMPORT_DESCRIPTOR ──────────────────────────────────────────────
    u32 RVA kernel32_ilt    ; OriginalFirstThunk
    u32 0                   ; TimeDateStamp
    u32 0                   ; ForwarderChain
    u32 RVA kernel32_name   ; Name
    u32 RVA kernel32_iat    ; FirstThunk
    u32 0, 0, 0, 0, 0       ; null terminator

    ; ── ILT ──────────────────────────────────────────────────────────────────
    kernel32_ilt:
        u64 RVA _ExitProcess
        u64 RVA _WriteFile
        u64 RVA _GetStdHandle
        u64 0

    ; ── IAT ──────────────────────────────────────────────────────────────────
    kernel32_iat:
        ExitProcess    u64 RVA _ExitProcess
        WriteFile      u64 RVA _WriteFile
        GetStdHandle   u64 RVA _GetStdHandle
        u64 0

    ; ── DLL name ─────────────────────────────────────────────────────────────
    kernel32_name u8 'KERNEL32.DLL', 0

    ; ── Hint/Name entries ────────────────────────────────────────────────────
    _ExitProcess   u16 0
        u8 'ExitProcess', 0
    _WriteFile     u16 0
        u8 'WriteFile', 0
    _GetStdHandle  u16 0
        u8 'GetStdHandle', 0
```

### Multi-DLL Example

```asm
section '.idata' import data readable

    ; descriptor for kernel32
    u32 RVA k32_ilt, 0, 0, RVA k32_name, RVA k32_iat
    ; descriptor for user32
    u32 RVA u32_ilt, 0, 0, RVA u32_name, RVA u32_iat
    ; null terminator
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

---

## Export Table (.edata)

Only needed for DLLs. The export table tells the loader which functions this DLL exposes.

```asm
format pe64 dll
entry DllMain

section '.code' code readable executable

DllMain:
    mov eax, 1
    ret

my_add:
    lea eax, [ecx + edx]
    ret

section '.edata' export data readable

    ; IMAGE_EXPORT_DIRECTORY (40 bytes)
    u32 0                       ; Characteristics
    u32 0                       ; TimeDateStamp
    u16 0, 0                    ; MajorVersion, MinorVersion
    u32 RVA dll_name            ; Name RVA
    u32 1                       ; Base (ordinal base)
    u32 1                       ; NumberOfFunctions
    u32 1                       ; NumberOfNames
    u32 RVA export_funcs        ; AddressOfFunctions
    u32 RVA export_names        ; AddressOfNames
    u32 RVA export_ordinals     ; AddressOfNameOrdinals

    export_funcs:
        u32 RVA my_add          ; function RVAs

    export_names:
        u32 RVA name_my_add     ; name RVAs

    export_ordinals:
        u16 0                   ; ordinals (0-based index into export_funcs)

    dll_name    u8 'mylib.dll', 0
    name_my_add u8 'my_add', 0
```

---

## Relocations

PE64 executables compiled with a known `ImageBase` technically don't need relocations — if the loader can place the image at the preferred base address, everything is fine.

But if ASLR is enabled (it usually is), the loader might put the image somewhere else. In that case, the `.reloc` section tells the loader which addresses to fix up.

tinyasm handles relocations automatically when you use `format pe64` — it generates the base relocation table for you. You generally don't need to write `.reloc` by hand.

If you want to explicitly disable ASLR (e.g., for a fixed-address tool):
```asm
format pe64 console
```
Without `DllCharacteristics` set to include `IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE` (0x0040), ASLR won't kick in. tinyasm defaults to this for simplicity.

---

## RVA — Relative Virtual Address

An RVA is an offset relative to the image base — not an absolute address, not a file offset.

```
Virtual Address = ImageBase + RVA
```

So if `ImageBase = 0x140000000` and the entry point RVA is `0x1000`, the entry point loads at `0x140001000`.

In tinyasm, `RVA label` computes the RVA of any label automatically:
```asm
u32 RVA my_function    ; writes the RVA of my_function as a dword
u64 RVA _WriteFile     ; writes the RVA as a qword (for IAT entries)
```

---

## How tinyasm Handles All This

When you write `format pe64 console`, tinyasm automatically generates:

- The DOS header + stub
- PE signature
- COFF file header (machine = 0x8664, correct characteristics)
- Optional header (magic = 0x020B, subsystem = 3 for console)
- Data directories (pointing to your `.idata` section if present)
- Section table entries for each section you declare
- Base relocation table if needed

What you still write manually:
- All section content
- The entire `.idata` import table structure
- The `.edata` export table (for DLLs)
- Any resource sections

---

## Minimal PE64 Template

Copy-paste starting point for any Windows console program:

```asm
format pe64 console
entry start

; ── Constants ────────────────────────────────────────────────────────────────
STD_OUTPUT_HANDLE = 0FFFFFFF5h
STD_ERROR_HANDLE  = 0FFFFFFF4h

; ── Code ─────────────────────────────────────────────────────────────────────
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

; ── Data ─────────────────────────────────────────────────────────────────────
section '.data' data readable writeable
    hOut     u32 ?
    written  u32 ?
    msg      u8 'Hello!', 13, 10
    msglen = $ - msg

; ── Imports ──────────────────────────────────────────────────────────────────
section '.idata' import data readable

    u32 RVA k32_ilt, 0, 0, RVA k32_name, RVA k32_iat
    u32 0, 0, 0, 0, 0

    k32_ilt:
        u64 RVA _GetStdHandle
        u64 RVA _WriteFile
        u64 RVA _ExitProcess
        u64 0

    k32_iat:
        GetStdHandle   u64 RVA _GetStdHandle
        WriteFile      u64 RVA _WriteFile
        ExitProcess    u64 RVA _ExitProcess
        u64 0

    k32_name u8 'KERNEL32.DLL', 0

    _GetStdHandle  u16 0
        u8 'GetStdHandle', 0
    _WriteFile     u16 0
        u8 'WriteFile', 0
    _ExitProcess   u16 0
        u8 'ExitProcess', 0
```

---

## Debugging PE Structure

If something's wrong with your PE output, these tools help:

```sh
# inspect headers (Linux)
objdump -x hello.exe
readelf -h hello.exe    # won't work on PE, use objdump

# PE-specific (Windows or Wine)
dumpbin /headers hello.exe
dumpbin /imports hello.exe

# cross-platform
python3 -c "
import struct
data = open('hello.exe','rb').read()
e_lfanew = struct.unpack_from('<I', data, 0x3C)[0]
print(f'PE offset: 0x{e_lfanew:X}')
sig = data[e_lfanew:e_lfanew+4]
print(f'PE sig: {sig}')
machine = struct.unpack_from('<H', data, e_lfanew+4)[0]
print(f'Machine: 0x{machine:X}')
"

# hex dump first 256 bytes
xxd hello.exe | head -16
```

Common issues:
- **Image won't load** — section VirtualAddresses must be aligned to SectionAlignment (0x1000). tinyasm handles this automatically.
- **Import fails at runtime** — check DLL name is uppercase and null-terminated. `KERNEL32.DLL` not `kernel32.dll`.
- **Wrong entry point** — verify the `entry` label matches exactly what you defined.
- **Crashes immediately** — check stack alignment. RSP must be 16-byte aligned before a `call`. `sub rsp, 28h` at entry is the minimum safe prologue.
