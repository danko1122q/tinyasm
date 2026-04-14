; tinyasm Win32 platform layer
; Drop-in replacement for core/linux.inc when targeting Windows.
; All symbols use the ta_ prefix to match tinyasm naming conventions.

; ---- Win32 constants -------------------------------------------------------

GENERIC_READ           = 80000000h
GENERIC_WRITE          = 40000000h
FILE_SHARE_READ        = 1
CREATE_ALWAYS          = 2
OPEN_EXISTING          = 3
FILE_BEGIN             = 0
FILE_CURRENT           = 1
FILE_END               = 2
STD_OUTPUT_HANDLE      = 0FFFFFFF5h
STD_ERROR_HANDLE       = 0FFFFFFF4h
MEM_COMMIT             = 1000h
MEM_RESERVE            = 2000h
MEM_RELEASE            = 8000h
PAGE_READWRITE         = 4
INVALID_HANDLE_VALUE   = -1

; ---------------------------------------------------------------------------
; ta_init_memory
;   Mirrors the brk-based logic in linux.inc exactly.
;   Allocates via VirtualAlloc, splits into heap (lower 3/4) + auxmem (upper 1/4).
; ---------------------------------------------------------------------------
ta_init_memory:
	mov	eax,esp
	and	eax,not 0FFFh
	add	eax,1000h-10000h
	mov	[ta_stack_limit],eax
	mov	ecx,[ta_memory_setting]
	shl	ecx,10
	jnz	.ta_do_alloc
	mov	ecx,4000000h		; default 64 MiB
  .ta_do_alloc:
	push	PAGE_READWRITE
	push	MEM_COMMIT+MEM_RESERVE
	push	ecx
	push	0
	call	[VirtualAlloc]
	or	eax,eax
	jz	ta_out_of_memory
	mov	[ta_additional_memory],eax
	; split: lower 3/4 = heap, upper 1/4 = auxmem
	mov	edx,ecx
	shr	edx,2			; edx = size/4
	add	edx,[ta_additional_memory]
	mov	[ta_additional_memory_end],edx
	mov	[ta_memory_start],edx
	mov	edx,[ta_additional_memory]
	add	edx,ecx
	mov	[ta_memory_end],edx
	ret

; ---------------------------------------------------------------------------
; ta_exit_program  al = exit code
; ---------------------------------------------------------------------------
ta_exit_program:
	movzx	eax,al
	mov	ecx,[ta_additional_memory]
	test	ecx,ecx
	jz	.ta_do_exit
	push	eax
	push	MEM_RELEASE
	push	0
	push	ecx
	call	[VirtualFree]
	pop	eax
  .ta_do_exit:
	push	eax
	call	[ExitProcess]

; ---------------------------------------------------------------------------
; ta_win32_init_argv
;   Parses GetCommandLineA() into argc/argv layout compatible with linux.inc.
;   Sets ta_command_line to point at our synthetic argc/argv block.
; ---------------------------------------------------------------------------
ta_win32_init_argv:
	push	ebx ecx edx esi edi ebp

	call	[GetCommandLine]	; eax -> "prog.exe arg1 arg2\0"
	mov	esi,eax

	lea	edi,[ta_w32_argv_str]
	lea	ebx,[ta_w32_argc_buf+4]	; first argv slot
	xor	ecx,ecx			; argc = 0

  .ta_skip_ws:
	lodsb
	cmp	al,' '
	je	.ta_skip_ws
	cmp	al,9			; TAB
	je	.ta_skip_ws
	or	al,al
	jz	.ta_done

	mov	[ebx],edi
	add	ebx,4
	inc	ecx

  .ta_token_chr:
	cmp	al,'"'
	je	.ta_quoted
	cmp	al,' '
	je	.ta_token_end
	cmp	al,9
	je	.ta_token_end
	or	al,al
	jz	.ta_token_nul
	stosb
	lodsb
	jmp	.ta_token_chr

  .ta_quoted:
	lodsb
  .ta_quoted_chr:
	or	al,al
	jz	.ta_token_nul
	cmp	al,'"'
	jne	.ta_quoted_store
	cmp	byte [esi],'"'
	jne	.ta_quoted_end
	inc	esi
  .ta_quoted_store:
	stosb
	lodsb
	jmp	.ta_quoted_chr
  .ta_quoted_end:
	lodsb
	jmp	.ta_token_chr

  .ta_token_end:
	xor	al,al
	stosb
	jmp	.ta_skip_ws

  .ta_token_nul:
	xor	al,al
	stosb

  .ta_done:
	mov	dword [ebx],0
	mov	dword [ta_w32_argc_buf],ecx
	lea	eax,[ta_w32_argc_buf]
	mov	[ta_command_line],rax

	pop	ebp edi esi edx ecx ebx
	ret

; ---------------------------------------------------------------------------
; ta_get_environment_variable
;   esi = pointer to null-terminated variable name
;   edi = destination buffer
;   Mirrors linux.inc walk-environ logic via GetEnvironmentVariableA.
; ---------------------------------------------------------------------------
ta_get_environment_variable:
	push	ebx ecx edx
	mov	ecx,[ta_memory_end]
	sub	ecx,edi
	cmp	ecx,4000h
	jbe	.ta_size_ok
	mov	ecx,4000h
  .ta_size_ok:
	push	ecx
	push	edi
	push	esi
	call	[GetEnvironmentVariable]
	or	eax,eax
	jz	.ta_miss
	add	edi,eax
	cmp	edi,[ta_memory_end]
	jae	ta_out_of_memory
  .ta_miss:
	pop	edx ecx ebx
	ret

; ---------------------------------------------------------------------------
; ta_open  edx = null-terminated path
;          returns ebx = HANDLE, CF set on error
; ---------------------------------------------------------------------------
ta_open:
	mov	r12d,esi
	mov	r13d,edi
	call	ta_adapt_path
	push	0			; hTemplateFile
	push	0			; dwFlagsAndAttributes
	push	OPEN_EXISTING
	push	0			; lpSecurityAttributes
	push	FILE_SHARE_READ
	push	GENERIC_READ
	push	ta_buffer
	call	[CreateFile]
	mov	esi,r12d
	mov	edi,r13d
	cmp	eax,INVALID_HANDLE_VALUE
	je	ta_file_error
	mov	ebx,eax
	clc
	ret

    ta_adapt_path:
	mov	esi,edx
	mov	edi,ta_buffer
      ta_copy_path:
	lods	byte [esi]
	cmp	al,'\'
	jne	ta_path_char_ok
	mov	al,'/'
      ta_path_char_ok:
	stos	byte [edi]
	or	al,al
	jnz	ta_copy_path
	cmp	edi,ta_buffer+1000h
	ja	ta_out_of_memory
	ret

; ---------------------------------------------------------------------------
; ta_create  edx = null-terminated path
;            returns ebx = HANDLE, CF set on error
; ---------------------------------------------------------------------------
ta_create:
	mov	r12d,esi
	mov	r13d,edi
	call	ta_adapt_path
	push	0			; hTemplateFile
	push	80h			; FILE_ATTRIBUTE_NORMAL
	push	CREATE_ALWAYS
	push	0
	push	FILE_SHARE_READ
	push	GENERIC_WRITE
	push	ta_buffer
	call	[CreateFile]
	mov	esi,r12d
	mov	edi,r13d
	cmp	eax,INVALID_HANDLE_VALUE
	je	ta_file_error
	mov	ebx,eax
	clc
	ret

; ---------------------------------------------------------------------------
; ta_close  ebx = handle
; ---------------------------------------------------------------------------
ta_close:
	mov	r13d,edi
	push	ebx
	call	[CloseHandle]
	mov	edi,r13d
	ret

; ---------------------------------------------------------------------------
; ta_read  ebx=handle  edx=buf  ecx=count
;          CF set on error or short read
; ---------------------------------------------------------------------------
ta_read:
	mov	r12d,esi
	mov	r13d,edi
	push	0			; lpOverlapped
	push	ta_bytes_count
	push	ecx
	push	edx
	push	ebx
	call	[ReadFile]
	mov	esi,r12d
	mov	edi,r13d
	or	eax,eax
	jz	ta_file_error
	cmp	ecx,[ta_bytes_count]
	jne	ta_file_error
	clc
	ret
    ta_file_error:
	stc
	ret

; ---------------------------------------------------------------------------
; ta_write  ebx=handle  edx=buf  ecx=count
;           CF set on error
; ---------------------------------------------------------------------------
ta_write:
	mov	r12d,esi
	mov	r13d,edi
	push	0
	push	ta_bytes_count
	push	ecx
	push	edx
	push	ebx
	call	[WriteFile]
	mov	esi,r12d
	mov	edi,r13d
	or	eax,eax
	jz	ta_file_error
	clc
	ret

; ---------------------------------------------------------------------------
; ta_lseek  ebx=handle  edx=offset  al=whence(0=SET,1=CUR,2=END)
;           returns eax=new position, CF set on error
; ---------------------------------------------------------------------------
ta_lseek:
	mov	r12d,esi
	mov	r13d,edi
	movzx	eax,al
	push	eax			; dwMoveMethod
	push	0			; lDistanceToMoveHigh
	push	edx
	push	ebx
	call	[SetFilePointer]
	mov	esi,r12d
	mov	edi,r13d
	cmp	eax,INVALID_HANDLE_VALUE
	je	ta_file_error
	clc
	ret

; ---------------------------------------------------------------------------
; ta_display_string  esi = null-terminated string
; ---------------------------------------------------------------------------
ta_display_string:
	mov	edi,esi
	mov	edx,esi
	or	ecx,-1
	xor	al,al
	repne	scasb
	neg	ecx
	sub	ecx,2
	jle	.ta_done
	push	0
	push	ta_bytes_count
	push	ecx
	push	edx
	push	[ta_con_handle]
	call	[WriteFile]
  .ta_done:
	ret

; ---------------------------------------------------------------------------
; ta_display_character  dl = character
; ---------------------------------------------------------------------------
ta_display_character:
	mov	r12d,esi
	mov	r13d,edi
	mov	[ta_character],dl
	push	0
	push	ta_bytes_count
	push	1
	push	ta_character
	push	[ta_con_handle]
	call	[WriteFile]
	mov	esi,r12d
	mov	edi,r13d
	ret

; ---------------------------------------------------------------------------
; ta_display_number  eax = uint32
; ---------------------------------------------------------------------------
ta_display_number:
	mov	r14d,ebx
	mov	ecx,1000000000
	xor	edx,edx
	xor	bl,bl
      ta_display_loop:
	div	ecx
	mov	r15d,edx
	cmp	ecx,1
	je	ta_display_digit
	or	bl,bl
	jnz	ta_display_digit
	or	al,al
	jz	ta_digit_ok
	not	bl
      ta_display_digit:
	mov	dl,al
	add	dl,30h
	mov	r10d,ecx
	call	ta_display_character
	mov	ecx,r10d
      ta_digit_ok:
	mov	eax,ecx
	xor	edx,edx
	mov	ecx,10
	div	ecx
	mov	ecx,eax
	mov	eax,r15d
	or	ecx,ecx
	jnz	ta_display_loop
	mov	ebx,r14d
	ret

; ---------------------------------------------------------------------------
; ta_display_user_messages
; ---------------------------------------------------------------------------
ta_display_user_messages:
	mov	[ta_displayed_count],0
	call	ta_show_display_buffer
	cmp	[ta_displayed_count],0
	je	ta_line_break_ok
	cmp	[ta_last_displayed],0Ah
	je	ta_line_break_ok
	mov	dl,0Ah
	call	ta_display_character
      ta_line_break_ok:
	ret

; ---------------------------------------------------------------------------
; ta_display_block  esi=buf  ecx=len
; ---------------------------------------------------------------------------
ta_display_block:
	jecxz	ta_block_displayed
	add	[ta_displayed_count],ecx
	mov	al,[esi+ecx-1]
	mov	[ta_last_displayed],al
	mov	r13d,edi
	push	0
	push	ta_bytes_count
	push	ecx
	push	esi
	push	[ta_con_handle]
	call	[WriteFile]
	mov	edi,r13d
      ta_block_displayed:
	ret

; ---------------------------------------------------------------------------
; ta_fatal_error
;   [esp] = pointer to error message string
; ---------------------------------------------------------------------------
ta_fatal_error:
	push	STD_ERROR_HANDLE
	call	[GetStdHandle]
	mov	[ta_con_handle],eax
	mov	esi,error_prefix
	call	ta_display_string
	pop	esi
	call	ta_display_string
	mov	esi,error_suffix
	call	ta_display_string
	mov	al,0FFh
	jmp	ta_exit_program

; ---------------------------------------------------------------------------
; ta_assembler_error
; ---------------------------------------------------------------------------
ta_assembler_error:
	push	STD_ERROR_HANDLE
	call	[GetStdHandle]
	mov	[ta_con_handle],eax
	call	ta_display_user_messages
	mov	ebx,[ta_current_line]
	test	ebx,ebx
	jz	ta_display_error_message
	push	dword 0
      ta_get_error_lines:
	mov	eax,[ebx]
	cmp	byte [eax],0
	je	ta_get_next_error_line
	push	ebx
	test	byte [ebx+7],80h
	jz	ta_display_error_line
	mov	edx,ebx
      ta_find_definition_origin:
	mov	edx,[edx+12]
	test	byte [edx+7],80h
	jnz	ta_find_definition_origin
	push	edx
      ta_get_next_error_line:
	mov	ebx,[ebx+8]
	jmp	ta_get_error_lines
      ta_display_error_line:
	mov	esi,[ebx]
	call	ta_display_string
	mov	esi,line_number_start
	call	ta_display_string
	mov	eax,[ebx+4]
	and	eax,7FFFFFFFh
	call	ta_display_number
	mov	dl,']'
	call	ta_display_character
	pop	esi
	cmp	ebx,esi
	je	ta_line_number_ok
	mov	dl,20h
	call	ta_display_character
	push	esi
	mov	esi,[esi]
	movzx	ecx,byte [esi]
	inc	esi
	call	ta_display_block
	mov	esi,line_number_start
	call	ta_display_string
	pop	esi
	mov	eax,[esi+4]
	and	eax,7FFFFFFFh
	call	ta_display_number
	mov	dl,']'
	call	ta_display_character
      ta_line_number_ok:
	mov	esi,line_data_start
	call	ta_display_string
	mov	esi,ebx
	mov	edx,[esi]
	call	ta_open
	mov	al,2
	xor	edx,edx
	call	ta_lseek
	mov	edx,[esi+8]
	sub	eax,edx
	jz	ta_line_data_displayed
	push	eax
	xor	al,al
	call	ta_lseek
	mov	ecx,[esp]
	mov	edx,[ta_additional_memory]
	lea	eax,[edx+ecx]
	cmp	eax,[ta_additional_memory_end]
	ja	ta_out_of_memory
	call	ta_read
	call	ta_close
	pop	ecx
	mov	esi,[ta_additional_memory]
      ta_get_line_data:
	mov	al,[esi]
	cmp	al,0Ah
	je	ta_display_line_data
	cmp	al,0Dh
	je	ta_display_line_data
	cmp	al,1Ah
	je	ta_display_line_data
	or	al,al
	jz	ta_display_line_data
	inc	esi
	loop	ta_get_line_data
      ta_display_line_data:
	mov	ecx,esi
	mov	esi,[ta_additional_memory]
	sub	ecx,esi
	call	ta_display_block
      ta_line_data_displayed:
	mov	esi,lf
	call	ta_display_string
	pop	ebx
	or	ebx,ebx
	jnz	ta_display_error_line
	cmp	[ta_preprocessing_done],0
	je	ta_display_error_message
	mov	esi,preprocessed_instruction_prefix
	call	ta_display_string
	mov	esi,[ta_current_line]
	add	esi,16
	mov	edi,[ta_additional_memory]
	xor	dl,dl
      ta_convert_instruction:
	lodsb
	cmp	al,1Ah
	je	ta_copy_symbol
	cmp	al,22h
	je	ta_copy_symbol
	cmp	al,3Bh
	je	ta_instruction_converted
	stosb
	or	al,al
	jz	ta_instruction_converted
	xor	dl,dl
	jmp	ta_convert_instruction
      ta_copy_symbol:
	or	dl,dl
	jz	ta_space_ok
	mov	byte [edi],20h
	inc	edi
      ta_space_ok:
	cmp	al,22h
	je	ta_quoted
	lodsb
	movzx	ecx,al
	rep	movsb
	or	dl,-1
	jmp	ta_convert_instruction
      ta_quoted:
	mov	al,27h
	stosb
	lodsd
	mov	ecx,eax
	jecxz	ta_quoted_copied
      ta_copy_quoted:
	lodsb
	stosb
	cmp	al,27h
	jne	ta_quote_ok
	stosb
      ta_quote_ok:
	loop	ta_copy_quoted
      ta_quoted_copied:
	mov	al,27h
	stosb
	or	dl,-1
	jmp	ta_convert_instruction
      ta_instruction_converted:
	xor	al,al
	stosb
	mov	esi,[ta_additional_memory]
	call	ta_display_string
	mov	esi,lf
	call	ta_display_string
      ta_display_error_message:
	mov	esi,error_prefix
	call	ta_display_string
	pop	esi
	call	ta_display_string
	mov	esi,error_suffix
	call	ta_display_string
	mov	al,2
	jmp	ta_exit_program

; ---------------------------------------------------------------------------
; ta_make_timestamp -> eax = milliseconds component, edx = seconds*1000
;   Uses GetSystemTime, mirrors linux.inc timing interface.
; ---------------------------------------------------------------------------
ta_make_timestamp:
	mov	r13d,edi
	sub	esp,16			; SYSTEMTIME buffer
	mov	esi,esp
	push	esi
	call	[GetSystemTime]

	; days since 1970-01-01 (same algorithm as FASM)
	movzx	ecx,word [esi]		; wYear
	mov	eax,ecx
	sub	eax,1970
	mov	ebx,365
	mul	ebx
	mov	ebp,eax

	mov	eax,ecx
	sub	eax,1969
	shr	eax,2
	add	ebp,eax

	mov	eax,ecx
	sub	eax,1901
	mov	ebx,100
	xor	edx,edx
	div	ebx
	sub	ebp,eax

	mov	eax,ecx
	xor	edx,edx
	sub	eax,1601
	mov	ebx,400
	div	ebx
	add	ebp,eax

	movzx	ecx,word [esi+2]	; wMonth
	mov	eax,ecx
	dec	eax
	mov	ebx,30
	mul	ebx
	add	ebp,eax

	cmp	ecx,8
	jbe	.ta_months_corr
	mov	eax,ecx
	sub	eax,7
	shr	eax,1
	add	ebp,eax
	mov	ecx,8
  .ta_months_corr:
	mov	eax,ecx
	shr	eax,1
	add	ebp,eax

	cmp	ecx,2
	jbe	.ta_day_corr_ok
	sub	ebp,2
	movzx	ecx,word [esi]
	test	ecx,11b
	jnz	.ta_day_corr_ok
	xor	edx,edx
	mov	eax,ecx
	mov	ebx,100
	div	ebx
	or	edx,edx
	jnz	.ta_day_corr
	mov	eax,ecx
	mov	ebx,400
	xor	edx,edx
	div	ebx
	or	edx,edx
	jnz	.ta_day_corr_ok
  .ta_day_corr:
	inc	ebp
  .ta_day_corr_ok:

	movzx	eax,word [esi+6]	; wDay
	dec	eax
	add	eax,ebp

	mov	ebx,24
	mul	ebx
	movzx	ecx,word [esi+8]	; wHour
	add	eax,ecx
	mov	ebx,60
	mul	ebx
	movzx	ecx,word [esi+10]	; wMinute
	add	eax,ecx
	mov	ebx,60
	mul	ebx
	movzx	ecx,word [esi+12]	; wSecond
	add	eax,ecx

	; multiply by 1000 for milliseconds
	mov	ecx,1000
	mul	ecx
	; add wMilliseconds
	movzx	ecx,word [esi+14]	; wMilliseconds
	add	eax,ecx

	add	esp,16
	mov	edi,r13d
	xor	edx,edx
	; store into ta_timestamp like linux.inc does
	mov	dword [ta_timestamp],eax
	mov	dword [ta_timestamp+4],edx
	ret

; ---- Shared string data ----------------------------------------------------

error_prefix db 'error: ',0
error_suffix db '.'
lf           db 0Dh,0Ah,0
line_number_start db ' [',0
line_data_start   db ':',0Dh,0Ah,0
preprocessed_instruction_prefix db 'processed: ',0

; ---- Win32 scratch buffers -------------------------------------------------

ta_bytes_count  dd ?
ta_w32_argc_buf rb 4 + 64*4 + 4
ta_w32_argv_str rb 4096

; ---- Win32 import table ----------------------------------------------------
; Declared here; actual .idata section goes in tinyasm.asm after .data

section '.idata' import data readable

  dd 0,0,0,RVA kernel32_name,RVA kernel32_table
  dd 0,0,0,0,0

  kernel32_table:
    CreateFile          dd RVA _CreateFile
    ReadFile            dd RVA _ReadFile
    WriteFile           dd RVA _WriteFile
    CloseHandle         dd RVA _CloseHandle
    SetFilePointer      dd RVA _SetFilePointer
    VirtualAlloc        dd RVA _VirtualAlloc
    VirtualFree         dd RVA _VirtualFree
    ExitProcess         dd RVA _ExitProcess
    GetStdHandle        dd RVA _GetStdHandle
    GetCommandLine      dd RVA _GetCommandLine
    GetEnvironmentVariable dd RVA _GetEnvironmentVariable
    GetSystemTime       dd RVA _GetSystemTime
  dd 0

  kernel32_name db 'KERNEL32.DLL',0

  _CreateFile             dw 0
    db 'CreateFileA',0
  _ReadFile               dw 0
    db 'ReadFile',0
  _WriteFile              dw 0
    db 'WriteFile',0
  _CloseHandle            dw 0
    db 'CloseHandle',0
  _SetFilePointer         dw 0
    db 'SetFilePointer',0
  _VirtualAlloc           dw 0
    db 'VirtualAlloc',0
  _VirtualFree            dw 0
    db 'VirtualFree',0
  _ExitProcess            dw 0
    db 'ExitProcess',0
  _GetStdHandle           dw 0
    db 'GetStdHandle',0
  _GetCommandLine         dw 0
    db 'GetCommandLineA',0
  _GetEnvironmentVariable dw 0
    db 'GetEnvironmentVariableA',0
  _GetSystemTime          dw 0
    db 'GetSystemTime',0
