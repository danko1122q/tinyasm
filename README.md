# tinyasm

A lightweight x86/x64 assembler for Linux and Windows, derived from [fasm (flat assembler)](https://flatassembler.net) by Tomasz Grysztar.

**Version:** 1.0.0

---

## Table of Contents

- [Features](#features)
- [Building](#building)
- [Usage](#usage)
- [Testing the examples](#testing-the-examples)
- [Syntax](#syntax)
- [Output formats](#output-formats)
- [Error messages](#error-messages)
- [Project structure](#project-structure)
- [License](#license)

---

## Features

- Assembles x86 (32-bit) and x64 (64-bit) code
- Supports SSE, AVX, and AVX-512 vector instructions
- Targets Linux (ELF32/ELF64) and Windows (PE32/PE64)
- Also outputs raw binary
- Multi-pass assembler (up to 100 passes by default)
- Macro preprocessor with `macro`, `repeat`, `if`/`end if`, and `include`
- Command-line symbol definitions (`-d`)
- Symbol table dump (`-s`)
- Configurable include search path (`-i`)
- Self-hosting: can rebuild itself without any external tools

---

## Building

### Bootstrap (first time)

If you don't have a tinyasm binary yet, build it with an existing assembler:

```sh
# Linux 64-bit
fasm tinyasm.asm tinyasm
chmod +x tinyasm

# Windows 64-bit
fasm -d WIN32=1 tinyasm.asm tinyasm.exe

# Linux 32-bit (separate entry file)
fasm tinyasm32.asm tinyasm32
chmod +x tinyasm32
```

### Self-hosting

Once you have a tinyasm binary, it can rebuild itself:

```sh
# Linux 64-bit
./tinyasm tinyasm.asm tinyasm

# Windows 64-bit
./tinyasm -d WIN32=1 tinyasm.asm tinyasm.exe

# Linux 32-bit
./tinyasm tinyasm32.asm tinyasm32
```

---

## Usage

```
tinyasm <source> [output]
```

If you omit the output filename, tinyasm derives it from the source filename and appends an extension based on the `format` directive in your source:

| Format | Auto extension |
|---|---|
| `format elf64 executable` / `format elf executable` | *(none)* |
| `format elf64` / `format elf` (object) | `.o` |
| `format pe64` / `format pe` (executable) | `.exe` |
| `format pe64` / `format pe` (DLL) | `.dll` |
| `format pe64` (driver, subsystem 1) | `.sys` |
| `format pe64` (EFI, subsystem ≥ 10) | `.efi` |
| `format binary` | *(source extension replaced, no new extension)* |

### Options

| Flag | Description |
|------|-------------|
| `-m <kb>` | Memory limit in kilobytes (default: 16384 = 16 MB) |
| `-p <n>` | Max number of passes (default: 100) |
| `-d <n>=<value>` | Define a symbolic variable from the command line |
| `-s <file>` | Dump symbol table to a file |
| `-i <path>` | Add a directory to the include search path |

Options are case-insensitive: `-m` and `-M` are equivalent.

### Include search path

When resolving an `include` directive, tinyasm searches in this order:

1. Directory of the file containing the `include`
2. Paths from the `INCLUDE` environment variable (semicolon-separated)
3. Paths added via `-i` flags (in the order given)

### Exit code

tinyasm exits with the number of passes performed — a two-pass build exits 2, three-pass exits 3, and so on. On error it exits non-zero. Scripts should treat any exit code as success if the output file was produced, or check stderr for error messages.

### Examples

```sh
# Assemble hello.asm → hello (ELF64 executable)
./tinyasm hello_elf64.asm hello64
chmod +x hello64
./hello64

# Let tinyasm derive the output name
./tinyasm hello_elf64.asm

# Pass a define at build time
./tinyasm -d DEBUG=1 main.asm main

# Conditional assembly
./tinyasm -d VERSION=2 main.asm main

# Limit memory to 8 MB, max 50 passes
./tinyasm -m 8192 -p 50 main.asm main

# Include directories
./tinyasm -i ./include -i /usr/local/share/asm main.asm main

# Combine INCLUDE env var with -i
INCLUDE=/usr/share/asm ./tinyasm -i ./include main.asm main

# Dump symbol table
./tinyasm -s symbols.txt main.asm main
```

---

## Testing the examples

The repo includes ready-to-build example files. All can be assembled with either `tinyasm` (64-bit) or `tinyasm32` (32-bit):

### Linux ELF32 executable

```sh
./tinyasm32 hello_elf32.asm hello32
chmod +x hello32
./hello32
```

### Linux ELF64 executable

```sh
./tinyasm32 hello_elf64.asm hello64
chmod +x hello64
./hello64
```

### Linux ELF64 object file

```sh
./tinyasm32 hello_elf64_obj.asm hello64.o
ld hello64.o -o hello64_linked
chmod +x hello64_linked
./hello64_linked
```

### Windows PE32 executable

```sh
./tinyasm32 hello_pe32.asm hello32.exe
# Run on Windows or Wine:
wine hello32.exe
```

### Windows PE64 executable

```sh
./tinyasm32 hello_pe64.asm hello64.exe
# Run on Windows or Wine:
wine hello64.exe
```

### Raw binary (MBR bootloader)

```sh
./tinyasm32 hello_bin.asm hello.bin
# Inspect:
xxd hello.bin | head
# Run in QEMU:
qemu-system-x86_64 -drive format=raw,file=hello.bin
```

---

## Syntax

### Hello World (Linux ELF64)

```asm
format elf64 executable 3
entry _start

segment readable executable

_start:
    mov eax, 1          ; sys_write
    mov edi, 1          ; stdout
    lea rsi, [msg]
    mov edx, msg_len
    syscall

    mov eax, 60         ; sys_exit
    xor edi, edi
    syscall

segment readable

msg     db 'hello, world', 10
msg_len = $ - msg
```

### Data definitions

```asm
db  0x41, 'A', "hello"   ; bytes
dw  1234                 ; 16-bit word
dd  0xDEADBEEF           ; 32-bit dword
dq  0x123456789ABCDEF0   ; 64-bit qword

here:
    db 'data'
size = $ - here
```

### Macros

```asm
macro push_all {
    push rax
    push rbx
    push rcx
}

push_all
```

### Conditional assembly

```asm
if defined DEBUG
    ; emit debug output
end if

if defined VERSION
    mov eax, VERSION
end if
```

### Repeat blocks

```asm
repeat 16
    db 0
end repeat
```

### Including files

```asm
include 'mylib.inc'
```

---

## Output formats

> **Note:** all `format` keywords are lowercase.

| Directive | Output |
|---|---|
| `format elf64 executable 3` | Linux 64-bit executable |
| `format elf executable 3` | Linux 32-bit executable |
| `format elf64` | Linux 64-bit object file |
| `format elf` | Linux 32-bit object file |
| `format pe64 console 5.0` | Windows 64-bit console app |
| `format pe64 GUI 5.0` | Windows 64-bit app (no console window) |
| `format pe console 5.0` | Windows 32-bit console app |
| `format binary` | Raw flat binary, no headers |

> **Note on `GUI` subsystem:** `format pe64 GUI 5.0` does not provide any GUI framework. It only tells Windows not to allocate a console window. Any actual windowing (buttons, dialogs, etc.) must be done manually via Win32 API calls.

---

## Error messages

tinyasm reports descriptive errors to stderr. Common messages and their meanings:

| Message | Meaning |
|---|---|
| `source file not found` | Input `.asm` file does not exist |
| `file not found` | An `include`d file could not be located |
| `out of memory` | Exceeded the memory limit; try `-m` to increase it |
| `code generation not possible` | A forward reference could not be resolved after max passes |
| `output format limit exceeded` | Output binary exceeds format constraints |
| `unknown instruction` | Unrecognised mnemonic or directive |
| `invalid operand` | Operand type not valid for this instruction |
| `operand size invalid` | Size override is not permitted here |
| `operand size missing` | Size is ambiguous; add `byte`/`word`/`dword`/`qword` |
| `operand size mismatch` | Source and destination sizes differ |
| `immediate value too large to encode` | Constant does not fit in the encoding |
| `jump target out of range` | Short or near jump cannot reach the target |
| `undefined symbol` | A label or constant was used before being defined |
| `symbol redefined` | Same name declared more than once |
| `unterminated macro` | `macro` block has no matching `}` or `end` |
| `bad macro arguments` | Wrong number or type of arguments to a macro |
| `malformed expression` | Expression syntax error |
| `invalid identifier` | Label name contains illegal characters |
| `reserved word used as label` | A keyword was used as a symbol name |
| `repeat count too large` | `repeat` count exceeds the limit |
| `assertion failed` | An `assert` directive evaluated to false |
| `output write failed` | Could not write the output file (disk full, permission denied, etc.) |

---

## Project structure

```
tinyasm.asm           entry point: startup, argument parsing, assembly pipeline
tinyasm32.asm         entry point for Linux 32-bit build
core/
  platform.inc        compatibility macros (use32/use64 bridging)
  platform32.inc      32-bit platform compatibility macros
  linux.inc           Linux 64-bit platform layer: syscalls for file I/O, memory, buffered output
  linux32.inc         Linux 32-bit platform layer
  win32.inc           Windows platform layer: CreateFile, VirtualAlloc, WriteConsole, etc.
  ver.inc             version constants (VERSION_MAJOR, VERSION_MINOR, VERSION_STRING)
  state.inc           assembler state variables
  fault.inc           error dispatch: maps error conditions to messages and exit
  msgdata.inc         error message strings
  expand.inc          preprocessor: macro expansion, include resolution, definitions
  scan.inc            source scanner and parser
  tokens.inc          expression tokenizer and evaluator
  emit.inc            instruction emitter
  calc.inc            expression calculator
  output_fmt.inc      output format writers: ELF, PE, binary
  structs.inc         instruction encoding tables and opcode data
  dump.inc            symbol table dump (-s flag)
arch/
  x86.inc             x86 / x64 instruction encoding
  vec.inc             SSE / AVX / AVX-512 vector instruction encoding
```

---

## License

See [LICENSE](LICENSE).

---

> tinyasm is derived from [fasm (flat assembler)](https://flatassembler.net) by Tomasz Grysztar.
