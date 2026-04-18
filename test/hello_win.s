format PE64 console
entry start

section '.text' code readable executable

start:
    sub     rsp, 28h

    ; GetStdHandle(STD_OUTPUT_HANDLE) -> rax = handle
    mov     ecx, 0FFFFFFF5h         ; STD_OUTPUT_HANDLE (-11)
    call    [GetStdHandle]
    mov     rbx, rax                ; save handle

    ; WriteFile(handle, msg, msglen, &written, NULL)
    mov     rcx, rbx
    lea     rdx, [msg]
    mov     r8d, msglen
    lea     r9, [written]
    push    0                        ; lpOverlapped = NULL
    sub     rsp, 20h                 ; shadow space
    call    [WriteFile]
    add     rsp, 28h

    ; ExitProcess(0)
    xor     ecx, ecx
    sub     rsp, 28h
    call    [ExitProcess]

section '.data' data readable writeable
    msg     u8 'Hello, World from tinyasm!',0Dh,0Ah
    msglen = $ - msg
    written u32 0

section '.idata' import data readable

; IMAGE_IMPORT_DESCRIPTOR (all fields are DWORD = 4 bytes)
;   OriginalFirstThunk (ILT RVA - 0 = use FirstThunk)
;   TimeDateStamp
;   ForwarderChain
;   Name (RVA to DLL name string)
;   FirstThunk (IAT RVA)

    u32 RVA kernel32_ilt            ; OriginalFirstThunk
    u32 0                           ; TimeDateStamp
    u32 0                           ; ForwarderChain
    u32 RVA kernel32_name           ; Name
    u32 RVA kernel32_iat            ; FirstThunk (IAT)
    ; null terminator descriptor
    u32 0, 0, 0, 0, 0

; ILT (Import Lookup Table) - 64-bit RVAs to hint/name entries
kernel32_ilt:
    u64 RVA _GetStdHandle
    u64 RVA _WriteFile
    u64 RVA _ExitProcess
    u64 0

; IAT (Import Address Table) - filled by loader, same layout as ILT initially
kernel32_iat:
    GetStdHandle    u64 RVA _GetStdHandle
    WriteFile       u64 RVA _WriteFile
    ExitProcess     u64 RVA _ExitProcess
    u64 0

kernel32_name u8 'KERNEL32.DLL',0

; Hint/Name entries (u16 hint, then null-terminated name)
_GetStdHandle   u16 0
    u8 'GetStdHandle',0
_WriteFile      u16 0
    u8 'WriteFile',0
_ExitProcess    u16 0
    u8 'ExitProcess',0
