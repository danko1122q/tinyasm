; Platform selection:
;   Linux 64-bit (default):  fasm tinyasm.asm tinyasm
;   Windows 64-bit:          fasm -d WIN32=1 tinyasm.asm tinyasm.exe
;   Linux 32-bit:            fasm tinyasm32.asm tinyasm32  (separate entry file)
; Default: Linux ELF64

if defined WIN32
	format	PE64 GUI 5.0
	entry	ta_start
else
	format	ELF64 executable 3 at 400000h
	entry	ta_start
end if

	include 'core/platform.tny'

segment readable executable

ta_start:

if defined WIN32
	call	ta_win32_init_argv
	mov	[ta_con_handle],1
	push	STD_OUTPUT_HANDLE
	call	[GetStdHandle]
	mov	[ta_con_handle],eax
else
	mov	[ta_con_handle],1
end if
	mov	esi,_ta_logo
	call	ta_display_string

	mov	[ta_command_line],rsp
	mov	rcx,[rsp]
	lea	rbx,[rsp+8+rcx*8+8]
	mov	[ta_environment],rbx
	call	ta_get_params
	jc	ta_information

	call	ta_init_memory

	mov	esi,_ta_memory_prefix
	call	ta_display_string
	mov	eax,[ta_memory_end]
	sub	eax,[ta_memory_start]
	add	eax,[ta_additional_memory_end]
	sub	eax,[ta_additional_memory]
	shr	eax,10
	call	ta_display_number
	mov	esi,_ta_memory_suffix
	call	ta_display_string

	mov	eax,228
	mov	edi,1
	mov	rsi,ta_buffer
	syscall
	mov	rax,qword [ta_buffer]
	mov	rcx,1000
	mul	rcx
	mov	rbx,rax
	mov	rax,qword [ta_buffer+8]
	mov	rcx,1000000
	xor	rdx,rdx
	div	rcx
	add	rax,rbx
	mov	[ta_start_time],rax

	and	[ta_preprocessing_done],0
	call	ta_preprocessor
	or	[ta_preprocessing_done],-1
	call	ta_parser
	call	ta_assembler
	call	ta_formatter

	call	ta_display_user_messages
	movzx	eax,[ta_current_pass]
	inc	eax
	call	ta_display_number
	mov	esi,_ta_passes_suffix
	call	ta_display_string
	mov	eax,228
	mov	edi,1
	mov	rsi,ta_buffer
	syscall
	mov	rax,qword [ta_buffer]
	mov	rcx,1000
	mul	rcx
	mov	rbx,rax
	mov	rax,qword [ta_buffer+8]
	mov	rcx,1000000
	xor	rdx,rdx
	div	rcx
	add	rax,rbx
	sub	rax,[ta_start_time]
      ta_time_ok:
	call	ta_display_number
	mov	esi,_ta_seconds_suffix
	call	ta_display_string
      ta_display_bytes_count:
	mov	eax,[ta_written_size]
	call	ta_display_number
	mov	esi,_ta_bytes_suffix
	call	ta_display_string
	xor	al,al
	jmp	ta_exit_program

ta_information:
	mov	esi,_ta_usage
	call	ta_display_string
	mov	al,1
	jmp	ta_exit_program

ta_get_params:
	mov	rbx,[ta_command_line]
	mov	[ta_input_file],0
	mov	[ta_output_file],0
	mov	[ta_symbols_file],0
	mov	[ta_memory_setting],0
	mov	[ta_passes_limit],100
	mov	rcx,[rbx]
	add	rbx,8*2
	dec	rcx
	jz	ta_bad_params
	mov	[ta_definitions_pointer],ta_predefinitions
	mov	[ta_path_pointer],ta_paths
	mov	[ta_include_extra_ptr],ta_include_extra
	mov	byte [ta_include_extra],0
      ta_get_param:
	mov	rsi,[rbx]
	mov	al,[rsi]
	cmp	al,'-'
	je	ta_option_param
	cmp	[ta_input_file],0
	jne	ta_get_output_file
	call	ta_collect_path
	mov	[ta_input_file],edx
	jmp	ta_next_param
      ta_get_output_file:
	cmp	[ta_output_file],0
	jne	ta_bad_params
	call	ta_collect_path
	mov	[ta_output_file],edx
	jmp	ta_next_param
      ta_option_param:
	inc	rsi
	lodsb
	cmp	al,'m'
	je	ta_memory_option
	cmp	al,'M'
	je	ta_memory_option
	cmp	al,'p'
	je	ta_passes_option
	cmp	al,'P'
	je	ta_passes_option
	cmp	al,'d'
	je	ta_definition_option
	cmp	al,'D'
	je	ta_definition_option
	cmp	al,'s'
	je	ta_symbols_option
	cmp	al,'S'
	je	ta_symbols_option
	cmp	al,'i'
	je	ta_include_option
	cmp	al,'I'
	je	ta_include_option
      ta_bad_params:
	stc
	ret
      ta_memory_option:
	cmp	byte [rsi],0
	jne	ta_get_memory_setting
	dec	rcx
	jz	ta_bad_params
	add	rbx,8
	mov	rsi,[rbx]
      ta_get_memory_setting:
	call	ta_get_option_value
	or	edx,edx
	jz	ta_bad_params
	cmp	edx,1 shl (32-10)
	jae	ta_bad_params
	mov	[ta_memory_setting],edx
	jmp	ta_next_param
      ta_passes_option:
	cmp	byte [rsi],0
	jne	ta_get_passes_setting
	dec	rcx
	jz	ta_bad_params
	add	rbx,8
	mov	rsi,[rbx]
      ta_get_passes_setting:
	call	ta_get_option_value
	or	edx,edx
	jz	ta_bad_params
	cmp	edx,10000h
	ja	ta_bad_params
	mov	[ta_passes_limit],dx
      ta_next_param:
	add	rbx,8
	dec	rcx
	jnz	ta_get_param
	cmp	[ta_input_file],0
	je	ta_bad_params
	mov	eax,[ta_definitions_pointer]
	mov	byte [eax],0
	mov	[ta_initial_definitions],ta_predefinitions
	clc
	ret
      ta_definition_option:
	cmp	byte [rsi],0
	jne	ta_get_definition
	dec	rcx
	jz	ta_bad_params
	add	rbx,8
	mov	rsi,[rbx]
      ta_get_definition:
	mov	r12d,edi
	mov	edi,[ta_definitions_pointer]
	call	ta_convert_definition_option
	mov	[ta_definitions_pointer],edi
	mov	edi,r12d
	jc	ta_bad_params
	jmp	ta_next_param
      ta_symbols_option:
	cmp	byte [rsi],0
	jne	ta_get_symbols_setting
	dec	rcx
	jz	ta_bad_params
	add	rbx,8
	mov	rsi,[rbx]
      ta_get_symbols_setting:
	call	ta_collect_path
	mov	[ta_symbols_file],edx
	jmp	ta_next_param
      ta_include_option:
	cmp	byte [rsi],0
	jne	ta_get_include_setting
	dec	rcx
	jz	ta_bad_params
	add	rbx,8
	mov	rsi,[rbx]
      ta_get_include_setting:
	mov	edi,[ta_include_extra_ptr]
	cmp	edi,ta_include_extra+4000h
	jae	ta_bad_params
      ta_copy_include_path:
	lodsb
	or	al,al
	jz	ta_include_path_done
	stosb
	cmp	edi,ta_include_extra+4000h
	jb	ta_copy_include_path
	jmp	ta_bad_params
      ta_include_path_done:
	mov	al,';'
	stosb
	mov	[ta_include_extra_ptr],edi
	jmp	ta_next_param
      ta_get_option_value:
	xor	eax,eax
	mov	edx,eax
      ta_get_option_digit:
	lodsb
	cmp	al,20h
	je	ta_option_value_ok
	or	al,al
	jz	ta_option_value_ok
	sub	al,30h
	jc	ta_invalid_option_value
	cmp	al,9
	ja	ta_invalid_option_value
	imul	edx,10
	jo	ta_invalid_option_value
	add	edx,eax
	jc	ta_invalid_option_value
	jmp	ta_get_option_digit
      ta_option_value_ok:
	dec	rsi
	clc
	ret
      ta_invalid_option_value:
	stc
	ret
      ta_convert_definition_option:
	mov	edx,edi
	cmp	edi,ta_predefinitions+1000h
	jae	ta_bad_definition_option
	xor	al,al
	stosb
      ta_copy_definition_name:
	lodsb
	cmp	al,'='
	je	ta_copy_definition_value
	cmp	al,20h
	je	ta_bad_definition_option
	or	al,al
	jz	ta_bad_definition_option
	cmp	edi,ta_predefinitions+1000h
	jae	ta_bad_definition_option
	stosb
	inc	byte [edx]
	jnz	ta_copy_definition_name
      ta_bad_definition_option:
	stc
	ret
      ta_copy_definition_value:
	lodsb
	cmp	al,20h
	je	ta_definition_value_end
	or	al,al
	jz	ta_definition_value_end
	cmp	edi,ta_predefinitions+1000h
	jae	ta_bad_definition_option
	stosb
	jmp	ta_copy_definition_value
      ta_definition_value_end:
	dec	rsi
	cmp	edi,ta_predefinitions+1000h
	jae	ta_bad_definition_option
	xor	al,al
	stosb
	clc
	ret
ta_collect_path:
	mov	edi,[ta_path_pointer]
	mov	edx,edi
     ta_copy_path_to_low_memory:
	lodsb
	stosb
	test	al,al
	jnz	ta_copy_path_to_low_memory
	mov	[ta_path_pointer],edi
	retn

if defined WIN32
include 'core/win32.tny'
else

segment readable writeable
ta_out_buf_pos u32 ?
ta_out_buf rb TA_OUT_BUF_SIZE

segment readable executable
include 'core/linux.tny'
end if

include 'core/ver.tny'

_ta_copyright u8 'tinyasm project',0xA,0

_ta_logo u8 'tinyasm  version ',VERSION_STRING,0
_ta_usage u8 0xA
       u8 'usage: tinyasm <source> [output]',0xA
       u8 'optional settings:',0xA
       u8 ' -m <limit>         set the limit in kilobytes for the available memory',0xA
       u8 ' -p <limit>         set the maximum allowed number of passes',0xA
       u8 ' -d <name>=<value>     define symbolic variable',0xA
       u8 ' -s <file>          dump symbolic information for debugging',0xA
       u8 ' -i <path>          add directory to include search path',0xA
       u8 0
_ta_memory_prefix u8 '  (',0
_ta_memory_suffix u8 ' kilobytes memory, x64)',0xA,0
_ta_passes_suffix u8 ' passes, ',0
_ta_seconds_suffix u8 ' ms, ',0
_ta_bytes_suffix u8 ' bytes.',0xA,0
_ta_no_low_memory u8 'failed to allocate memory within 32-bit addressing range',0

include 'core/fault.tny'
include 'core/dump.tny'
include 'core/expand.tny'
include 'core/scan.tny'
include 'core/tokens.tny'
include 'core/emit.tny'
include 'core/calc.tny'
include 'arch/x86.tny'
include 'arch/vec.tny'
include 'core/output_fmt.tny'

include 'core/structs.tny'
include 'core/msgdata.tny'

segment readable writeable

align 4

include 'core/state.tny'

ta_command_line u64 ?
ta_memory_setting u32 ?
ta_path_pointer u32 ?
ta_definitions_pointer u32 ?
ta_environment u64 ?
ta_timestamp u64 ?
ta_start_time u64 ?
ta_con_handle u32 ?
ta_displayed_count u32 ?
ta_last_displayed u8 ?
ta_character u8 ?
ta_preprocessing_done u8 ?

ta_buffer rb 1000h
ta_predefinitions rb 1000h
ta_paths rb 10000h
ta_include_extra rb 4000h
ta_include_extra_ptr u32 ?
