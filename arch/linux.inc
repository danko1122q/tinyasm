O_ACCMODE  = 0003o
O_RDONLY   = 0000o
O_WRONLY   = 0001o
O_RDWR	   = 0002o
O_CREAT    = 0100o
O_EXCL	   = 0200o
O_NOCTTY   = 0400o
O_TRUNC    = 1000o
O_APPEND   = 2000o
O_NONBLOCK = 4000o

S_ISUID    = 4000o
S_ISGID    = 2000o
S_ISVTX    = 1000o
S_IRUSR    = 0400o
S_IWUSR    = 0200o
S_IXUSR    = 0100o
S_IRGRP    = 0040o
S_IWGRP    = 0020o
S_IXGRP    = 0010o
S_IROTH    = 0004o
S_IWOTH    = 0002o
S_IXOTH    = 0001o

ta_init_memory:
	mov	eax,esp
	and	eax,not 0FFFh
	add	eax,1000h-10000h
	mov	[ta_stack_limit],eax
	xor	edi,edi
	mov	eax,12
	syscall
	mov	ecx,[ta_memory_setting]
	shl	ecx,10
	jnz	ta_allocate_memory
	mov ecx,1000000h
      ta_allocate_memory:
	mov	r9d,eax
	cmp	rax,r9
	jne	ta_high_brk
	mov	[ta_additional_memory],eax
	lea	edi,[eax+ecx]
	mov	eax,12
	syscall
	mov	r9d,eax
	cmp	rax,r9
	jne	ta_no_low_memory
	mov	[ta_memory_end],eax
	sub	eax,[ta_additional_memory]
	shr	eax,2
	add	eax,[ta_additional_memory]
	mov	[ta_additional_memory_end],eax
	mov	[ta_memory_start],eax
	ret
      ta_high_brk:
	xor	r9d,r9d
	or	r8,-1
	mov	r10d,62h		; MAP_PRIVATE + MAP_ANONYMOUS + MAP_32BIT
	mov	edx,3			; PROT_READ + PROT_WRITE
	mov	esi,ecx
	xor	edi,edi
	mov	eax,9			; sys_mmap
	syscall
	cmp	eax,-1
	je	ta_mmap_with_hint
	mov	r9d,eax
	cmp	rax,r9
	jne	ta_mmap_unusable
	add	r9d,esi
	jnc	ta_mmap_ok
      ta_mmap_unusable:
	mov	rdi,rax
	mov	eax,11			; sys_munmap
	syscall
      ta_mmap_with_hint:
	mov	r10d,22h		; MAP_PRIVATE + MAP_ANONYMOUS
	mov	edx,3			; PROT_READ + PROT_WRITE
	mov	edi,480000h
	mov	eax,9			; sys_mmap
	syscall
	cmp	eax,-1
	je	ta_no_low_memory
	mov	r9d,eax
	cmp	rax,r9
	jne	ta_no_low_memory
	add	r9d,esi
	jnc	ta_mmap_ok
      ta_no_low_memory:
	mov	esi,lf
	call	ta_display_string
	push	_ta_no_low_memory
	jmp	ta_fatal_error
      ta_mmap_ok:
	mov	[ta_additional_memory],eax
	lea	edi,[eax+esi]
	mov	[ta_memory_end],edi
	shr	esi,2
	add	eax,esi
	mov	[ta_additional_memory_end],eax
	mov	[ta_memory_start],eax
	ret

ta_exit_program:
	call	ta_flush_output
	movzx	edi,al
	mov	eax,60
	syscall

ta_get_environment_variable:
	mov	ecx,esi
	mov	rbx,[ta_environment]
      ta_next_variable:
	mov	rsi,[rbx]
	test	rsi,rsi
	jz	ta_no_environment_variable
	add	rbx,8
      ta_compare_variable_names:
	mov	edx,ecx
      ta_compare_character:
	lodsb
	mov	ah,[edx]
	inc	edx
	cmp	al,'='
	je	ta_end_of_variable_name
	or	ah,ah
	jz	ta_next_variable
	sub	ah,al
	jz	ta_compare_character
	cmp	ah,20h
	jne	ta_next_variable
	cmp	al,41h
	jb	ta_next_variable
	cmp	al,5Ah
	jna	ta_compare_character
	jmp	ta_next_variable
      ta_no_environment_variable:
	ret
      ta_end_of_variable_name:
	or	ah,ah
	jnz	ta_next_variable
      ta_copy_variable_value:
	lodsb
	cmp	edi,[ta_memory_end]
	jae	ta_out_of_memory
	stosb
	or	al,al
	jnz	ta_copy_variable_value
	dec	edi
	ret

ta_open:
	mov	r12d,esi
	mov	r13d,edi
	; On Linux paths use '/' natively - pass directly without copy
	mov	eax,2
	mov	edi,edx		; path pointer directly
	mov	esi,O_RDONLY
	xor	edx,edx
	syscall
	mov	esi,r12d
	mov	edi,r13d
	test	eax,eax
	js	ta_file_error
	mov	ebx,eax
	clc
	ret
    ta_adapt_path:
	; Kept for ta_create which still uses ta_buffer for mode flags setup
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
ta_create:
	mov	r12d,esi
	mov	r13d,edi
	mov	r15d,edx
	call	ta_adapt_path
	mov	edi,ta_buffer
	mov	esi,O_CREAT+O_TRUNC+O_WRONLY
	mov	edx,S_IRUSR+S_IWUSR+S_IRGRP+S_IROTH
	cmp	r15d,[ta_output_file]
	jne	ta_do_create
	cmp	[ta_output_format],5
	jne	ta_do_create
	bt	[ta_format_flags],0
	jnc	ta_do_create
	or	edx,S_IXUSR+S_IXGRP+S_IXOTH
      ta_do_create:
	mov	eax,2
	syscall
	mov	esi,r12d
	mov	edi,r13d
	test	eax,eax
	js	ta_file_error
	mov	ebx,eax
	clc
	ret
ta_close:
	mov	r13d,edi
	mov	edi,ebx
	mov	eax,3
	syscall
	mov	edi,r13d
	ret
ta_read:
	mov	r12d,esi
	mov	r13d,edi
	mov	eax,0
	mov	edi,ebx
	mov	esi,edx
	mov	edx,ecx
	syscall
	mov	ecx,edx
	mov	edx,esi
	mov	esi,r12d
	mov	edi,r13d
	test	eax,eax
	js	ta_file_error
	cmp	eax,ecx
	jne	ta_file_error
	clc
	ret
    ta_file_error:
	stc
	ret
ta_write:
	mov	r12d,esi
	mov	r13d,edi
	mov	eax,1
	mov	edi,ebx
	mov	esi,edx
	mov	edx,ecx
	syscall
	mov	ecx,edx
	mov	edx,esi
	mov	esi,r12d
	mov	edi,r13d
	test	eax,eax
	js	ta_file_error
	cmp	eax,ecx		; verify all bytes were written (guard against partial write)
	jne	ta_file_error
	clc
	ret
ta_lseek:
	mov	r12d,esi
	mov	r13d,edi
	mov	edi,ebx
	mov	esi,edx
	xor	edx,edx
	mov	dl,al
	mov	eax,8
	syscall
	mov	esi,r12d
	mov	edi,r13d
	cmp	rax,-1		; check full 64-bit return value (lseek returns signed 64-bit offset)
	je	ta_file_error
	clc
	ret

; --- Buffered output ---
; All display calls write into ta_out_buf; ta_flush_output does one write syscall.

TA_OUT_BUF_SIZE = 4000h		; 16 KB output buffer

ta_display_string:
	push	esi edi
      ta_ds_loop:
	mov	al,[esi]
	or	al,al
	jz	ta_ds_done
	call	ta_buf_putc
	inc	esi
	jmp	ta_ds_loop
      ta_ds_done:
	pop	edi esi
	ret

ta_display_character:
	push	esi edi
	mov	al,dl
	call	ta_buf_putc
	pop	edi esi
	ret

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
	add	al,30h
	mov	r10d,ecx
	push	esi edi
	call	ta_buf_putc
	pop	edi esi
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

ta_buf_putc:
	; al = byte to buffer; trashes r8d
	mov	r8d,[ta_out_buf_pos]
	mov	[ta_out_buf+r8],al
	inc	r8d
	mov	[ta_out_buf_pos],r8d
	cmp	r8d,TA_OUT_BUF_SIZE
	jb	ta_buf_putc_done
	call	ta_flush_output
      ta_buf_putc_done:
	ret

ta_flush_output:
	mov	ecx,[ta_out_buf_pos]
	jecxz	ta_flush_done
	mov	eax,1
	mov	edi,[ta_con_handle]
	mov	esi,ta_out_buf
	mov	edx,ecx
	syscall
	mov	dword [ta_out_buf_pos],0
      ta_flush_done:
	ret

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
	call	ta_flush_output
	ret
ta_display_block:
	jecxz	ta_block_displayed
	add	[ta_displayed_count],ecx
	mov	al,[esi+ecx-1]
	mov	[ta_last_displayed],al
	push	esi edi ecx
      ta_display_block_loop:
	mov	al,[esi]
	call	ta_buf_putc
	inc	esi
	loop	ta_display_block_loop
	pop	ecx edi esi
      ta_block_displayed:
	ret

ta_fatal_error:
	call	ta_flush_output
	mov	[ta_con_handle],2
	mov	esi,error_prefix
	call	ta_display_string
	pop	esi
	call	ta_display_string
	mov	esi,error_suffix
	call	ta_display_string
	call	ta_flush_output
	mov	al,0FFh
	jmp	ta_exit_program
ta_assembler_error:
	call	ta_flush_output
	mov	[ta_con_handle],2
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

ta_make_timestamp:
	mov	r13d,edi
	mov	eax,201
	mov	edi,ta_timestamp
	syscall
	mov	eax,dword [ta_timestamp]
	mov	edx,dword [ta_timestamp+4]
	mov	edi,r13d
	ret

error_prefix db 'error: ',0
error_suffix db '.'
lf db 0xA,0
line_number_start db ' [',0
line_data_start db ':',0xA,0
preprocessed_instruction_prefix db 'processed: ',0
