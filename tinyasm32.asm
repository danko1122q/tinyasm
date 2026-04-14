; tinyasm 32-bit Linux entry point
; Derived from fasm/source/linux/fasm.asm by Tomasz Grysztar
; Build: fasm tinyasm32.asm tinyasm32 && chmod +x tinyasm32

	format	ELF executable 3
	entry	ta_start

	include 'core/platform32.inc'

segment readable executable

ta_start:

	mov	[ta_con_handle],1
	mov	esi,_ta_logo
	call	ta_display_string

	; 32-bit Linux argv layout: [esp] = argc, [esp+4] = argv[0], ...
	mov	[ta_command_line],esp
	mov	ecx,[esp]
	lea	ebx,[esp+4+ecx*4+4]
	mov	[ta_environment],ebx
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

	; gettimeofday (sys 78) -> buffer: [seconds dd, useconds dd]
	mov	eax,78
	mov	ebx,ta_buffer
	xor	ecx,ecx
	int	0x80
	mov	eax,dword [ta_buffer]
	mov	ecx,1000
	mul	ecx
	mov	ebx,eax
	mov	eax,dword [ta_buffer+4]
	mov	ecx,1000
	div	ecx
	add	eax,ebx
	mov	[ta_start_time],eax

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
	mov	eax,78
	mov	ebx,ta_buffer
	xor	ecx,ecx
	int	0x80
	mov	eax,dword [ta_buffer]
	mov	ecx,1000
	mul	ecx
	mov	ebx,eax
	mov	eax,dword [ta_buffer+4]
	mov	ecx,1000
	div	ecx
	add	eax,ebx
	sub	eax,[ta_start_time]
	jnc	ta_time_ok
	add	eax,3600000
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
	mov	ebx,[ta_command_line]
	mov	[ta_input_file],0
	mov	[ta_output_file],0
	mov	[ta_symbols_file],0
	mov	[ta_memory_setting],0
	mov	[ta_passes_limit],100
	mov	ecx,[ebx]
	add	ebx,8
	dec	ecx
	jz	ta_bad_params
	mov	[ta_definitions_pointer],ta_predefinitions
	mov	[ta_path_pointer],ta_paths
	mov	[ta_include_extra_ptr],ta_include_extra
	mov	byte [ta_include_extra],0
      ta_get_param:
	mov	esi,[ebx]
	mov	al,[esi]
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
	inc	esi
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
	cmp	byte [esi],0
	jne	ta_get_memory_setting
	dec	ecx
	jz	ta_bad_params
	add	ebx,4
	mov	esi,[ebx]
      ta_get_memory_setting:
	call	ta_get_option_value
	or	edx,edx
	jz	ta_bad_params
	cmp	edx,1 shl (32-10)
	jae	ta_bad_params
	mov	[ta_memory_setting],edx
	jmp	ta_next_param
      ta_passes_option:
	cmp	byte [esi],0
	jne	ta_get_passes_setting
	dec	ecx
	jz	ta_bad_params
	add	ebx,4
	mov	esi,[ebx]
      ta_get_passes_setting:
	call	ta_get_option_value
	or	edx,edx
	jz	ta_bad_params
	cmp	edx,10000h
	ja	ta_bad_params
	mov	[ta_passes_limit],dx
      ta_next_param:
	add	ebx,4
	dec	ecx
	jnz	ta_get_param
	cmp	[ta_input_file],0
	je	ta_bad_params
	mov	eax,[ta_definitions_pointer]
	mov	byte [eax],0
	mov	[ta_initial_definitions],ta_predefinitions
	clc
	ret
      ta_definition_option:
	cmp	byte [esi],0
	jne	ta_get_definition
	dec	ecx
	jz	ta_bad_params
	add	ebx,4
	mov	esi,[ebx]
      ta_get_definition:
	push	edi
	mov	edi,[ta_definitions_pointer]
	call	ta_convert_definition_option
	mov	[ta_definitions_pointer],edi
	pop	edi
	jc	ta_bad_params
	jmp	ta_next_param
      ta_symbols_option:
	cmp	byte [esi],0
	jne	ta_get_symbols_setting
	dec	ecx
	jz	ta_bad_params
	add	ebx,4
	mov	esi,[ebx]
      ta_get_symbols_setting:
	call	ta_collect_path
	mov	[ta_symbols_file],edx
	jmp	ta_next_param
      ta_include_option:
	cmp	byte [esi],0
	jne	ta_get_include_setting
	dec	ecx
	jz	ta_bad_params
	add	ebx,4
	mov	esi,[ebx]
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
	dec	esi
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
	dec	esi
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

include 'core/linux32.inc'

include 'core/ver.inc'

_ta_copyright db 'tinyasm project',0xA,0

_ta_logo db 'tinyasm  version ',VERSION_STRING,' (32-bit)',0
_ta_usage db 0xA
       db 'usage: tinyasm32 <source> [output]',0xA
       db 'optional settings:',0xA
       db ' -m <limit>         set the limit in kilobytes for the available memory',0xA
       db ' -p <limit>         set the maximum allowed number of passes',0xA
       db ' -d <n>=<value>  define symbolic variable',0xA
       db ' -s <file>          dump symbolic information for debugging',0xA
       db ' -i <path>          add directory to include search path',0xA
       db 0
_ta_memory_prefix db '  (',0
_ta_memory_suffix db ' kilobytes memory, x86)',0xA,0
_ta_passes_suffix db ' passes, ',0
_ta_seconds_suffix db ' ms, ',0
_ta_bytes_suffix db ' bytes.',0xA,0

include 'core/fault.inc'
include 'core/dump.inc'
include 'core/expand.inc'
include 'core/scan.inc'
include 'core/tokens.inc'
include 'core/emit.inc'
include 'core/calc.inc'
include 'arch/x86.inc'
include 'arch/vec.inc'
include 'core/output_fmt.inc'

include 'core/structs.inc'
include 'core/msgdata.inc'

segment readable writeable

align 4

include 'core/state.inc'

ta_command_line dd ?
ta_memory_setting dd ?
ta_path_pointer dd ?
ta_definitions_pointer dd ?
ta_environment dd ?
ta_timestamp dq ?
ta_start_time dd ?
ta_con_handle dd ?
ta_displayed_count dd ?
ta_last_displayed db ?
ta_character db ?
ta_preprocessing_done db ?

ta_buffer rb 1000h
ta_predefinitions rb 1000h
ta_paths rb 10000h
ta_include_extra rb 4000h
ta_include_extra_ptr dd ?
