ta_simple_instruction_except64:
	cmp	[ta_code_type],64
	je	ta_illegal_instruction
ta_simple_instruction:
	stos	byte [edi]
	jmp	ta_instruction_assembled
ta_simple_instruction_only64:
	cmp	[ta_code_type],64
	jne	ta_illegal_instruction
	jmp	ta_simple_instruction
ta_simple_instruction_16bit_except64:
	cmp	[ta_code_type],64
	je	ta_illegal_instruction
ta_simple_instruction_16bit:
	cmp	[ta_code_type],16
	jne	ta_size_prefix
	stos	byte [edi]
	jmp	ta_instruction_assembled
      ta_size_prefix:
	mov	ah,al
	mov	al,66h
	stos	word [edi]
	jmp	ta_instruction_assembled
ta_simple_instruction_32bit_except64:
	cmp	[ta_code_type],64
	je	ta_illegal_instruction
ta_simple_instruction_32bit:
	cmp	[ta_code_type],16
	je	ta_size_prefix
	stos	byte [edi]
	jmp	ta_instruction_assembled
ta_iret_instruction:
	cmp	[ta_code_type],64
	jne	ta_simple_instruction
ta_simple_instruction_64bit:
	cmp	[ta_code_type],64
	jne	ta_illegal_instruction
	mov	ah,al
	mov	al,48h
	stos	word [edi]
	jmp	ta_instruction_assembled
ta_simple_extended_instruction_64bit:
	cmp	[ta_code_type],64
	jne	ta_illegal_instruction
	mov	byte [edi],48h
	inc	edi
ta_simple_extended_instruction:
	mov	ah,al
	mov	al,0Fh
	stos	word [edi]
	jmp	ta_instruction_assembled
ta_simple_extended_instruction_f3:
	mov	byte [edi],0F3h
	inc	edi
	jmp	ta_simple_extended_instruction
ta_prefix_instruction:
	stos	byte [edi]
	or	[ta_prefix_flags],1
	jmp	ta_continue_line
ta_segment_prefix:
	mov	ah,al
	shr	ah,4
	cmp	ah,3
	jne	ta_illegal_instruction
	and	al,1111b
	mov	[ta_segment_register],al
	call	ta_store_segment_prefix
	or	[ta_prefix_flags],1
	jmp	ta_continue_line
ta_bnd_prefix_instruction:
	stos	byte [edi]
	or	[ta_prefix_flags],1 + 10h
	jmp	ta_continue_line
ta_int_instruction:
	call	ta_take_byte_value
	test	eax,eax
	jns	ta_int_imm_ok
	call	ta_recoverable_overflow
      ta_int_imm_ok:
	mov	ah,al
	mov	al,0CDh
	stos	word [edi]
	jmp	ta_instruction_assembled
    ta_take_byte_value:
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	ah,1
	ja	ta_invalid_operand_size
	cmp	al,'('
	jne	ta_invalid_operand
	call	ta_get_byte_value
	ret

ta_aa_instruction:
	cmp	[ta_code_type],64
	je	ta_illegal_instruction
	push	eax
	mov	bl,10
	cmp	byte [esi],'('
	jne	ta_aa_store
	inc	esi
	xor	al,al
	xchg	al,[ta_operand_size]
	cmp	al,1
	ja	ta_invalid_operand_size
	call	ta_get_byte_value
	mov	bl,al
      ta_aa_store:
	cmp	[ta_operand_size],0
	jne	ta_invalid_operand
	pop	eax
	mov	ah,bl
	stos	word [edi]
	jmp	ta_instruction_assembled

ta_basic_instruction:
	mov	[ta_base_code],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_basic_reg
	cmp	al,'['
	jne	ta_invalid_operand
      ta_basic_mem:
	call	ta_get_address
	push	edx ebx ecx
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'('
	je	ta_basic_mem_imm
	cmp	al,10h
	jne	ta_invalid_operand
      ta_basic_mem_reg:
	lods	byte [esi]
	call	ta_convert_register
	mov	[ta_postbyte_register],al
	pop	ecx ebx edx
	mov	al,ah
	cmp	al,1
	je	ta_instruction_ready
	call	ta_operand_autodetect
	inc	[ta_base_code]
      ta_instruction_ready:
	call	ta_store_instruction
	jmp	ta_instruction_assembled
      ta_basic_mem_imm:
	mov	al,[ta_operand_size]
	cmp	al,1
	jb	ta_basic_mem_imm_nosize
	je	ta_basic_mem_imm_8bit
	cmp	al,2
	je	ta_basic_mem_imm_16bit
	cmp	al,4
	je	ta_basic_mem_imm_32bit
	cmp	al,8
	jne	ta_invalid_operand_size
      ta_basic_mem_imm_64bit:
	cmp	[ta_size_declared],0
	jne	ta_long_immediate_not_encodable
	call	ta_operand_64bit
	call	ta_get_simm32
	cmp	[ta_value_type],4
	jae	ta_long_immediate_not_encodable
	jmp	ta_basic_mem_imm_32bit_ok
      ta_basic_mem_imm_nosize:
	call	ta_recoverable_unknown_size
      ta_basic_mem_imm_8bit:
	call	ta_get_byte_value
	mov	byte [ta_value],al
	mov	al,[ta_base_code]
	shr	al,3
	mov	[ta_postbyte_register],al
	pop	ecx ebx edx
	mov	[ta_base_code],80h
	call	ta_store_instruction_with_imm8
	jmp	ta_instruction_assembled
      ta_basic_mem_imm_16bit:
	call	ta_operand_16bit
	call	ta_get_word_value
	mov	word [ta_value],ax
	mov	al,[ta_base_code]
	shr	al,3
	mov	[ta_postbyte_register],al
	pop	ecx ebx edx
	cmp	[ta_value_type],0
	jne	ta_basic_mem_imm_16bit_store
	cmp	[ta_size_declared],0
	jne	ta_basic_mem_imm_16bit_store
	cmp	word [ta_value],80h
	jb	ta_basic_mem_simm_8bit
	cmp	word [ta_value],-80h
	jae	ta_basic_mem_simm_8bit
      ta_basic_mem_imm_16bit_store:
	mov	[ta_base_code],81h
	call	ta_store_instruction_with_imm16
	jmp	ta_instruction_assembled
      ta_basic_mem_simm_8bit:
	mov	[ta_base_code],83h
	call	ta_store_instruction_with_imm8
	jmp	ta_instruction_assembled
      ta_basic_mem_imm_32bit:
	call	ta_operand_32bit
	call	ta_get_dword_value
      ta_basic_mem_imm_32bit_ok:
	mov	dword [ta_value],eax
	mov	al,[ta_base_code]
	shr	al,3
	mov	[ta_postbyte_register],al
	pop	ecx ebx edx
	cmp	[ta_value_type],0
	jne	ta_basic_mem_imm_32bit_store
	cmp	[ta_size_declared],0
	jne	ta_basic_mem_imm_32bit_store
	cmp	dword [ta_value],80h
	jb	ta_basic_mem_simm_8bit
	cmp	dword [ta_value],-80h
	jae	ta_basic_mem_simm_8bit
      ta_basic_mem_imm_32bit_store:
	mov	[ta_base_code],81h
	call	ta_store_instruction_with_imm32
	jmp	ta_instruction_assembled
      ta_get_simm32:
	call	ta_get_qword_value
	mov	ecx,edx
	cdq
	cmp	ecx,edx
	je	ta_simm32_range_ok
	call	ta_recoverable_overflow
      ta_simm32_range_ok:
	cmp	[ta_value_type],4
	jne	ta_get_simm32_ok
	mov	[ta_value_type],2
      ta_get_simm32_ok:
	ret
      ta_basic_reg:
	lods	byte [esi]
	call	ta_convert_register
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_basic_reg_reg
	cmp	al,'('
	je	ta_basic_reg_imm
	cmp	al,'['
	jne	ta_invalid_operand
      ta_basic_reg_mem:
	call	ta_get_address
	mov	al,[ta_operand_size]
	cmp	al,1
	je	ta_basic_reg_mem_8bit
	call	ta_operand_autodetect
	add	[ta_base_code],3
	jmp	ta_instruction_ready
      ta_basic_reg_mem_8bit:
	add	[ta_base_code],2
	jmp	ta_instruction_ready
      ta_basic_reg_reg:
	lods	byte [esi]
	call	ta_convert_register
	mov	bl,[ta_postbyte_register]
	mov	[ta_postbyte_register],al
	mov	al,ah
	cmp	al,1
	je	ta_nomem_instruction_ready
	call	ta_operand_autodetect
	inc	[ta_base_code]
      ta_nomem_instruction_ready:
	call	ta_store_nomem_instruction
	jmp	ta_instruction_assembled
      ta_basic_reg_imm:
	mov	al,[ta_operand_size]
	cmp	al,1
	je	ta_basic_reg_imm_8bit
	cmp	al,2
	je	ta_basic_reg_imm_16bit
	cmp	al,4
	je	ta_basic_reg_imm_32bit
	cmp	al,8
	jne	ta_invalid_operand_size
      ta_basic_reg_imm_64bit:
	cmp	[ta_size_declared],0
	jne	ta_long_immediate_not_encodable
	call	ta_operand_64bit
	call	ta_get_simm32
	cmp	[ta_value_type],4
	jae	ta_long_immediate_not_encodable
	jmp	ta_basic_reg_imm_32bit_ok
      ta_basic_reg_imm_8bit:
	call	ta_get_byte_value
	mov	dl,al
	mov	bl,[ta_base_code]
	shr	bl,3
	xchg	bl,[ta_postbyte_register]
	or	bl,bl
	jz	ta_basic_al_imm
	mov	[ta_base_code],80h
	call	ta_store_nomem_instruction
	mov	al,dl
	stos	byte [edi]
	jmp	ta_instruction_assembled
      ta_basic_al_imm:
	mov	al,[ta_base_code]
	add	al,4
	stos	byte [edi]
	mov	al,dl
	stos	byte [edi]
	jmp	ta_instruction_assembled
      ta_basic_reg_imm_16bit:
	call	ta_operand_16bit
	call	ta_get_word_value
	mov	dx,ax
	mov	bl,[ta_base_code]
	shr	bl,3
	xchg	bl,[ta_postbyte_register]
	cmp	[ta_value_type],0
	jne	ta_basic_reg_imm_16bit_store
	cmp	[ta_size_declared],0
	jne	ta_basic_reg_imm_16bit_store
	cmp	dx,80h
	jb	ta_basic_reg_simm_8bit
	cmp	dx,-80h
	jae	ta_basic_reg_simm_8bit
      ta_basic_reg_imm_16bit_store:
	or	bl,bl
	jz	ta_basic_ax_imm
	mov	[ta_base_code],81h
	call	ta_store_nomem_instruction
      ta_basic_store_imm_16bit:
	mov	ax,dx
	call	ta_mark_relocation
	stos	word [edi]
	jmp	ta_instruction_assembled
      ta_basic_reg_simm_8bit:
	mov	[ta_base_code],83h
	call	ta_store_nomem_instruction
	mov	al,dl
	stos	byte [edi]
	jmp	ta_instruction_assembled
      ta_basic_ax_imm:
	add	[ta_base_code],5
	call	ta_store_classic_instruction_code
	jmp	ta_basic_store_imm_16bit
      ta_basic_reg_imm_32bit:
	call	ta_operand_32bit
	call	ta_get_dword_value
      ta_basic_reg_imm_32bit_ok:
	mov	edx,eax
	mov	bl,[ta_base_code]
	shr	bl,3
	xchg	bl,[ta_postbyte_register]
	cmp	[ta_value_type],0
	jne	ta_basic_reg_imm_32bit_store
	cmp	[ta_size_declared],0
	jne	ta_basic_reg_imm_32bit_store
	cmp	edx,80h
	jb	ta_basic_reg_simm_8bit
	cmp	edx,-80h
	jae	ta_basic_reg_simm_8bit
      ta_basic_reg_imm_32bit_store:
	or	bl,bl
	jz	ta_basic_eax_imm
	mov	[ta_base_code],81h
	call	ta_store_nomem_instruction
      ta_basic_store_imm_32bit:
	mov	eax,edx
	call	ta_mark_relocation
	stos	dword [edi]
	jmp	ta_instruction_assembled
      ta_basic_eax_imm:
	add	[ta_base_code],5
	call	ta_store_classic_instruction_code
	jmp	ta_basic_store_imm_32bit
      ta_recoverable_unknown_size:
	cmp	[ta_error_line],0
	jne	ta_ignore_unknown_size
	push	[ta_current_line]
	pop	[ta_error_line]
	mov	[ta_error],ta_operand_size_not_specified
      ta_ignore_unknown_size:
	ret
ta_single_operand_instruction:
	mov	[ta_base_code],0F6h
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_single_reg
	cmp	al,'['
	jne	ta_invalid_operand
      ta_single_mem:
	call	ta_get_address
	mov	al,[ta_operand_size]
	cmp	al,1
	je	ta_single_mem_8bit
	jb	ta_single_mem_nosize
	call	ta_operand_autodetect
	inc	[ta_base_code]
	jmp	ta_instruction_ready
      ta_single_mem_nosize:
	call	ta_recoverable_unknown_size
      ta_single_mem_8bit:
	jmp	ta_instruction_ready
      ta_single_reg:
	lods	byte [esi]
	call	ta_convert_register
	mov	bl,al
	mov	al,ah
	cmp	al,1
	je	ta_single_reg_8bit
	call	ta_operand_autodetect
	inc	[ta_base_code]
      ta_single_reg_8bit:
	jmp	ta_nomem_instruction_ready
ta_mov_instruction:
	mov	[ta_base_code],88h
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_mov_reg
	cmp	al,14h
	je	ta_mov_creg
	cmp	al,'['
	jne	ta_invalid_operand
      ta_mov_mem:
	call	ta_get_address
	push	edx ebx ecx
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'('
	je	ta_mov_mem_imm
	cmp	al,10h
	jne	ta_invalid_operand
      ta_mov_mem_reg:
	lods	byte [esi]
	cmp	al,30h
	jb	ta_mov_mem_general_reg
	cmp	al,40h
	jb	ta_mov_mem_sreg
      ta_mov_mem_general_reg:
	call	ta_convert_register
	mov	[ta_postbyte_register],al
	pop	ecx ebx edx
	cmp	ah,1
	je	ta_mov_mem_reg_8bit
	inc	[ta_base_code]
	mov	al,ah
	call	ta_operand_autodetect
	mov	al,[ta_postbyte_register]
	or	al,bl
	or	al,bh
	jz	ta_mov_mem_ax
	jmp	ta_instruction_ready
      ta_mov_mem_reg_8bit:
	or	al,bl
	or	al,bh
	jnz	ta_instruction_ready
      ta_mov_mem_al:
	test	ch,22h
	jnz	ta_mov_mem_address16_al
	test	ch,44h
	jnz	ta_mov_mem_address32_al
	test	ch,not 88h
	jnz	ta_invalid_address_size
	call	ta_check_mov_address64
	cmp	al,0
	jg	ta_mov_mem_address64_al
	jl	ta_instruction_ready
	cmp	[ta_code_type],16
	jne	ta_mov_mem_address32_al
	cmp	edx,10000h
	jb	ta_mov_mem_address16_al
      ta_mov_mem_address32_al:
	call	ta_store_segment_prefix_if_necessary
	call	ta_address_32bit_prefix
	mov	[ta_base_code],0A2h
      ta_store_mov_address32:
	call	ta_store_classic_instruction_code
	call	ta_store_address_32bit_value
	jmp	ta_instruction_assembled
      ta_mov_mem_address16_al:
	call	ta_store_segment_prefix_if_necessary
	call	ta_address_16bit_prefix
	mov	[ta_base_code],0A2h
      ta_store_mov_address16:
	cmp	[ta_code_type],64
	je	ta_invalid_address
	call	ta_store_classic_instruction_code
	mov	eax,edx
	stos	word [edi]
	cmp	edx,10000h
	jge	ta_value_out_of_range
	jmp	ta_instruction_assembled
      ta_check_mov_address64:
	cmp	[ta_code_type],64
	jne	ta_no_address64
	test	ch,88h
	jnz	ta_address64_required
	mov	eax,[ta_address_high]
	or	eax,eax
	jz	ta_no_address64
	bt	edx,31
	adc	eax,0
	jz	ta_address64_simm32
      ta_address64_required:
	mov	al,1
	ret
      ta_address64_simm32:
	mov	al,-1
	ret
      ta_no_address64:
	test	ch,08h
	jnz	ta_invalid_address_size
	xor	al,al
	ret
      ta_mov_mem_address64_al:
	call	ta_store_segment_prefix_if_necessary
	mov	[ta_base_code],0A2h
      ta_store_mov_address64:
	call	ta_store_classic_instruction_code
	call	ta_store_address_64bit_value
	jmp	ta_instruction_assembled
      ta_mov_mem_ax:
	test	ch,22h
	jnz	ta_mov_mem_address16_ax
	test	ch,44h
	jnz	ta_mov_mem_address32_ax
	test	ch,not 88h
	jnz	ta_invalid_address_size
	call	ta_check_mov_address64
	cmp	al,0
	jg	ta_mov_mem_address64_ax
	jl	ta_instruction_ready
	cmp	[ta_code_type],16
	jne	ta_mov_mem_address32_ax
	cmp	edx,10000h
	jb	ta_mov_mem_address16_ax
      ta_mov_mem_address32_ax:
	call	ta_store_segment_prefix_if_necessary
	call	ta_address_32bit_prefix
	mov	[ta_base_code],0A3h
	jmp	ta_store_mov_address32
      ta_mov_mem_address16_ax:
	call	ta_store_segment_prefix_if_necessary
	call	ta_address_16bit_prefix
	mov	[ta_base_code],0A3h
	jmp	ta_store_mov_address16
      ta_mov_mem_address64_ax:
	call	ta_store_segment_prefix_if_necessary
	mov	[ta_base_code],0A3h
	jmp	ta_store_mov_address64
      ta_mov_mem_sreg:
	sub	al,31h
	mov	[ta_postbyte_register],al
	pop	ecx ebx edx
	mov	ah,[ta_operand_size]
	or	ah,ah
	jz	ta_mov_mem_sreg_store
	cmp	ah,2
	jne	ta_invalid_operand_size
      ta_mov_mem_sreg_store:
	mov	[ta_base_code],8Ch
	jmp	ta_instruction_ready
      ta_mov_mem_imm:
	mov	al,[ta_operand_size]
	cmp	al,1
	jb	ta_mov_mem_imm_nosize
	je	ta_mov_mem_imm_8bit
	cmp	al,2
	je	ta_mov_mem_imm_16bit
	cmp	al,4
	je	ta_mov_mem_imm_32bit
	cmp	al,8
	jne	ta_invalid_operand_size
      ta_mov_mem_imm_64bit:
	cmp	[ta_size_declared],0
	jne	ta_long_immediate_not_encodable
	call	ta_operand_64bit
	call	ta_get_simm32
	cmp	[ta_value_type],4
	jae	ta_long_immediate_not_encodable
	jmp	ta_mov_mem_imm_32bit_store
      ta_mov_mem_imm_nosize:
	call	ta_recoverable_unknown_size
      ta_mov_mem_imm_8bit:
	call	ta_get_byte_value
	mov	byte [ta_value],al
	mov	[ta_postbyte_register],0
	mov	[ta_base_code],0C6h
	pop	ecx ebx edx
	call	ta_store_instruction_with_imm8
	jmp	ta_instruction_assembled
      ta_mov_mem_imm_16bit:
	call	ta_operand_16bit
	call	ta_get_word_value
	mov	word [ta_value],ax
	mov	[ta_postbyte_register],0
	mov	[ta_base_code],0C7h
	pop	ecx ebx edx
	call	ta_store_instruction_with_imm16
	jmp	ta_instruction_assembled
      ta_mov_mem_imm_32bit:
	call	ta_operand_32bit
	call	ta_get_dword_value
      ta_mov_mem_imm_32bit_store:
	mov	dword [ta_value],eax
	mov	[ta_postbyte_register],0
	mov	[ta_base_code],0C7h
	pop	ecx ebx edx
	call	ta_store_instruction_with_imm32
	jmp	ta_instruction_assembled
      ta_mov_reg:
	lods	byte [esi]
	mov	ah,al
	sub	ah,10h
	and	ah,al
	test	ah,0F0h
	jnz	ta_mov_sreg
	call	ta_convert_register
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	je	ta_mov_reg_mem
	cmp	al,'('
	je	ta_mov_reg_imm
	cmp	al,14h
	je	ta_mov_reg_creg
	cmp	al,10h
	jne	ta_invalid_operand
      ta_mov_reg_reg:
	lods	byte [esi]
	mov	ah,al
	sub	ah,10h
	and	ah,al
	test	ah,0F0h
	jnz	ta_mov_reg_sreg
	call	ta_convert_register
	mov	bl,[ta_postbyte_register]
	mov	[ta_postbyte_register],al
	mov	al,ah
	cmp	al,1
	je	ta_mov_reg_reg_8bit
	call	ta_operand_autodetect
	inc	[ta_base_code]
      ta_mov_reg_reg_8bit:
	jmp	ta_nomem_instruction_ready
      ta_mov_reg_sreg:
	mov	bl,[ta_postbyte_register]
	mov	ah,al
	and	al,1111b
	mov	[ta_postbyte_register],al
	shr	ah,4
	cmp	ah,3
	jne	ta_invalid_operand
	dec	[ta_postbyte_register]
	cmp	[ta_operand_size],8
	je	ta_mov_reg_sreg64
	cmp	[ta_operand_size],4
	je	ta_mov_reg_sreg32
	cmp	[ta_operand_size],2
	jne	ta_invalid_operand_size
	call	ta_operand_16bit
	jmp	ta_mov_reg_sreg_store
      ta_mov_reg_sreg64:
	call	ta_operand_64bit
	jmp	ta_mov_reg_sreg_store
      ta_mov_reg_sreg32:
	call	ta_operand_32bit
      ta_mov_reg_sreg_store:
	mov	[ta_base_code],8Ch
	jmp	ta_nomem_instruction_ready
      ta_mov_reg_creg:
	lods	byte [esi]
	mov	bl,al
	shr	al,4
	cmp	al,4
	ja	ta_invalid_operand
	add	al,20h
	mov	[ta_extended_code],al
	and	bl,1111b
	xchg	bl,[ta_postbyte_register]
	mov	[ta_base_code],0Fh
	cmp	[ta_code_type],64
	je	ta_mov_reg_creg_64bit
	cmp	[ta_operand_size],4
	jne	ta_invalid_operand_size
	cmp	[ta_postbyte_register],8
	jne	ta_mov_reg_creg_store
	cmp	[ta_extended_code],20h
	jne	ta_mov_reg_creg_store
	mov	al,0F0h
	stos	byte [edi]
	mov	[ta_postbyte_register],0
      ta_mov_reg_creg_store:
	jmp	ta_nomem_instruction_ready
      ta_mov_reg_creg_64bit:
	cmp	[ta_operand_size],8
	jne	ta_invalid_operand_size
	jmp	ta_nomem_instruction_ready
      ta_mov_reg_mem:
	add	[ta_base_code],2
	call	ta_get_address
	mov	al,[ta_operand_size]
	cmp	al,1
	je	ta_mov_reg_mem_8bit
	inc	[ta_base_code]
	call	ta_operand_autodetect
	mov	al,[ta_postbyte_register]
	or	al,bl
	or	al,bh
	jz	ta_mov_ax_mem
	jmp	ta_instruction_ready
      ta_mov_reg_mem_8bit:
	mov	al,[ta_postbyte_register]
	or	al,bl
	or	al,bh
	jz	ta_mov_al_mem
	jmp	ta_instruction_ready
      ta_mov_al_mem:
	test	ch,22h
	jnz	ta_mov_al_mem_address16
	test	ch,44h
	jnz	ta_mov_al_mem_address32
	test	ch,not 88h
	jnz	ta_invalid_address_size
	call	ta_check_mov_address64
	cmp	al,0
	jg	ta_mov_al_mem_address64
	jl	ta_instruction_ready
	cmp	[ta_code_type],16
	jne	ta_mov_al_mem_address32
	cmp	edx,10000h
	jb	ta_mov_al_mem_address16
      ta_mov_al_mem_address32:
	call	ta_store_segment_prefix_if_necessary
	call	ta_address_32bit_prefix
	mov	[ta_base_code],0A0h
	jmp	ta_store_mov_address32
      ta_mov_al_mem_address16:
	call	ta_store_segment_prefix_if_necessary
	call	ta_address_16bit_prefix
	mov	[ta_base_code],0A0h
	jmp	ta_store_mov_address16
      ta_mov_al_mem_address64:
	call	ta_store_segment_prefix_if_necessary
	mov	[ta_base_code],0A0h
	jmp	ta_store_mov_address64
      ta_mov_ax_mem:
	test	ch,22h
	jnz	ta_mov_ax_mem_address16
	test	ch,44h
	jnz	ta_mov_ax_mem_address32
	test	ch,not 88h
	jnz	ta_invalid_address_size
	call	ta_check_mov_address64
	cmp	al,0
	jg	ta_mov_ax_mem_address64
	jl	ta_instruction_ready
	cmp	[ta_code_type],16
	jne	ta_mov_ax_mem_address32
	cmp	edx,10000h
	jb	ta_mov_ax_mem_address16
      ta_mov_ax_mem_address32:
	call	ta_store_segment_prefix_if_necessary
	call	ta_address_32bit_prefix
	mov	[ta_base_code],0A1h
	jmp	ta_store_mov_address32
      ta_mov_ax_mem_address16:
	call	ta_store_segment_prefix_if_necessary
	call	ta_address_16bit_prefix
	mov	[ta_base_code],0A1h
	jmp	ta_store_mov_address16
      ta_mov_ax_mem_address64:
	call	ta_store_segment_prefix_if_necessary
	mov	[ta_base_code],0A1h
	jmp	ta_store_mov_address64
      ta_mov_reg_imm:
	mov	al,[ta_operand_size]
	cmp	al,1
	je	ta_mov_reg_imm_8bit
	cmp	al,2
	je	ta_mov_reg_imm_16bit
	cmp	al,4
	je	ta_mov_reg_imm_32bit
	cmp	al,8
	jne	ta_invalid_operand_size
      ta_mov_reg_imm_64bit:
	call	ta_operand_64bit
	call	ta_get_qword_value
	mov	ecx,edx
	cmp	[ta_size_declared],0
	jne	ta_mov_reg_imm_64bit_store
	cmp	[ta_value_type],4
	jae	ta_mov_reg_imm_64bit_store
	cdq
	cmp	ecx,edx
	je	ta_mov_reg_64bit_imm_32bit
      ta_mov_reg_imm_64bit_store:
	push	eax ecx
	mov	al,0B8h
	call	ta_store_mov_reg_imm_code
	pop	edx eax
	call	ta_mark_relocation
	stos	dword [edi]
	mov	eax,edx
	stos	dword [edi]
	jmp	ta_instruction_assembled
      ta_mov_reg_imm_8bit:
	call	ta_get_byte_value
	mov	dl,al
	mov	al,0B0h
	call	ta_store_mov_reg_imm_code
	mov	al,dl
	stos	byte [edi]
	jmp	ta_instruction_assembled
      ta_mov_reg_imm_16bit:
	call	ta_get_word_value
	mov	dx,ax
	call	ta_operand_16bit
	mov	al,0B8h
	call	ta_store_mov_reg_imm_code
	mov	ax,dx
	call	ta_mark_relocation
	stos	word [edi]
	jmp	ta_instruction_assembled
      ta_mov_reg_imm_32bit:
	call	ta_operand_32bit
	call	ta_get_dword_value
	mov	edx,eax
	mov	al,0B8h
	call	ta_store_mov_reg_imm_code
      ta_mov_store_imm_32bit:
	mov	eax,edx
	call	ta_mark_relocation
	stos	dword [edi]
	jmp	ta_instruction_assembled
      ta_store_mov_reg_imm_code:
	mov	ah,[ta_postbyte_register]
	test	ah,1000b
	jz	ta_mov_reg_imm_prefix_ok
	or	[ta_rex_prefix],41h
      ta_mov_reg_imm_prefix_ok:
	and	ah,111b
	add	al,ah
	mov	[ta_base_code],al
	call	ta_store_classic_instruction_code
	ret
      ta_mov_reg_64bit_imm_32bit:
	mov	edx,eax
	mov	bl,[ta_postbyte_register]
	mov	[ta_postbyte_register],0
	mov	[ta_base_code],0C7h
	call	ta_store_nomem_instruction
	jmp	ta_mov_store_imm_32bit
      ta_mov_sreg:
	mov	ah,al
	and	al,1111b
	mov	[ta_postbyte_register],al
	shr	ah,4
	cmp	ah,3
	jne	ta_invalid_operand
	cmp	al,2
	je	ta_illegal_instruction
	dec	[ta_postbyte_register]
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	je	ta_mov_sreg_mem
	cmp	al,10h
	jne	ta_invalid_operand
      ta_mov_sreg_reg:
	lods	byte [esi]
	call	ta_convert_register
	or	ah,ah
	jz	ta_mov_sreg_reg_size_ok
	cmp	ah,2
	jne	ta_invalid_operand_size
	mov	bl,al
      ta_mov_sreg_reg_size_ok:
	mov	[ta_base_code],8Eh
	jmp	ta_nomem_instruction_ready
      ta_mov_sreg_mem:
	call	ta_get_address
	mov	al,[ta_operand_size]
	or	al,al
	jz	ta_mov_sreg_mem_size_ok
	cmp	al,2
	jne	ta_invalid_operand_size
      ta_mov_sreg_mem_size_ok:
	mov	[ta_base_code],8Eh
	jmp	ta_instruction_ready
      ta_mov_creg:
	lods	byte [esi]
	mov	ah,al
	shr	ah,4
	cmp	ah,4
	ja	ta_invalid_operand
	add	ah,22h
	mov	[ta_extended_code],ah
	and	al,1111b
	mov	[ta_postbyte_register],al
	mov	[ta_base_code],0Fh
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_register
	mov	bl,al
	cmp	[ta_code_type],64
	je	ta_mov_creg_64bit
	cmp	ah,4
	jne	ta_invalid_operand_size
	cmp	[ta_postbyte_register],8
	jne	ta_mov_creg_store
	cmp	[ta_extended_code],22h
	jne	ta_mov_creg_store
	mov	al,0F0h
	stos	byte [edi]
	mov	[ta_postbyte_register],0
      ta_mov_creg_store:
	jmp	ta_nomem_instruction_ready
      ta_mov_creg_64bit:
	cmp	ah,8
	je	ta_mov_creg_store
	jmp	ta_invalid_operand_size
ta_test_instruction:
	mov	[ta_base_code],84h
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_test_reg
	cmp	al,'['
	jne	ta_invalid_operand
      ta_test_mem:
	call	ta_get_address
	push	edx ebx ecx
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'('
	je	ta_test_mem_imm
	cmp	al,10h
	jne	ta_invalid_operand
      ta_test_mem_reg:
	lods	byte [esi]
	call	ta_convert_register
	mov	[ta_postbyte_register],al
	pop	ecx ebx edx
	mov	al,ah
	cmp	al,1
	je	ta_test_mem_reg_8bit
	call	ta_operand_autodetect
	inc	[ta_base_code]
      ta_test_mem_reg_8bit:
	jmp	ta_instruction_ready
      ta_test_mem_imm:
	mov	al,[ta_operand_size]
	cmp	al,1
	jb	ta_test_mem_imm_nosize
	je	ta_test_mem_imm_8bit
	cmp	al,2
	je	ta_test_mem_imm_16bit
	cmp	al,4
	je	ta_test_mem_imm_32bit
	cmp	al,8
	jne	ta_invalid_operand_size
      ta_test_mem_imm_64bit:
	cmp	[ta_size_declared],0
	jne	ta_long_immediate_not_encodable
	call	ta_operand_64bit
	call	ta_get_simm32
	cmp	[ta_value_type],4
	jae	ta_long_immediate_not_encodable
	jmp	ta_test_mem_imm_32bit_store
      ta_test_mem_imm_nosize:
	call	ta_recoverable_unknown_size
      ta_test_mem_imm_8bit:
	call	ta_get_byte_value
	mov	byte [ta_value],al
	mov	[ta_postbyte_register],0
	mov	[ta_base_code],0F6h
	pop	ecx ebx edx
	call	ta_store_instruction_with_imm8
	jmp	ta_instruction_assembled
      ta_test_mem_imm_16bit:
	call	ta_operand_16bit
	call	ta_get_word_value
	mov	word [ta_value],ax
	mov	[ta_postbyte_register],0
	mov	[ta_base_code],0F7h
	pop	ecx ebx edx
	call	ta_store_instruction_with_imm16
	jmp	ta_instruction_assembled
      ta_test_mem_imm_32bit:
	call	ta_operand_32bit
	call	ta_get_dword_value
      ta_test_mem_imm_32bit_store:
	mov	dword [ta_value],eax
	mov	[ta_postbyte_register],0
	mov	[ta_base_code],0F7h
	pop	ecx ebx edx
	call	ta_store_instruction_with_imm32
	jmp	ta_instruction_assembled
      ta_test_reg:
	lods	byte [esi]
	call	ta_convert_register
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	je	ta_test_reg_mem
	cmp	al,'('
	je	ta_test_reg_imm
	cmp	al,10h
	jne	ta_invalid_operand
      ta_test_reg_reg:
	lods	byte [esi]
	call	ta_convert_register
	mov	bl,[ta_postbyte_register]
	mov	[ta_postbyte_register],al
	mov	al,ah
	cmp	al,1
	je	ta_test_reg_reg_8bit
	call	ta_operand_autodetect
	inc	[ta_base_code]
      ta_test_reg_reg_8bit:
	jmp	ta_nomem_instruction_ready
      ta_test_reg_imm:
	mov	al,[ta_operand_size]
	cmp	al,1
	je	ta_test_reg_imm_8bit
	cmp	al,2
	je	ta_test_reg_imm_16bit
	cmp	al,4
	je	ta_test_reg_imm_32bit
	cmp	al,8
	jne	ta_invalid_operand_size
      ta_test_reg_imm_64bit:
	cmp	[ta_size_declared],0
	jne	ta_long_immediate_not_encodable
	call	ta_operand_64bit
	call	ta_get_simm32
	cmp	[ta_value_type],4
	jae	ta_long_immediate_not_encodable
	jmp	ta_test_reg_imm_32bit_store
      ta_test_reg_imm_8bit:
	call	ta_get_byte_value
	mov	dl,al
	mov	bl,[ta_postbyte_register]
	mov	[ta_postbyte_register],0
	mov	[ta_base_code],0F6h
	or	bl,bl
	jz	ta_test_al_imm
	call	ta_store_nomem_instruction
	mov	al,dl
	stos	byte [edi]
	jmp	ta_instruction_assembled
      ta_test_al_imm:
	mov	[ta_base_code],0A8h
	call	ta_store_classic_instruction_code
	mov	al,dl
	stos	byte [edi]
	jmp	ta_instruction_assembled
      ta_test_reg_imm_16bit:
	call	ta_operand_16bit
	call	ta_get_word_value
	mov	dx,ax
	mov	bl,[ta_postbyte_register]
	mov	[ta_postbyte_register],0
	mov	[ta_base_code],0F7h
	or	bl,bl
	jz	ta_test_ax_imm
	call	ta_store_nomem_instruction
	mov	ax,dx
	call	ta_mark_relocation
	stos	word [edi]
	jmp	ta_instruction_assembled
      ta_test_ax_imm:
	mov	[ta_base_code],0A9h
	call	ta_store_classic_instruction_code
	mov	ax,dx
	stos	word [edi]
	jmp	ta_instruction_assembled
      ta_test_reg_imm_32bit:
	call	ta_operand_32bit
	call	ta_get_dword_value
      ta_test_reg_imm_32bit_store:
	mov	edx,eax
	mov	bl,[ta_postbyte_register]
	mov	[ta_postbyte_register],0
	mov	[ta_base_code],0F7h
	or	bl,bl
	jz	ta_test_eax_imm
	call	ta_store_nomem_instruction
	mov	eax,edx
	call	ta_mark_relocation
	stos	dword [edi]
	jmp	ta_instruction_assembled
      ta_test_eax_imm:
	mov	[ta_base_code],0A9h
	call	ta_store_classic_instruction_code
	mov	eax,edx
	stos	dword [edi]
	jmp	ta_instruction_assembled
      ta_test_reg_mem:
	call	ta_get_address
	mov	al,[ta_operand_size]
	cmp	al,1
	je	ta_test_reg_mem_8bit
	call	ta_operand_autodetect
	inc	[ta_base_code]
      ta_test_reg_mem_8bit:
	jmp	ta_instruction_ready
ta_xchg_instruction:
	mov	[ta_base_code],86h
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_xchg_reg
	cmp	al,'['
	jne	ta_invalid_operand
      ta_xchg_mem:
	call	ta_get_address
	push	edx ebx ecx
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_test_mem_reg
	jmp	ta_invalid_operand
      ta_xchg_reg:
	lods	byte [esi]
	call	ta_convert_register
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	je	ta_test_reg_mem
	cmp	al,10h
	jne	ta_invalid_operand
      ta_xchg_reg_reg:
	lods	byte [esi]
	call	ta_convert_register
	mov	bl,al
	mov	al,ah
	cmp	al,1
	je	ta_xchg_reg_reg_8bit
	call	ta_operand_autodetect
	cmp	[ta_postbyte_register],0
	je	ta_xchg_ax_reg
	or	bl,bl
	jnz	ta_xchg_reg_reg_store
	mov	bl,[ta_postbyte_register]
      ta_xchg_ax_reg:
	cmp	[ta_code_type],64
	jne	ta_xchg_ax_reg_ok
	cmp	ah,4
	jne	ta_xchg_ax_reg_ok
	or	bl,bl
	jz	ta_xchg_reg_reg_store
      ta_xchg_ax_reg_ok:
	test	bl,1000b
	jz	ta_xchg_ax_reg_store
	or	[ta_rex_prefix],41h
	and	bl,111b
      ta_xchg_ax_reg_store:
	add	bl,90h
	mov	[ta_base_code],bl
	call	ta_store_classic_instruction_code
	jmp	ta_instruction_assembled
      ta_xchg_reg_reg_store:
	inc	[ta_base_code]
      ta_xchg_reg_reg_8bit:
	jmp	ta_nomem_instruction_ready
ta_push_instruction:
	mov	[ta_push_size],al
      ta_push_next:
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_push_reg
	cmp	al,'('
	je	ta_push_imm
	cmp	al,'['
	jne	ta_invalid_operand
      ta_push_mem:
	call	ta_get_address
	mov	al,[ta_operand_size]
	mov	ah,[ta_push_size]
	cmp	al,2
	je	ta_push_mem_16bit
	cmp	al,4
	je	ta_push_mem_32bit
	cmp	al,8
	je	ta_push_mem_64bit
	or	al,al
	jnz	ta_invalid_operand_size
	cmp	ah,2
	je	ta_push_mem_16bit
	cmp	ah,4
	je	ta_push_mem_32bit
	cmp	ah,8
	je	ta_push_mem_64bit
	call	ta_recoverable_unknown_size
	jmp	ta_push_mem_store
      ta_push_mem_16bit:
	test	ah,not 2
	jnz	ta_invalid_operand_size
	call	ta_operand_16bit
	jmp	ta_push_mem_store
      ta_push_mem_32bit:
	test	ah,not 4
	jnz	ta_invalid_operand_size
	cmp	[ta_code_type],64
	je	ta_illegal_instruction
	call	ta_operand_32bit
	jmp	ta_push_mem_store
      ta_push_mem_64bit:
	test	ah,not 8
	jnz	ta_invalid_operand_size
	cmp	[ta_code_type],64
	jne	ta_illegal_instruction
      ta_push_mem_store:
	mov	[ta_base_code],0FFh
	mov	[ta_postbyte_register],110b
	call	ta_store_instruction
	jmp	ta_push_done
      ta_push_reg:
	lods	byte [esi]
	mov	ah,al
	sub	ah,10h
	and	ah,al
	test	ah,0F0h
	jnz	ta_push_sreg
	call	ta_convert_register
	test	al,1000b
	jz	ta_push_reg_ok
	or	[ta_rex_prefix],41h
	and	al,111b
      ta_push_reg_ok:
	add	al,50h
	mov	[ta_base_code],al
	mov	al,ah
	mov	ah,[ta_push_size]
	cmp	al,2
	je	ta_push_reg_16bit
	cmp	al,4
	je	ta_push_reg_32bit
	cmp	al,8
	jne	ta_invalid_operand_size
      ta_push_reg_64bit:
	test	ah,not 8
	jnz	ta_invalid_operand_size
	cmp	[ta_code_type],64
	jne	ta_illegal_instruction
	jmp	ta_push_reg_store
      ta_push_reg_32bit:
	test	ah,not 4
	jnz	ta_invalid_operand_size
	cmp	[ta_code_type],64
	je	ta_illegal_instruction
	call	ta_operand_32bit
	jmp	ta_push_reg_store
      ta_push_reg_16bit:
	test	ah,not 2
	jnz	ta_invalid_operand_size
	call	ta_operand_16bit
      ta_push_reg_store:
	call	ta_store_classic_instruction_code
	jmp	ta_push_done
      ta_push_sreg:
	mov	bl,al
	mov	dl,[ta_operand_size]
	mov	dh,[ta_push_size]
	cmp	dl,2
	je	ta_push_sreg16
	cmp	dl,4
	je	ta_push_sreg32
	cmp	dl,8
	je	ta_push_sreg64
	or	dl,dl
	jnz	ta_invalid_operand_size
	cmp	dh,2
	je	ta_push_sreg16
	cmp	dh,4
	je	ta_push_sreg32
	cmp	dh,8
	je	ta_push_sreg64
	jmp	ta_push_sreg_store
      ta_push_sreg16:
	test	dh,not 2
	jnz	ta_invalid_operand_size
	call	ta_operand_16bit
	jmp	ta_push_sreg_store
      ta_push_sreg32:
	test	dh,not 4
	jnz	ta_invalid_operand_size
	cmp	[ta_code_type],64
	je	ta_illegal_instruction
	call	ta_operand_32bit
	jmp	ta_push_sreg_store
      ta_push_sreg64:
	test	dh,not 8
	jnz	ta_invalid_operand_size
	cmp	[ta_code_type],64
	jne	ta_illegal_instruction
      ta_push_sreg_store:
	mov	al,bl
	cmp	al,40h
	jae	ta_invalid_operand
	sub	al,31h
	jc	ta_invalid_operand
	cmp	al,4
	jae	ta_push_sreg_386
	shl	al,3
	add	al,6
	mov	[ta_base_code],al
	cmp	[ta_code_type],64
	je	ta_illegal_instruction
	jmp	ta_push_reg_store
      ta_push_sreg_386:
	sub	al,4
	shl	al,3
	add	al,0A0h
	mov	[ta_extended_code],al
	mov	[ta_base_code],0Fh
	jmp	ta_push_reg_store
      ta_push_imm:
	mov	al,[ta_operand_size]
	mov	ah,[ta_push_size]
	or	al,al
	je	ta_push_imm_size_ok
	or	ah,ah
	je	ta_push_imm_size_ok
	cmp	al,ah
	jne	ta_invalid_operand_size
      ta_push_imm_size_ok:
	cmp	al,2
	je	ta_push_imm_16bit
	cmp	al,4
	je	ta_push_imm_32bit
	cmp	al,8
	je	ta_push_imm_64bit
	cmp	ah,2
	je	ta_push_imm_optimized_16bit
	cmp	ah,4
	je	ta_push_imm_optimized_32bit
	cmp	ah,8
	je	ta_push_imm_optimized_64bit
	or	al,al
	jnz	ta_invalid_operand_size
	cmp	[ta_code_type],16
	je	ta_push_imm_optimized_16bit
	cmp	[ta_code_type],32
	je	ta_push_imm_optimized_32bit
      ta_push_imm_optimized_64bit:
	cmp	[ta_code_type],64
	jne	ta_illegal_instruction
	call	ta_get_simm32
	mov	edx,eax
	cmp	[ta_value_type],0
	jne	ta_push_imm_32bit_store
	cmp	eax,-80h
	jl	ta_push_imm_32bit_store
	cmp	eax,80h
	jge	ta_push_imm_32bit_store
	jmp	ta_push_imm_8bit
      ta_push_imm_optimized_32bit:
	cmp	[ta_code_type],64
	je	ta_illegal_instruction
	call	ta_get_dword_value
	mov	edx,eax
	call	ta_operand_32bit
	cmp	[ta_value_type],0
	jne	ta_push_imm_32bit_store
	cmp	eax,-80h
	jl	ta_push_imm_32bit_store
	cmp	eax,80h
	jge	ta_push_imm_32bit_store
	jmp	ta_push_imm_8bit
      ta_push_imm_optimized_16bit:
	call	ta_get_word_value
	mov	dx,ax
	call	ta_operand_16bit
	cmp	[ta_value_type],0
	jne	ta_push_imm_16bit_store
	cmp	ax,-80h
	jl	ta_push_imm_16bit_store
	cmp	ax,80h
	jge	ta_push_imm_16bit_store
      ta_push_imm_8bit:
	mov	ah,al
	mov	[ta_base_code],6Ah
	call	ta_store_classic_instruction_code
	mov	al,ah
	stos	byte [edi]
	jmp	ta_push_done
      ta_push_imm_16bit:
	call	ta_get_word_value
	mov	dx,ax
	call	ta_operand_16bit
      ta_push_imm_16bit_store:
	mov	[ta_base_code],68h
	call	ta_store_classic_instruction_code
	mov	ax,dx
	call	ta_mark_relocation
	stos	word [edi]
	jmp	ta_push_done
      ta_push_imm_64bit:
	cmp	[ta_code_type],64
	jne	ta_illegal_instruction
	call	ta_get_simm32
	mov	edx,eax
	jmp	ta_push_imm_32bit_store
      ta_push_imm_32bit:
	cmp	[ta_code_type],64
	je	ta_illegal_instruction
	call	ta_get_dword_value
	mov	edx,eax
	call	ta_operand_32bit
      ta_push_imm_32bit_store:
	mov	[ta_base_code],68h
	call	ta_store_classic_instruction_code
	mov	eax,edx
	call	ta_mark_relocation
	stos	dword [edi]
      ta_push_done:
	lods	byte [esi]
	dec	esi
	cmp	al,0Fh
	je	ta_instruction_assembled
	or	al,al
	jz	ta_instruction_assembled
;	 mov	 [ta_operand_size],0
;	 mov	 [ta_operand_flags],0
;	 mov	 [ta_operand_prefix],0
;	 mov	 [ta_rex_prefix],0
	and	dword [ta_operand_size],0
	jmp	ta_push_next
ta_pop_instruction:
	mov	[ta_push_size],al
      ta_pop_next:
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_pop_reg
	cmp	al,'['
	jne	ta_invalid_operand
      ta_pop_mem:
	call	ta_get_address
	mov	al,[ta_operand_size]
	mov	ah,[ta_push_size]
	cmp	al,2
	je	ta_pop_mem_16bit
	cmp	al,4
	je	ta_pop_mem_32bit
	cmp	al,8
	je	ta_pop_mem_64bit
	or	al,al
	jnz	ta_invalid_operand_size
	cmp	ah,2
	je	ta_pop_mem_16bit
	cmp	ah,4
	je	ta_pop_mem_32bit
	cmp	ah,8
	je	ta_pop_mem_64bit
	call	ta_recoverable_unknown_size
	jmp	ta_pop_mem_store
      ta_pop_mem_16bit:
	test	ah,not 2
	jnz	ta_invalid_operand_size
	call	ta_operand_16bit
	jmp	ta_pop_mem_store
      ta_pop_mem_32bit:
	test	ah,not 4
	jnz	ta_invalid_operand_size
	cmp	[ta_code_type],64
	je	ta_illegal_instruction
	call	ta_operand_32bit
	jmp	ta_pop_mem_store
      ta_pop_mem_64bit:
	test	ah,not 8
	jnz	ta_invalid_operand_size
	cmp	[ta_code_type],64
	jne	ta_illegal_instruction
      ta_pop_mem_store:
	mov	[ta_base_code],08Fh
	mov	[ta_postbyte_register],0
	call	ta_store_instruction
	jmp	ta_pop_done
      ta_pop_reg:
	lods	byte [esi]
	mov	ah,al
	sub	ah,10h
	and	ah,al
	test	ah,0F0h
	jnz	ta_pop_sreg
	call	ta_convert_register
	test	al,1000b
	jz	ta_pop_reg_ok
	or	[ta_rex_prefix],41h
	and	al,111b
      ta_pop_reg_ok:
	add	al,58h
	mov	[ta_base_code],al
	mov	al,ah
	mov	ah,[ta_push_size]
	cmp	al,2
	je	ta_pop_reg_16bit
	cmp	al,4
	je	ta_pop_reg_32bit
	cmp	al,8
	je	ta_pop_reg_64bit
	jmp	ta_invalid_operand_size
      ta_pop_reg_64bit:
	test	ah,not 8
	jnz	ta_invalid_operand_size
	cmp	[ta_code_type],64
	jne	ta_illegal_instruction
	jmp	ta_pop_reg_store
      ta_pop_reg_32bit:
	test	ah,not 4
	jnz	ta_invalid_operand_size
	cmp	[ta_code_type],64
	je	ta_illegal_instruction
	call	ta_operand_32bit
	jmp	ta_pop_reg_store
      ta_pop_reg_16bit:
	test	ah,not 2
	jnz	ta_invalid_operand_size
	call	ta_operand_16bit
      ta_pop_reg_store:
	call	ta_store_classic_instruction_code
      ta_pop_done:
	lods	byte [esi]
	dec	esi
	cmp	al,0Fh
	je	ta_instruction_assembled
	or	al,al
	jz	ta_instruction_assembled
;	 mov	 [ta_operand_size],0
;	 mov	 [ta_operand_flags],0
;	 mov	 [ta_operand_prefix],0
;	 mov	 [ta_rex_prefix],0
	and	dword [ta_operand_size],0
	jmp	ta_pop_next
      ta_pop_sreg:
	mov	dl,[ta_operand_size]
	mov	dh,[ta_push_size]
	cmp	al,32h
	je	ta_pop_cs
	mov	bl,al
	cmp	dl,2
	je	ta_pop_sreg16
	cmp	dl,4
	je	ta_pop_sreg32
	cmp	dl,8
	je	ta_pop_sreg64
	or	dl,dl
	jnz	ta_invalid_operand_size
	cmp	dh,2
	je	ta_pop_sreg16
	cmp	dh,4
	je	ta_pop_sreg32
	cmp	dh,8
	je	ta_pop_sreg64
	jmp	ta_pop_sreg_store
      ta_pop_sreg16:
	test	dh,not 2
	jnz	ta_invalid_operand_size
	call	ta_operand_16bit
	jmp	ta_pop_sreg_store
      ta_pop_sreg32:
	test	dh,not 4
	jnz	ta_invalid_operand_size
	cmp	[ta_code_type],64
	je	ta_illegal_instruction
	call	ta_operand_32bit
	jmp	ta_pop_sreg_store
      ta_pop_sreg64:
	test	dh,not 8
	jnz	ta_invalid_operand_size
	cmp	[ta_code_type],64
	jne	ta_illegal_instruction
      ta_pop_sreg_store:
	mov	al,bl
	cmp	al,40h
	jae	ta_invalid_operand
	sub	al,31h
	jc	ta_invalid_operand
	cmp	al,4
	jae	ta_pop_sreg_386
	shl	al,3
	add	al,7
	mov	[ta_base_code],al
	cmp	[ta_code_type],64
	je	ta_illegal_instruction
	jmp	ta_pop_reg_store
      ta_pop_cs:
	cmp	[ta_code_type],16
	jne	ta_illegal_instruction
	cmp	dl,2
	je	ta_pop_cs_store
	or	dl,dl
	jnz	ta_invalid_operand_size
	cmp	dh,2
	je	ta_pop_cs_store
	or	dh,dh
	jnz	ta_illegal_instruction
      ta_pop_cs_store:
	test	dh,not 2
	jnz	ta_invalid_operand_size
	mov	al,0Fh
	stos	byte [edi]
	jmp	ta_pop_done
      ta_pop_sreg_386:
	sub	al,4
	shl	al,3
	add	al,0A1h
	mov	[ta_extended_code],al
	mov	[ta_base_code],0Fh
	jmp	ta_pop_reg_store
ta_inc_instruction:
	mov	[ta_base_code],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_inc_reg
	cmp	al,'['
	je	ta_inc_mem
	jne	ta_invalid_operand
      ta_inc_mem:
	call	ta_get_address
	mov	al,[ta_operand_size]
	cmp	al,1
	je	ta_inc_mem_8bit
	jb	ta_inc_mem_nosize
	call	ta_operand_autodetect
	mov	al,0FFh
	xchg	al,[ta_base_code]
	mov	[ta_postbyte_register],al
	jmp	ta_instruction_ready
      ta_inc_mem_nosize:
	call	ta_recoverable_unknown_size
      ta_inc_mem_8bit:
	mov	al,0FEh
	xchg	al,[ta_base_code]
	mov	[ta_postbyte_register],al
	jmp	ta_instruction_ready
      ta_inc_reg:
	lods	byte [esi]
	call	ta_convert_register
	mov	bl,al
	mov	al,0FEh
	xchg	al,[ta_base_code]
	mov	[ta_postbyte_register],al
	mov	al,ah
	cmp	al,1
	je	ta_inc_reg_8bit
	call	ta_operand_autodetect
	cmp	[ta_code_type],64
	je	ta_inc_reg_long_form
	mov	al,[ta_postbyte_register]
	shl	al,3
	add	al,bl
	add	al,40h
	mov	[ta_base_code],al
	call	ta_store_classic_instruction_code
	jmp	ta_instruction_assembled
      ta_inc_reg_long_form:
	inc	[ta_base_code]
      ta_inc_reg_8bit:
	jmp	ta_nomem_instruction_ready
ta_set_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_set_reg
	cmp	al,'['
	jne	ta_invalid_operand
      ta_set_mem:
	call	ta_get_address
	cmp	[ta_operand_size],1
	ja	ta_invalid_operand_size
	mov	[ta_postbyte_register],0
	jmp	ta_instruction_ready
      ta_set_reg:
	lods	byte [esi]
	call	ta_convert_register
	cmp	ah,1
	jne	ta_invalid_operand_size
	mov	bl,al
	mov	[ta_postbyte_register],0
	jmp	ta_nomem_instruction_ready
ta_arpl_instruction:
	cmp	[ta_code_type],64
	je	ta_illegal_instruction
	mov	[ta_base_code],63h
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_arpl_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_register
	mov	[ta_postbyte_register],al
	cmp	ah,2
	jne	ta_invalid_operand_size
	jmp	ta_instruction_ready
      ta_arpl_reg:
	lods	byte [esi]
	call	ta_convert_register
	cmp	ah,2
	jne	ta_invalid_operand_size
	mov	bl,al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_register
	mov	[ta_postbyte_register],al
	jmp	ta_nomem_instruction_ready
ta_bound_instruction:
	cmp	[ta_code_type],64
	je	ta_illegal_instruction
	call	ta_take_register
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	mov	al,[ta_operand_size]
	cmp	al,2
	je	ta_bound_store
	cmp	al,4
	jne	ta_invalid_operand_size
      ta_bound_store:
	call	ta_operand_autodetect
	mov	[ta_base_code],62h
	jmp	ta_instruction_ready
ta_enter_instruction:
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	ah,2
	je	ta_enter_imm16_size_ok
	or	ah,ah
	jnz	ta_invalid_operand_size
      ta_enter_imm16_size_ok:
	cmp	al,'('
	jne	ta_invalid_operand
	call	ta_get_word_value
	cmp	[ta_next_pass_needed],0
	jne	ta_enter_imm16_ok
	cmp	[ta_value_type],0
	jne	ta_invalid_use_of_symbol
	test	eax,eax
	js	ta_value_out_of_range
      ta_enter_imm16_ok:
	push	eax
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	ah,1
	je	ta_enter_imm8_size_ok
	or	ah,ah
	jnz	ta_invalid_operand_size
      ta_enter_imm8_size_ok:
	cmp	al,'('
	jne	ta_invalid_operand
	call	ta_get_byte_value
	cmp	[ta_next_pass_needed],0
	jne	ta_enter_imm8_ok
	test	eax,eax
	js	ta_value_out_of_range
      ta_enter_imm8_ok:
	mov	dl,al
	pop	ebx
	mov	al,0C8h
	stos	byte [edi]
	mov	ax,bx
	stos	word [edi]
	mov	al,dl
	stos	byte [edi]
	jmp	ta_instruction_assembled
ta_ret_instruction_only64:
	cmp	[ta_code_type],64
	jne	ta_illegal_instruction
	jmp	ta_ret_instruction
ta_ret_instruction_32bit_except64:
	cmp	[ta_code_type],64
	je	ta_illegal_instruction
ta_ret_instruction_32bit:
	call	ta_operand_32bit
	jmp	ta_ret_instruction
ta_ret_instruction_16bit:
	call	ta_operand_16bit
	jmp	ta_ret_instruction
ta_ret_instruction_64bit:
	call	ta_operand_64bit
ta_ret_instruction:
	and	[ta_prefix_flags],not 10h
      ta_ret_common:
	mov	[ta_base_code],al
	lods	byte [esi]
	dec	esi
	or	al,al
	jz	ta_simple_ret
	cmp	al,0Fh
	je	ta_simple_ret
	lods	byte [esi]
	call	ta_get_size_operator
	or	ah,ah
	jz	ta_ret_imm
	cmp	ah,2
	je	ta_ret_imm
	jmp	ta_invalid_operand_size
      ta_ret_imm:
	cmp	al,'('
	jne	ta_invalid_operand
	call	ta_get_word_value
	cmp	[ta_next_pass_needed],0
	jne	ta_ret_imm_ok
	cmp	[ta_value_type],0
	jne	ta_invalid_use_of_symbol
	test	eax,eax
	js	ta_value_out_of_range
      ta_ret_imm_ok:
	cmp	[ta_size_declared],0
	jne	ta_ret_imm_store
	or	ax,ax
	jz	ta_simple_ret
      ta_ret_imm_store:
	mov	dx,ax
	call	ta_store_classic_instruction_code
	mov	ax,dx
	stos	word [edi]
	jmp	ta_instruction_assembled
      ta_simple_ret:
	inc	[ta_base_code]
	call	ta_store_classic_instruction_code
	jmp	ta_instruction_assembled
ta_retf_instruction:
	cmp	[ta_code_type],64
	jne	ta_ret_common
ta_retf_instruction_64bit:
	call	ta_operand_64bit
	jmp	ta_ret_common
ta_retf_instruction_32bit:
	call	ta_operand_32bit
	jmp	ta_ret_common
ta_retf_instruction_16bit:
	call	ta_operand_16bit
	jmp	ta_ret_common
ta_lea_instruction:
	mov	[ta_base_code],8Dh
	call	ta_take_register
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	xor	al,al
	xchg	al,[ta_operand_size]
	push	eax
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	or	[ta_operand_flags],1
	call	ta_get_address
	pop	eax
	mov	[ta_operand_size],al
	call	ta_operand_autodetect
	jmp	ta_instruction_ready
ta_ls_instruction:
	or	al,al
	jz	ta_les_instruction
	cmp	al,3
	jz	ta_lds_instruction
	add	al,0B0h
	mov	[ta_extended_code],al
	mov	[ta_base_code],0Fh
	jmp	ta_ls_code_ok
      ta_les_instruction:
	mov	[ta_base_code],0C4h
	jmp	ta_ls_short_code
      ta_lds_instruction:
	mov	[ta_base_code],0C5h
      ta_ls_short_code:
	cmp	[ta_code_type],64
	je	ta_illegal_instruction
      ta_ls_code_ok:
	call	ta_take_register
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	add	[ta_operand_size],2
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	mov	al,[ta_operand_size]
	cmp	al,4
	je	ta_ls_16bit
	cmp	al,6
	je	ta_ls_32bit
	cmp	al,10
	je	ta_ls_64bit
	jmp	ta_invalid_operand_size
      ta_ls_16bit:
	call	ta_operand_16bit
	jmp	ta_instruction_ready
      ta_ls_32bit:
	call	ta_operand_32bit
	jmp	ta_instruction_ready
      ta_ls_64bit:
	call	ta_operand_64bit
	jmp	ta_instruction_ready
ta_sh_instruction:
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_sh_reg
	cmp	al,'['
	jne	ta_invalid_operand
      ta_sh_mem:
	call	ta_get_address
	push	edx ebx ecx
	mov	al,[ta_operand_size]
	push	eax
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'('
	je	ta_sh_mem_imm
	cmp	al,10h
	jne	ta_invalid_operand
      ta_sh_mem_reg:
	lods	byte [esi]
	cmp	al,11h
	jne	ta_invalid_operand
	pop	eax ecx ebx edx
	cmp	al,1
	je	ta_sh_mem_cl_8bit
	jb	ta_sh_mem_cl_nosize
	call	ta_operand_autodetect
	mov	[ta_base_code],0D3h
	jmp	ta_instruction_ready
      ta_sh_mem_cl_nosize:
	call	ta_recoverable_unknown_size
      ta_sh_mem_cl_8bit:
	mov	[ta_base_code],0D2h
	jmp	ta_instruction_ready
      ta_sh_mem_imm:
	mov	al,[ta_operand_size]
	or	al,al
	jz	ta_sh_mem_imm_size_ok
	cmp	al,1
	jne	ta_invalid_operand_size
      ta_sh_mem_imm_size_ok:
	call	ta_get_byte_value
	mov	byte [ta_value],al
	pop	eax ecx ebx edx
	cmp	al,1
	je	ta_sh_mem_imm_8bit
	jb	ta_sh_mem_imm_nosize
	call	ta_operand_autodetect
	cmp	byte [ta_value],1
	je	ta_sh_mem_1
	mov	[ta_base_code],0C1h
	call	ta_store_instruction_with_imm8
	jmp	ta_instruction_assembled
      ta_sh_mem_1:
	mov	[ta_base_code],0D1h
	jmp	ta_instruction_ready
      ta_sh_mem_imm_nosize:
	call	ta_recoverable_unknown_size
      ta_sh_mem_imm_8bit:
	cmp	byte [ta_value],1
	je	ta_sh_mem_1_8bit
	mov	[ta_base_code],0C0h
	call	ta_store_instruction_with_imm8
	jmp	ta_instruction_assembled
      ta_sh_mem_1_8bit:
	mov	[ta_base_code],0D0h
	jmp	ta_instruction_ready
      ta_sh_reg:
	lods	byte [esi]
	call	ta_convert_register
	mov	bx,ax
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'('
	je	ta_sh_reg_imm
	cmp	al,10h
	jne	ta_invalid_operand
      ta_sh_reg_reg:
	lods	byte [esi]
	cmp	al,11h
	jne	ta_invalid_operand
	mov	al,bh
	cmp	al,1
	je	ta_sh_reg_cl_8bit
	call	ta_operand_autodetect
	mov	[ta_base_code],0D3h
	jmp	ta_nomem_instruction_ready
      ta_sh_reg_cl_8bit:
	mov	[ta_base_code],0D2h
	jmp	ta_nomem_instruction_ready
      ta_sh_reg_imm:
	mov	al,[ta_operand_size]
	or	al,al
	jz	ta_sh_reg_imm_size_ok
	cmp	al,1
	jne	ta_invalid_operand_size
      ta_sh_reg_imm_size_ok:
	push	ebx
	call	ta_get_byte_value
	mov	dl,al
	pop	ebx
	mov	al,bh
	cmp	al,1
	je	ta_sh_reg_imm_8bit
	call	ta_operand_autodetect
	cmp	dl,1
	je	ta_sh_reg_1
	mov	[ta_base_code],0C1h
	call	ta_store_nomem_instruction
	mov	al,dl
	stos	byte [edi]
	jmp	ta_instruction_assembled
      ta_sh_reg_1:
	mov	[ta_base_code],0D1h
	jmp	ta_nomem_instruction_ready
      ta_sh_reg_imm_8bit:
	cmp	dl,1
	je	ta_sh_reg_1_8bit
	mov	[ta_base_code],0C0h
	call	ta_store_nomem_instruction
	mov	al,dl
	stos	byte [edi]
	jmp	ta_instruction_assembled
      ta_sh_reg_1_8bit:
	mov	[ta_base_code],0D0h
	jmp	ta_nomem_instruction_ready
ta_shd_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_shd_reg
	cmp	al,'['
	jne	ta_invalid_operand
      ta_shd_mem:
	call	ta_get_address
	push	edx ebx ecx
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_register
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	mov	al,ah
	mov	[ta_operand_size],0
	push	eax
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'('
	je	ta_shd_mem_reg_imm
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	cmp	al,11h
	jne	ta_invalid_operand
	pop	eax ecx ebx edx
	call	ta_operand_autodetect
	inc	[ta_extended_code]
	jmp	ta_instruction_ready
      ta_shd_mem_reg_imm:
	mov	al,[ta_operand_size]
	or	al,al
	jz	ta_shd_mem_reg_imm_size_ok
	cmp	al,1
	jne	ta_invalid_operand_size
      ta_shd_mem_reg_imm_size_ok:
	call	ta_get_byte_value
	mov	byte [ta_value],al
	pop	eax ecx ebx edx
	call	ta_operand_autodetect
	call	ta_store_instruction_with_imm8
	jmp	ta_instruction_assembled
      ta_shd_reg:
	lods	byte [esi]
	call	ta_convert_register
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_register
	mov	bl,[ta_postbyte_register]
	mov	[ta_postbyte_register],al
	mov	al,ah
	push	eax ebx
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	mov	[ta_operand_size],0
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'('
	je	ta_shd_reg_reg_imm
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	cmp	al,11h
	jne	ta_invalid_operand
	pop	ebx eax
	call	ta_operand_autodetect
	inc	[ta_extended_code]
	jmp	ta_nomem_instruction_ready
      ta_shd_reg_reg_imm:
	mov	al,[ta_operand_size]
	or	al,al
	jz	ta_shd_reg_reg_imm_size_ok
	cmp	al,1
	jne	ta_invalid_operand_size
      ta_shd_reg_reg_imm_size_ok:
	call	ta_get_byte_value
	mov	dl,al
	pop	ebx eax
	call	ta_operand_autodetect
	call	ta_store_nomem_instruction
	mov	al,dl
	stos	byte [edi]
	jmp	ta_instruction_assembled
ta_movx_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	call	ta_take_register
	mov	[ta_postbyte_register],al
	mov	al,ah
	push	eax
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	mov	[ta_operand_size],0
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_movx_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	pop	eax
	mov	ah,[ta_operand_size]
	or	ah,ah
	jz	ta_movx_unknown_size
	cmp	ah,al
	jae	ta_invalid_operand_size
	cmp	ah,1
	je	ta_movx_mem_store
	cmp	ah,2
	jne	ta_invalid_operand_size
	inc	[ta_extended_code]
      ta_movx_mem_store:
	call	ta_operand_autodetect
	jmp	ta_instruction_ready
      ta_movx_unknown_size:
	cmp	al,2
	je	ta_movx_mem_store
	call	ta_recoverable_unknown_size
	jmp	ta_movx_mem_store
      ta_movx_reg:
	lods	byte [esi]
	call	ta_convert_register
	pop	ebx
	xchg	bl,al
	cmp	ah,al
	jae	ta_invalid_operand_size
	cmp	ah,1
	je	ta_movx_reg_8bit
	cmp	ah,2
	je	ta_movx_reg_16bit
	jmp	ta_invalid_operand_size
      ta_movx_reg_8bit:
	call	ta_operand_autodetect
	jmp	ta_nomem_instruction_ready
      ta_movx_reg_16bit:
	call	ta_operand_autodetect
	inc	[ta_extended_code]
	jmp	ta_nomem_instruction_ready
ta_movsxd_instruction:
	mov	[ta_base_code],al
	call	ta_take_register
	mov	[ta_postbyte_register],al
	cmp	ah,8
	jne	ta_invalid_operand_size
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	mov	[ta_operand_size],0
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_movsxd_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	cmp	[ta_operand_size],4
	je	ta_movsxd_mem_store
	cmp	[ta_operand_size],0
	jne	ta_invalid_operand_size
      ta_movsxd_mem_store:
	call	ta_operand_64bit
	jmp	ta_instruction_ready
      ta_movsxd_reg:
	lods	byte [esi]
	call	ta_convert_register
	cmp	ah,4
	jne	ta_invalid_operand_size
	mov	bl,al
	call	ta_operand_64bit
	jmp	ta_nomem_instruction_ready
ta_bt_instruction:
	mov	[ta_postbyte_register],al
	shl	al,3
	add	al,83h
	mov	[ta_extended_code],al
	mov	[ta_base_code],0Fh
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_bt_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	push	eax ebx ecx
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	cmp	byte [esi],'('
	je	ta_bt_mem_imm
	cmp	byte [esi],11h
	jne	ta_bt_mem_reg
	cmp	byte [esi+2],'('
	je	ta_bt_mem_imm
      ta_bt_mem_reg:
	call	ta_take_register
	mov	[ta_postbyte_register],al
	pop	ecx ebx edx
	mov	al,ah
	call	ta_operand_autodetect
	jmp	ta_instruction_ready
      ta_bt_mem_imm:
	xor	al,al
	xchg	al,[ta_operand_size]
	push	eax
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'('
	jne	ta_invalid_operand
	mov	al,[ta_operand_size]
	or	al,al
	jz	ta_bt_mem_imm_size_ok
	cmp	al,1
	jne	ta_invalid_operand_size
      ta_bt_mem_imm_size_ok:
	call	ta_get_byte_value
	mov	byte [ta_value],al
	pop	eax
	or	al,al
	jz	ta_bt_mem_imm_nosize
	call	ta_operand_autodetect
      ta_bt_mem_imm_store:
	pop	ecx ebx edx
	mov	[ta_extended_code],0BAh
	call	ta_store_instruction_with_imm8
	jmp	ta_instruction_assembled
      ta_bt_mem_imm_nosize:
	call	ta_recoverable_unknown_size
	jmp	ta_bt_mem_imm_store
      ta_bt_reg:
	lods	byte [esi]
	call	ta_convert_register
	mov	bl,al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	cmp	byte [esi],'('
	je	ta_bt_reg_imm
	cmp	byte [esi],11h
	jne	ta_bt_reg_reg
	cmp	byte [esi+2],'('
	je	ta_bt_reg_imm
      ta_bt_reg_reg:
	call	ta_take_register
	mov	[ta_postbyte_register],al
	mov	al,ah
	call	ta_operand_autodetect
	jmp	ta_nomem_instruction_ready
      ta_bt_reg_imm:
	xor	al,al
	xchg	al,[ta_operand_size]
	push	eax ebx
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'('
	jne	ta_invalid_operand
	mov	al,[ta_operand_size]
	or	al,al
	jz	ta_bt_reg_imm_size_ok
	cmp	al,1
	jne	ta_invalid_operand_size
      ta_bt_reg_imm_size_ok:
	call	ta_get_byte_value
	mov	byte [ta_value],al
	pop	ebx eax
	call	ta_operand_autodetect
      ta_bt_reg_imm_store:
	mov	[ta_extended_code],0BAh
	call	ta_store_nomem_instruction
	mov	al,byte [ta_value]
	stos	byte [edi]
	jmp	ta_instruction_assembled
ta_bs_instruction:
	mov	[ta_extended_code],al
	mov	[ta_base_code],0Fh
	call	ta_get_reg_mem
	jc	ta_bs_reg_reg
	mov	al,[ta_operand_size]
	call	ta_operand_autodetect
	jmp	ta_instruction_ready
      ta_bs_reg_reg:
	mov	al,ah
	call	ta_operand_autodetect
	jmp	ta_nomem_instruction_ready
      ta_get_reg_mem:
	call	ta_take_register
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_get_reg_reg
	cmp	al,'['
	jne	ta_invalid_argument
	call	ta_get_address
	clc
	ret
      ta_get_reg_reg:
	lods	byte [esi]
	call	ta_convert_register
	mov	bl,al
	stc
	ret
ta_ud_instruction:
	mov	[ta_extended_code],al
	mov	[ta_base_code],0Fh
	call	ta_get_reg_mem
	jc	ta_ud_reg_reg
	cmp	[ta_operand_size],4
	jne	ta_invalid_operand_size
	jmp	ta_instruction_ready
      ta_ud_reg_reg:
	cmp	ah,4
	jne	ta_invalid_operand_size
	jmp	ta_nomem_instruction_ready

ta_imul_instruction:
	mov	[ta_base_code],0F6h
	mov	[ta_postbyte_register],5
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_imul_reg
	cmp	al,'['
	jne	ta_invalid_operand
      ta_imul_mem:
	call	ta_get_address
	mov	al,[ta_operand_size]
	cmp	al,1
	je	ta_imul_mem_8bit
	jb	ta_imul_mem_nosize
	call	ta_operand_autodetect
	inc	[ta_base_code]
	jmp	ta_instruction_ready
      ta_imul_mem_nosize:
	call	ta_recoverable_unknown_size
      ta_imul_mem_8bit:
	jmp	ta_instruction_ready
      ta_imul_reg:
	lods	byte [esi]
	call	ta_convert_register
	cmp	byte [esi],','
	je	ta_imul_reg_
	mov	bl,al
	mov	al,ah
	cmp	al,1
	je	ta_imul_reg_8bit
	call	ta_operand_autodetect
	inc	[ta_base_code]
      ta_imul_reg_8bit:
	jmp	ta_nomem_instruction_ready
      ta_imul_reg_:
	mov	[ta_postbyte_register],al
	inc	esi
	cmp	byte [esi],'('
	je	ta_imul_reg_imm
	cmp	byte [esi],11h
	jne	ta_imul_reg_noimm
	cmp	byte [esi+2],'('
	je	ta_imul_reg_imm
      ta_imul_reg_noimm:
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_imul_reg_reg
	cmp	al,'['
	jne	ta_invalid_operand
      ta_imul_reg_mem:
	call	ta_get_address
	push	edx ebx ecx
	cmp	byte [esi],','
	je	ta_imul_reg_mem_imm
	mov	al,[ta_operand_size]
	call	ta_operand_autodetect
	pop	ecx ebx edx
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],0AFh
	jmp	ta_instruction_ready
      ta_imul_reg_mem_imm:
	inc	esi
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'('
	jne	ta_invalid_operand
	mov	al,[ta_operand_size]
	cmp	al,2
	je	ta_imul_reg_mem_imm_16bit
	cmp	al,4
	je	ta_imul_reg_mem_imm_32bit
	cmp	al,8
	jne	ta_invalid_operand_size
      ta_imul_reg_mem_imm_64bit:
	cmp	[ta_size_declared],0
	jne	ta_long_immediate_not_encodable
	call	ta_operand_64bit
	call	ta_get_simm32
	cmp	[ta_value_type],4
	jae	ta_long_immediate_not_encodable
	jmp	ta_imul_reg_mem_imm_32bit_ok
      ta_imul_reg_mem_imm_16bit:
	call	ta_operand_16bit
	call	ta_get_word_value
	mov	word [ta_value],ax
	cmp	[ta_value_type],0
	jne	ta_imul_reg_mem_imm_16bit_store
	cmp	[ta_size_declared],0
	jne	ta_imul_reg_mem_imm_16bit_store
	cmp	ax,-80h
	jl	ta_imul_reg_mem_imm_16bit_store
	cmp	ax,80h
	jl	ta_imul_reg_mem_imm_8bit_store
      ta_imul_reg_mem_imm_16bit_store:
	pop	ecx ebx edx
	mov	[ta_base_code],69h
	call	ta_store_instruction_with_imm16
	jmp	ta_instruction_assembled
      ta_imul_reg_mem_imm_32bit:
	call	ta_operand_32bit
	call	ta_get_dword_value
      ta_imul_reg_mem_imm_32bit_ok:
	mov	dword [ta_value],eax
	cmp	[ta_value_type],0
	jne	ta_imul_reg_mem_imm_32bit_store
	cmp	[ta_size_declared],0
	jne	ta_imul_reg_mem_imm_32bit_store
	cmp	eax,-80h
	jl	ta_imul_reg_mem_imm_32bit_store
	cmp	eax,80h
	jl	ta_imul_reg_mem_imm_8bit_store
      ta_imul_reg_mem_imm_32bit_store:
	pop	ecx ebx edx
	mov	[ta_base_code],69h
	call	ta_store_instruction_with_imm32
	jmp	ta_instruction_assembled
      ta_imul_reg_mem_imm_8bit_store:
	pop	ecx ebx edx
	mov	[ta_base_code],6Bh
	call	ta_store_instruction_with_imm8
	jmp	ta_instruction_assembled
      ta_imul_reg_imm:
	mov	bl,[ta_postbyte_register]
	dec	esi
	jmp	ta_imul_reg_reg_imm
      ta_imul_reg_reg:
	lods	byte [esi]
	call	ta_convert_register
	mov	bl,al
	cmp	byte [esi],','
	je	ta_imul_reg_reg_imm
	mov	al,ah
	call	ta_operand_autodetect
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],0AFh
	jmp	ta_nomem_instruction_ready
      ta_imul_reg_reg_imm:
	inc	esi
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'('
	jne	ta_invalid_operand
	mov	al,[ta_operand_size]
	cmp	al,2
	je	ta_imul_reg_reg_imm_16bit
	cmp	al,4
	je	ta_imul_reg_reg_imm_32bit
	cmp	al,8
	jne	ta_invalid_operand_size
      ta_imul_reg_reg_imm_64bit:
	cmp	[ta_size_declared],0
	jne	ta_long_immediate_not_encodable
	call	ta_operand_64bit
	push	ebx
	call	ta_get_simm32
	cmp	[ta_value_type],4
	jae	ta_long_immediate_not_encodable
	jmp	ta_imul_reg_reg_imm_32bit_ok
      ta_imul_reg_reg_imm_16bit:
	call	ta_operand_16bit
	push	ebx
	call	ta_get_word_value
	pop	ebx
	mov	dx,ax
	cmp	[ta_value_type],0
	jne	ta_imul_reg_reg_imm_16bit_store
	cmp	[ta_size_declared],0
	jne	ta_imul_reg_reg_imm_16bit_store
	cmp	ax,-80h
	jl	ta_imul_reg_reg_imm_16bit_store
	cmp	ax,80h
	jl	ta_imul_reg_reg_imm_8bit_store
      ta_imul_reg_reg_imm_16bit_store:
	mov	[ta_base_code],69h
	call	ta_store_nomem_instruction
	mov	ax,dx
	call	ta_mark_relocation
	stos	word [edi]
	jmp	ta_instruction_assembled
      ta_imul_reg_reg_imm_32bit:
	call	ta_operand_32bit
	push	ebx
	call	ta_get_dword_value
      ta_imul_reg_reg_imm_32bit_ok:
	pop	ebx
	mov	edx,eax
	cmp	[ta_value_type],0
	jne	ta_imul_reg_reg_imm_32bit_store
	cmp	[ta_size_declared],0
	jne	ta_imul_reg_reg_imm_32bit_store
	cmp	eax,-80h
	jl	ta_imul_reg_reg_imm_32bit_store
	cmp	eax,80h
	jl	ta_imul_reg_reg_imm_8bit_store
      ta_imul_reg_reg_imm_32bit_store:
	mov	[ta_base_code],69h
	call	ta_store_nomem_instruction
	mov	eax,edx
	call	ta_mark_relocation
	stos	dword [edi]
	jmp	ta_instruction_assembled
      ta_imul_reg_reg_imm_8bit_store:
	mov	[ta_base_code],6Bh
	call	ta_store_nomem_instruction
	mov	al,dl
	stos	byte [edi]
	jmp	ta_instruction_assembled
ta_in_instruction:
	call	ta_take_register
	or	al,al
	jnz	ta_invalid_operand
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	mov	al,ah
	push	eax
	mov	[ta_operand_size],0
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'('
	je	ta_in_imm
	cmp	al,10h
	je	ta_in_reg
	jmp	ta_invalid_operand
      ta_in_reg:
	lods	byte [esi]
	cmp	al,22h
	jne	ta_invalid_operand
	pop	eax
	cmp	al,1
	je	ta_in_al_dx
	cmp	al,2
	je	ta_in_ax_dx
	cmp	al,4
	jne	ta_invalid_operand_size
      ta_in_ax_dx:
	call	ta_operand_autodetect
	mov	[ta_base_code],0EDh
	call	ta_store_classic_instruction_code
	jmp	ta_instruction_assembled
      ta_in_al_dx:
	mov	al,0ECh
	stos	byte [edi]
	jmp	ta_instruction_assembled
      ta_in_imm:
	mov	al,[ta_operand_size]
	or	al,al
	jz	ta_in_imm_size_ok
	cmp	al,1
	jne	ta_invalid_operand_size
      ta_in_imm_size_ok:
	call	ta_get_byte_value
	mov	dl,al
	pop	eax
	cmp	al,1
	je	ta_in_al_imm
	cmp	al,2
	je	ta_in_ax_imm
	cmp	al,4
	jne	ta_invalid_operand_size
      ta_in_ax_imm:
	call	ta_operand_autodetect
	mov	[ta_base_code],0E5h
	call	ta_store_classic_instruction_code
	mov	al,dl
	stos	byte [edi]
	jmp	ta_instruction_assembled
      ta_in_al_imm:
	mov	al,0E4h
	stos	byte [edi]
	mov	al,dl
	stos	byte [edi]
	jmp	ta_instruction_assembled
ta_out_instruction:
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'('
	je	ta_out_imm
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	cmp	al,22h
	jne	ta_invalid_operand
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	mov	[ta_operand_size],0
	call	ta_take_register
	or	al,al
	jnz	ta_invalid_operand
	mov	al,ah
	cmp	al,1
	je	ta_out_dx_al
	cmp	al,2
	je	ta_out_dx_ax
	cmp	al,4
	jne	ta_invalid_operand_size
      ta_out_dx_ax:
	call	ta_operand_autodetect
	mov	[ta_base_code],0EFh
	call	ta_store_classic_instruction_code
	jmp	ta_instruction_assembled
      ta_out_dx_al:
	mov	al,0EEh
	stos	byte [edi]
	jmp	ta_instruction_assembled
      ta_out_imm:
	mov	al,[ta_operand_size]
	or	al,al
	jz	ta_out_imm_size_ok
	cmp	al,1
	jne	ta_invalid_operand_size
      ta_out_imm_size_ok:
	call	ta_get_byte_value
	mov	dl,al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	mov	[ta_operand_size],0
	call	ta_take_register
	or	al,al
	jnz	ta_invalid_operand
	mov	al,ah
	cmp	al,1
	je	ta_out_imm_al
	cmp	al,2
	je	ta_out_imm_ax
	cmp	al,4
	jne	ta_invalid_operand_size
      ta_out_imm_ax:
	call	ta_operand_autodetect
	mov	[ta_base_code],0E7h
	call	ta_store_classic_instruction_code
	mov	al,dl
	stos	byte [edi]
	jmp	ta_instruction_assembled
      ta_out_imm_al:
	mov	al,0E6h
	stos	byte [edi]
	mov	al,dl
	stos	byte [edi]
	jmp	ta_instruction_assembled

ta_call_instruction:
	mov	[ta_postbyte_register],10b
	mov	[ta_base_code],0E8h
	mov	[ta_extended_code],9Ah
	jmp	ta_process_jmp
ta_jmp_instruction:
	mov	[ta_postbyte_register],100b
	mov	[ta_base_code],0E9h
	mov	[ta_extended_code],0EAh
      ta_process_jmp:
	lods	byte [esi]
	call	ta_get_jump_operator
	test	[ta_prefix_flags],10h
	jz	ta_jmp_type_ok
	test	[ta_jump_type],not 2
	jnz	ta_illegal_instruction
	mov	[ta_jump_type],2
	and	[ta_prefix_flags],not 10h
      ta_jmp_type_ok:
	call	ta_get_size_operator
	cmp	al,'('
	je	ta_jmp_imm
	mov	[ta_base_code],0FFh
	cmp	al,10h
	je	ta_jmp_reg
	cmp	al,'['
	jne	ta_invalid_operand
      ta_jmp_mem:
	cmp	[ta_jump_type],1
	je	ta_illegal_instruction
	call	ta_get_address
	mov	edx,eax
	mov	al,[ta_operand_size]
	or	al,al
	jz	ta_jmp_mem_size_not_specified
	cmp	al,2
	je	ta_jmp_mem_16bit
	cmp	al,4
	je	ta_jmp_mem_32bit
	cmp	al,6
	je	ta_jmp_mem_48bit
	cmp	al,8
	je	ta_jmp_mem_64bit
	cmp	al,10
	je	ta_jmp_mem_80bit
	jmp	ta_invalid_operand_size
      ta_jmp_mem_size_not_specified:
	cmp	[ta_jump_type],3
	je	ta_jmp_mem_far
	cmp	[ta_jump_type],2
	je	ta_jmp_mem_near
	call	ta_recoverable_unknown_size
      ta_jmp_mem_near:
	cmp	[ta_code_type],16
	je	ta_jmp_mem_16bit
	cmp	[ta_code_type],32
	je	ta_jmp_mem_near_32bit
      ta_jmp_mem_64bit:
	cmp	[ta_jump_type],3
	je	ta_invalid_operand_size
	cmp	[ta_code_type],64
	jne	ta_illegal_instruction
	jmp	ta_instruction_ready
      ta_jmp_mem_far:
	cmp	[ta_code_type],16
	je	ta_jmp_mem_far_32bit
      ta_jmp_mem_48bit:
	call	ta_operand_32bit
      ta_jmp_mem_far_store:
	cmp	[ta_jump_type],2
	je	ta_invalid_operand_size
	inc	[ta_postbyte_register]
	jmp	ta_instruction_ready
      ta_jmp_mem_80bit:
	call	ta_operand_64bit
	jmp	ta_jmp_mem_far_store
      ta_jmp_mem_far_32bit:
	call	ta_operand_16bit
	jmp	ta_jmp_mem_far_store
      ta_jmp_mem_32bit:
	cmp	[ta_jump_type],3
	je	ta_jmp_mem_far_32bit
	cmp	[ta_jump_type],2
	je	ta_jmp_mem_near_32bit
	cmp	[ta_code_type],16
	je	ta_jmp_mem_far_32bit
      ta_jmp_mem_near_32bit:
	cmp	[ta_code_type],64
	je	ta_illegal_instruction
	call	ta_operand_32bit
	jmp	ta_instruction_ready
      ta_jmp_mem_16bit:
	cmp	[ta_jump_type],3
	je	ta_invalid_operand_size
	call	ta_operand_16bit
	jmp	ta_instruction_ready
      ta_jmp_reg:
	test	[ta_jump_type],1
	jnz	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_register
	mov	bl,al
	mov	al,ah
	cmp	al,2
	je	ta_jmp_reg_16bit
	cmp	al,4
	je	ta_jmp_reg_32bit
	cmp	al,8
	jne	ta_invalid_operand_size
      ta_jmp_reg_64bit:
	cmp	[ta_code_type],64
	jne	ta_illegal_instruction
	jmp	ta_nomem_instruction_ready
      ta_jmp_reg_32bit:
	cmp	[ta_code_type],64
	je	ta_illegal_instruction
	call	ta_operand_32bit
	jmp	ta_nomem_instruction_ready
      ta_jmp_reg_16bit:
	call	ta_operand_16bit
	jmp	ta_nomem_instruction_ready
      ta_jmp_imm:
	cmp	byte [esi],'.'
	je	ta_invalid_value
	mov	ebx,esi
	dec	esi
	call	ta_skip_symbol
	xchg	esi,ebx
	cmp	byte [ebx],':'
	je	ta_jmp_far
	cmp	[ta_jump_type],3
	je	ta_invalid_operand
      ta_jmp_near:
	mov	al,[ta_operand_size]
	cmp	al,2
	je	ta_jmp_imm_16bit
	cmp	al,4
	je	ta_jmp_imm_32bit
	cmp	al,8
	je	ta_jmp_imm_64bit
	or	al,al
	jnz	ta_invalid_operand_size
	cmp	[ta_code_type],16
	je	ta_jmp_imm_16bit
	cmp	[ta_code_type],64
	je	ta_jmp_imm_64bit
      ta_jmp_imm_32bit:
	cmp	[ta_code_type],64
	je	ta_invalid_operand_size
	call	ta_get_address_dword_value
	cmp	[ta_code_type],16
	jne	ta_jmp_imm_32bit_prefix_ok
	mov	byte [edi],66h
	inc	edi
      ta_jmp_imm_32bit_prefix_ok:
	call	ta_calculate_jump_offset
	cdq
	call	ta_check_for_short_jump
	jc	ta_jmp_short
      ta_jmp_imm_32bit_store:
	mov	edx,eax
	sub	edx,3
	jno	ta_jmp_imm_32bit_ok
	cmp	[ta_code_type],64
	je	ta_jump_out_of_range
      ta_jmp_imm_32bit_ok:
	mov	al,[ta_base_code]
	stos	byte [edi]
	mov	eax,edx
	call	ta_mark_relocation
	stos	dword [edi]
	jmp	ta_instruction_assembled
      ta_jmp_imm_64bit:
	cmp	[ta_code_type],64
	jne	ta_invalid_operand_size
	call	ta_get_address_qword_value
	call	ta_calculate_jump_offset
	mov	ecx,edx
	cdq
	cmp	edx,ecx
	jne	ta_jump_out_of_range
	call	ta_check_for_short_jump
	jnc	ta_jmp_imm_32bit_store
      ta_jmp_short:
	mov	ah,al
	mov	al,0EBh
	stos	word [edi]
	jmp	ta_instruction_assembled
      ta_jmp_imm_16bit:
	call	ta_get_address_word_value
	cmp	[ta_code_type],16
	je	ta_jmp_imm_16bit_prefix_ok
	mov	byte [edi],66h
	inc	edi
      ta_jmp_imm_16bit_prefix_ok:
	call	ta_calculate_jump_offset
	cwde
	cdq
	call	ta_check_for_short_jump
	jc	ta_jmp_short
	cmp	[ta_value_type],0
	jne	ta_invalid_use_of_symbol
	mov	edx,eax
	dec	edx
	mov	al,[ta_base_code]
	stos	byte [edi]
	mov	eax,edx
	stos	word [edi]
	jmp	ta_instruction_assembled
      ta_calculate_jump_offset:
	add	edi,2
	mov	ebp,[ta_addressing_space]
	call	ta_calculate_relative_offset
	sub	edi,2
	ret
      ta_check_for_short_jump:
	cmp	[ta_jump_type],1
	je	ta_forced_short
	ja	ta_no_short_jump
	cmp	[ta_base_code],0E8h
	je	ta_no_short_jump
	cmp	[ta_value_type],0
	jne	ta_no_short_jump
	cmp	eax,80h
	jb	ta_short_jump
	cmp	eax,-80h
	jae	ta_short_jump
      ta_no_short_jump:
	clc
	ret
      ta_forced_short:
	cmp	[ta_base_code],0E8h
	je	ta_illegal_instruction
	cmp	[ta_next_pass_needed],0
	jne	ta_jmp_short_value_type_ok
	cmp	[ta_value_type],0
	jne	ta_invalid_use_of_symbol
      ta_jmp_short_value_type_ok:
	cmp	eax,-80h
	jae	ta_short_jump
	cmp	eax,80h
	jae	ta_jump_out_of_range
      ta_short_jump:
	stc
	ret
      ta_jump_out_of_range:
	cmp	[ta_error_line],0
	jne	ta_instruction_assembled
	mov	eax,[ta_current_line]
	mov	[ta_error_line],eax
	mov	[ta_error],ta_relative_jump_out_of_range
	jmp	ta_instruction_assembled
      ta_jmp_far:
	cmp	[ta_jump_type],2
	je	ta_invalid_operand
	cmp	[ta_code_type],64
	je	ta_illegal_instruction
	mov	al,[ta_extended_code]
	mov	[ta_base_code],al
	call	ta_get_word_value
	push	eax
	inc	esi
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_operand
	mov	al,[ta_value_type]
	push	eax
	push	[ta_symbol_identifier]
	cmp	byte [esi],'.'
	je	ta_invalid_value
	mov	al,[ta_operand_size]
	cmp	al,4
	je	ta_jmp_far_16bit
	cmp	al,6
	je	ta_jmp_far_32bit
	or	al,al
	jnz	ta_invalid_operand_size
	cmp	[ta_code_type],16
	jne	ta_jmp_far_32bit
      ta_jmp_far_16bit:
	call	ta_get_word_value
	mov	ebx,eax
	call	ta_operand_16bit
	call	ta_store_classic_instruction_code
	mov	ax,bx
	call	ta_mark_relocation
	stos	word [edi]
      ta_jmp_far_segment:
	pop	[ta_symbol_identifier]
	pop	eax
	mov	[ta_value_type],al
	pop	eax
	call	ta_mark_relocation
	stos	word [edi]
	jmp	ta_instruction_assembled
      ta_jmp_far_32bit:
	call	ta_get_dword_value
	mov	ebx,eax
	call	ta_operand_32bit
	call	ta_store_classic_instruction_code
	mov	eax,ebx
	call	ta_mark_relocation
	stos	dword [edi]
	jmp	ta_jmp_far_segment
ta_conditional_jump:
	mov	[ta_base_code],al
	and	[ta_prefix_flags],not 10h
	lods	byte [esi]
	call	ta_get_jump_operator
	cmp	[ta_jump_type],3
	je	ta_invalid_operand
	call	ta_get_size_operator
	cmp	al,'('
	jne	ta_invalid_operand
	cmp	byte [esi],'.'
	je	ta_invalid_value
	mov	al,[ta_operand_size]
	cmp	al,2
	je	ta_conditional_jump_16bit
	cmp	al,4
	je	ta_conditional_jump_32bit
	cmp	al,8
	je	ta_conditional_jump_64bit
	or	al,al
	jnz	ta_invalid_operand_size
	cmp	[ta_code_type],16
	je	ta_conditional_jump_16bit
	cmp	[ta_code_type],64
	je	ta_conditional_jump_64bit
      ta_conditional_jump_32bit:
	cmp	[ta_code_type],64
	je	ta_invalid_operand_size
	call	ta_get_address_dword_value
	cmp	[ta_code_type],16
	jne	ta_conditional_jump_32bit_prefix_ok
	mov	byte [edi],66h
	inc	edi
      ta_conditional_jump_32bit_prefix_ok:
	call	ta_calculate_jump_offset
	cdq
	call	ta_check_for_short_jump
	jc	ta_conditional_jump_short
      ta_conditional_jump_32bit_store:
	mov	edx,eax
	sub	edx,4
	jno	ta_conditional_jump_32bit_range_ok
	cmp	[ta_code_type],64
	je	ta_jump_out_of_range
      ta_conditional_jump_32bit_range_ok:
	mov	ah,[ta_base_code]
	add	ah,10h
	mov	al,0Fh
	stos	word [edi]
	mov	eax,edx
	call	ta_mark_relocation
	stos	dword [edi]
	jmp	ta_instruction_assembled
      ta_conditional_jump_64bit:
	cmp	[ta_code_type],64
	jne	ta_invalid_operand_size
	call	ta_get_address_qword_value
	call	ta_calculate_jump_offset
	mov	ecx,edx
	cdq
	cmp	edx,ecx
	jne	ta_jump_out_of_range
	call	ta_check_for_short_jump
	jnc	ta_conditional_jump_32bit_store
      ta_conditional_jump_short:
	mov	ah,al
	mov	al,[ta_base_code]
	stos	word [edi]
	jmp	ta_instruction_assembled
      ta_conditional_jump_16bit:
	call	ta_get_address_word_value
	cmp	[ta_code_type],16
	je	ta_conditional_jump_16bit_prefix_ok
	mov	byte [edi],66h
	inc	edi
      ta_conditional_jump_16bit_prefix_ok:
	call	ta_calculate_jump_offset
	cwde
	cdq
	call	ta_check_for_short_jump
	jc	ta_conditional_jump_short
	cmp	[ta_value_type],0
	jne	ta_invalid_use_of_symbol
	mov	edx,eax
	sub	dx,2
	mov	ah,[ta_base_code]
	add	ah,10h
	mov	al,0Fh
	stos	word [edi]
	mov	eax,edx
	stos	word [edi]
	jmp	ta_instruction_assembled
ta_loop_instruction_16bit:
	cmp	[ta_code_type],64
	je	ta_illegal_instruction
	cmp	[ta_code_type],16
	je	ta_loop_instruction
	mov	[ta_operand_prefix],67h
	jmp	ta_loop_instruction
ta_loop_instruction_32bit:
	cmp	[ta_code_type],32
	je	ta_loop_instruction
	mov	[ta_operand_prefix],67h
      jmp     ta_loop_instruction
ta_loop_instruction_64bit:
	cmp	[ta_code_type],64
	jne	ta_illegal_instruction
ta_loop_instruction:
	mov	[ta_base_code],al
	lods	byte [esi]
	call	ta_get_jump_operator
	cmp	[ta_jump_type],1
	ja	ta_invalid_operand
	call	ta_get_size_operator
	cmp	al,'('
	jne	ta_invalid_operand
	cmp	byte [esi],'.'
	je	ta_invalid_value
	mov	al,[ta_operand_size]
	cmp	al,2
	je	ta_loop_jump_16bit
	cmp	al,4
	je	ta_loop_jump_32bit
	cmp	al,8
	je	ta_loop_jump_64bit
	or	al,al
	jnz	ta_invalid_operand_size
	cmp	[ta_code_type],16
	je	ta_loop_jump_16bit
	cmp	[ta_code_type],64
	je	ta_loop_jump_64bit
      ta_loop_jump_32bit:
	cmp	[ta_code_type],64
	je	ta_invalid_operand_size
	call	ta_get_address_dword_value
	cmp	[ta_code_type],16
	jne	ta_loop_jump_32bit_prefix_ok
	mov	byte [edi],66h
	inc	edi
      ta_loop_jump_32bit_prefix_ok:
	call	ta_loop_counter_size
	call	ta_calculate_jump_offset
	cdq
      ta_make_loop_jump:
	call	ta_check_for_short_jump
	jc	ta_conditional_jump_short
	scas	word [edi]
	jmp	ta_jump_out_of_range
      ta_loop_counter_size:
	cmp	[ta_operand_prefix],0
	je	ta_loop_counter_size_ok
	push	eax
	mov	al,[ta_operand_prefix]
	stos	byte [edi]
	pop	eax
      ta_loop_counter_size_ok:
	ret
      ta_loop_jump_64bit:
	cmp	[ta_code_type],64
	jne	ta_invalid_operand_size
	call	ta_get_address_qword_value
	call	ta_loop_counter_size
	call	ta_calculate_jump_offset
	mov	ecx,edx
	cdq
	cmp	edx,ecx
	jne	ta_jump_out_of_range
	jmp	ta_make_loop_jump
      ta_loop_jump_16bit:
	call	ta_get_address_word_value
	cmp	[ta_code_type],16
	je	ta_loop_jump_16bit_prefix_ok
	mov	byte [edi],66h
	inc	edi
      ta_loop_jump_16bit_prefix_ok:
	call	ta_loop_counter_size
	call	ta_calculate_jump_offset
	cwde
	cdq
	jmp	ta_make_loop_jump

ta_movs_instruction:
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	or	eax,eax
	jnz	ta_invalid_address
	or	bl,ch
	jnz	ta_invalid_address
	cmp	[ta_segment_register],1
	ja	ta_invalid_address
	push	ebx
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	pop	edx
	or	eax,eax
	jnz	ta_invalid_address
	or	bl,ch
	jnz	ta_invalid_address
	mov	al,dh
	mov	ah,bh
	shr	al,4
	shr	ah,4
	cmp	al,ah
	jne	ta_address_sizes_do_not_agree
	and	bh,111b
	and	dh,111b
	cmp	bh,6
	jne	ta_invalid_address
	cmp	dh,7
	jne	ta_invalid_address
	cmp	al,2
	je	ta_movs_address_16bit
	cmp	al,4
	je	ta_movs_address_32bit
	cmp	[ta_code_type],64
	jne	ta_invalid_address_size
	jmp	ta_movs_store
      ta_movs_address_32bit:
	call	ta_address_32bit_prefix
	jmp	ta_movs_store
      ta_movs_address_16bit:
	cmp	[ta_code_type],64
	je	ta_invalid_address_size
	call	ta_address_16bit_prefix
      ta_movs_store:
	xor	ebx,ebx
	call	ta_store_segment_prefix_if_necessary
	mov	al,0A4h
      ta_movs_check_size:
	mov	bl,[ta_operand_size]
	cmp	bl,1
	je	ta_simple_instruction
	inc	al
	cmp	bl,2
	je	ta_simple_instruction_16bit
	cmp	bl,4
	je	ta_simple_instruction_32bit
	cmp	bl,8
	je	ta_simple_instruction_64bit
	or	bl,bl
	jnz	ta_invalid_operand_size
	call	ta_recoverable_unknown_size
	jmp	ta_simple_instruction
ta_lods_instruction:
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	or	eax,eax
	jnz	ta_invalid_address
	or	bl,ch
	jnz	ta_invalid_address
	cmp	bh,26h
	je	ta_lods_address_16bit
	cmp	bh,46h
	je	ta_lods_address_32bit
	cmp	bh,86h
	jne	ta_invalid_address
	cmp	[ta_code_type],64
	jne	ta_invalid_address_size
	jmp	ta_lods_store
      ta_lods_address_32bit:
	call	ta_address_32bit_prefix
	jmp	ta_lods_store
      ta_lods_address_16bit:
	cmp	[ta_code_type],64
	je	ta_invalid_address_size
	call	ta_address_16bit_prefix
      ta_lods_store:
	xor	ebx,ebx
	call	ta_store_segment_prefix_if_necessary
	mov	al,0ACh
	jmp	ta_movs_check_size
ta_stos_instruction:
	mov	[ta_base_code],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	or	eax,eax
	jnz	ta_invalid_address
	or	bl,ch
	jnz	ta_invalid_address
	cmp	bh,27h
	je	ta_stos_address_16bit
	cmp	bh,47h
	je	ta_stos_address_32bit
	cmp	bh,87h
	jne	ta_invalid_address
	cmp	[ta_code_type],64
	jne	ta_invalid_address_size
	jmp	ta_stos_store
      ta_stos_address_32bit:
	call	ta_address_32bit_prefix
	jmp	ta_stos_store
      ta_stos_address_16bit:
	cmp	[ta_code_type],64
	je	ta_invalid_address_size
	call	ta_address_16bit_prefix
      ta_stos_store:
	cmp	[ta_segment_register],1
	ja	ta_invalid_address
	mov	al,[ta_base_code]
	jmp	ta_movs_check_size
ta_cmps_instruction:
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	or	eax,eax
	jnz	ta_invalid_address
	or	bl,ch
	jnz	ta_invalid_address
	mov	al,[ta_segment_register]
	push	eax ebx
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	or	eax,eax
	jnz	ta_invalid_address
	or	bl,ch
	jnz	ta_invalid_address
	pop	edx eax
	cmp	[ta_segment_register],1
	ja	ta_invalid_address
	mov	[ta_segment_register],al
	mov	al,dh
	mov	ah,bh
	shr	al,4
	shr	ah,4
	cmp	al,ah
	jne	ta_address_sizes_do_not_agree
	and	bh,111b
	and	dh,111b
	cmp	bh,7
	jne	ta_invalid_address
	cmp	dh,6
	jne	ta_invalid_address
	cmp	al,2
	je	ta_cmps_address_16bit
	cmp	al,4
	je	ta_cmps_address_32bit
	cmp	[ta_code_type],64
	jne	ta_invalid_address_size
	jmp	ta_cmps_store
      ta_cmps_address_32bit:
	call	ta_address_32bit_prefix
	jmp	ta_cmps_store
      ta_cmps_address_16bit:
	cmp	[ta_code_type],64
	je	ta_invalid_address_size
	call	ta_address_16bit_prefix
      ta_cmps_store:
	xor	ebx,ebx
	call	ta_store_segment_prefix_if_necessary
	mov	al,0A6h
	jmp	ta_movs_check_size
ta_ins_instruction:
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	or	eax,eax
	jnz	ta_invalid_address
	or	bl,ch
	jnz	ta_invalid_address
	cmp	bh,27h
	je	ta_ins_address_16bit
	cmp	bh,47h
	je	ta_ins_address_32bit
	cmp	bh,87h
	jne	ta_invalid_address
	cmp	[ta_code_type],64
	jne	ta_invalid_address_size
	jmp	ta_ins_store
      ta_ins_address_32bit:
	call	ta_address_32bit_prefix
	jmp	ta_ins_store
      ta_ins_address_16bit:
	cmp	[ta_code_type],64
	je	ta_invalid_address_size
	call	ta_address_16bit_prefix
      ta_ins_store:
	cmp	[ta_segment_register],1
	ja	ta_invalid_address
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	cmp	al,22h
	jne	ta_invalid_operand
	mov	al,6Ch
      ta_ins_check_size:
	cmp	[ta_operand_size],8
	jne	ta_movs_check_size
	jmp	ta_invalid_operand_size
ta_outs_instruction:
	lods	byte [esi]
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	cmp	al,22h
	jne	ta_invalid_operand
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	or	eax,eax
	jnz	ta_invalid_address
	or	bl,ch
	jnz	ta_invalid_address
	cmp	bh,26h
	je	ta_outs_address_16bit
	cmp	bh,46h
	je	ta_outs_address_32bit
	cmp	bh,86h
	jne	ta_invalid_address
	cmp	[ta_code_type],64
	jne	ta_invalid_address_size
	jmp	ta_outs_store
      ta_outs_address_32bit:
	call	ta_address_32bit_prefix
	jmp	ta_outs_store
      ta_outs_address_16bit:
	cmp	[ta_code_type],64
	je	ta_invalid_address_size
	call	ta_address_16bit_prefix
      ta_outs_store:
	xor	ebx,ebx
	call	ta_store_segment_prefix_if_necessary
	mov	al,6Eh
	jmp	ta_ins_check_size
ta_xlat_instruction:
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	or	eax,eax
	jnz	ta_invalid_address
	or	bl,ch
	jnz	ta_invalid_address
	cmp	bh,23h
	je	ta_xlat_address_16bit
	cmp	bh,43h
	je	ta_xlat_address_32bit
	cmp	bh,83h
	jne	ta_invalid_address
	cmp	[ta_code_type],64
	jne	ta_invalid_address_size
	jmp	ta_xlat_store
      ta_xlat_address_32bit:
	call	ta_address_32bit_prefix
	jmp	ta_xlat_store
      ta_xlat_address_16bit:
	cmp	[ta_code_type],64
	je	ta_invalid_address_size
	call	ta_address_16bit_prefix
      ta_xlat_store:
	call	ta_store_segment_prefix_if_necessary
	mov	al,0D7h
	cmp	[ta_operand_size],1
	jbe	ta_simple_instruction
	jmp	ta_invalid_operand_size

ta_pm_word_instruction:
	mov	ah,al
	shr	ah,4
	and	al,111b
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],ah
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_pm_reg
      ta_pm_mem:
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	mov	al,[ta_operand_size]
	cmp	al,2
	je	ta_pm_mem_store
	or	al,al
	jnz	ta_invalid_operand_size
      ta_pm_mem_store:
	jmp	ta_instruction_ready
      ta_pm_reg:
	lods	byte [esi]
	call	ta_convert_register
	mov	bl,al
	cmp	ah,2
	jne	ta_invalid_operand_size
	jmp	ta_nomem_instruction_ready
ta_pm_store_word_instruction:
	mov	ah,al
	shr	ah,4
	and	al,111b
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],ah
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_pm_mem
	lods	byte [esi]
	call	ta_convert_register
	mov	bl,al
	mov	al,ah
	call	ta_operand_autodetect
	jmp	ta_nomem_instruction_ready
ta_lgdt_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],1
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	mov	al,[ta_operand_size]
	cmp	al,6
	je	ta_lgdt_mem_48bit
	cmp	al,10
	je	ta_lgdt_mem_80bit
	or	al,al
	jnz	ta_invalid_operand_size
	jmp	ta_lgdt_mem_store
      ta_lgdt_mem_80bit:
	cmp	[ta_code_type],64
	jne	ta_illegal_instruction
	jmp	ta_lgdt_mem_store
      ta_lgdt_mem_48bit:
	cmp	[ta_code_type],64
	je	ta_illegal_instruction
	cmp	[ta_postbyte_register],2
	jb	ta_lgdt_mem_store
	call	ta_operand_32bit
      ta_lgdt_mem_store:
	jmp	ta_instruction_ready
ta_lar_instruction:
	mov	[ta_extended_code],al
	mov	[ta_base_code],0Fh
	call	ta_take_register
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	xor	al,al
	xchg	al,[ta_operand_size]
	call	ta_operand_autodetect
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_lar_reg_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	mov	al,[ta_operand_size]
	or	al,al
	jz	ta_lar_reg_mem
	cmp	al,2
	jne	ta_invalid_operand_size
      ta_lar_reg_mem:
	jmp	ta_instruction_ready
      ta_lar_reg_reg:
	lods	byte [esi]
	call	ta_convert_register
	cmp	ah,2
	jne	ta_invalid_operand_size
	mov	bl,al
	jmp	ta_nomem_instruction_ready
ta_invlpg_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],1
	mov	[ta_postbyte_register],7
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	jmp	ta_instruction_ready
ta_simple_instruction_f2_0f_01:
	mov	byte [edi],0F2h
	inc	edi
	jmp	ta_simple_instruction_0f_01
ta_simple_instruction_f3_0f_01_64bit:
	cmp	[ta_code_type],64
	jne	ta_illegal_instruction
ta_simple_instruction_f3_0f_01:
	mov	byte [edi],0F3h
	inc	edi
	jmp	ta_simple_instruction_0f_01
ta_swapgs_instruction:
	cmp	[ta_code_type],64
	jne	ta_illegal_instruction
ta_simple_instruction_0f_01:
	mov	ah,al
	mov	al,0Fh
	stos	byte [edi]
	mov	al,1
	stos	word [edi]
	jmp	ta_instruction_assembled

ta_basic_486_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_basic_486_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	push	edx ebx ecx
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_register
	mov	[ta_postbyte_register],al
	pop	ecx ebx edx
	mov	al,ah
	cmp	al,1
	je	ta_basic_486_mem_reg_8bit
	call	ta_operand_autodetect
	inc	[ta_extended_code]
      ta_basic_486_mem_reg_8bit:
	jmp	ta_instruction_ready
      ta_basic_486_reg:
	lods	byte [esi]
	call	ta_convert_register
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_register
	mov	bl,al
	xchg	bl,[ta_postbyte_register]
	mov	al,ah
	cmp	al,1
	je	ta_basic_486_reg_reg_8bit
	call	ta_operand_autodetect
	inc	[ta_extended_code]
      ta_basic_486_reg_reg_8bit:
	jmp	ta_nomem_instruction_ready
ta_bswap_instruction:
	call	ta_take_register
	test	al,1000b
	jz	ta_bswap_reg_code_ok
	or	[ta_rex_prefix],41h
	and	al,111b
      ta_bswap_reg_code_ok:
	add	al,0C8h
	mov	[ta_extended_code],al
	mov	[ta_base_code],0Fh
	cmp	ah,8
	je	ta_bswap_reg64
	cmp	ah,4
	jne	ta_invalid_operand_size
	call	ta_operand_32bit
	call	ta_store_classic_instruction_code
	jmp	ta_instruction_assembled
      ta_bswap_reg64:
	call	ta_operand_64bit
	call	ta_store_classic_instruction_code
	jmp	ta_instruction_assembled
ta_cmpxchgx_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],0C7h
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	mov	ah,1
	xchg	[ta_postbyte_register],ah
	mov	al,[ta_operand_size]
	or	al,al
	jz	ta_cmpxchgx_size_ok
	cmp	al,ah
	jne	ta_invalid_operand_size
      ta_cmpxchgx_size_ok:
	cmp	ah,16
	jne	ta_cmpxchgx_store
	call	ta_operand_64bit
      ta_cmpxchgx_store:
	jmp	ta_instruction_ready
ta_nop_instruction:
	mov	ah,[esi]
	cmp	ah,10h
	je	ta_extended_nop
	cmp	ah,11h
	je	ta_extended_nop
	cmp	ah,'['
	je	ta_extended_nop
	stos	byte [edi]
	jmp	ta_instruction_assembled
      ta_extended_nop:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],1Fh
	mov	[ta_postbyte_register],0
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_extended_nop_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	mov	al,[ta_operand_size]
	or	al,al
	jz	ta_extended_nop_store
	call	ta_operand_autodetect
      ta_extended_nop_store:
	jmp	ta_instruction_ready
      ta_extended_nop_reg:
	lods	byte [esi]
	call	ta_convert_register
	mov	bl,al
	mov	al,ah
	call	ta_operand_autodetect
	jmp	ta_nomem_instruction_ready

ta_basic_fpu_instruction:
	mov	[ta_postbyte_register],al
	mov	[ta_base_code],0D8h
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_basic_fpu_streg
	cmp	al,'['
	je	ta_basic_fpu_mem
	dec	esi
	mov	ah,[ta_postbyte_register]
	cmp	ah,2
	jb	ta_invalid_operand
	cmp	ah,3
	ja	ta_invalid_operand
	mov	bl,1
	jmp	ta_nomem_instruction_ready
      ta_basic_fpu_mem:
	call	ta_get_address
	mov	al,[ta_operand_size]
	cmp	al,4
	je	ta_basic_fpu_mem_32bit
	cmp	al,8
	je	ta_basic_fpu_mem_64bit
	or	al,al
	jnz	ta_invalid_operand_size
	call	ta_recoverable_unknown_size
      ta_basic_fpu_mem_32bit:
	jmp	ta_instruction_ready
      ta_basic_fpu_mem_64bit:
	mov	[ta_base_code],0DCh
	jmp	ta_instruction_ready
      ta_basic_fpu_streg:
	lods	byte [esi]
	call	ta_convert_fpu_register
	mov	bl,al
	mov	ah,[ta_postbyte_register]
	cmp	ah,2
	je	ta_basic_fpu_single_streg
	cmp	ah,3
	je	ta_basic_fpu_single_streg
	or	al,al
	jz	ta_basic_fpu_st0
	test	ah,110b
	jz	ta_basic_fpu_streg_st0
	xor	[ta_postbyte_register],1
      ta_basic_fpu_streg_st0:
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_fpu_register
	or	al,al
	jnz	ta_invalid_operand
	mov	[ta_base_code],0DCh
	jmp	ta_nomem_instruction_ready
      ta_basic_fpu_st0:
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_fpu_register
	mov	bl,al
      ta_basic_fpu_single_streg:
	mov	[ta_base_code],0D8h
	jmp	ta_nomem_instruction_ready
ta_simple_fpu_instruction:
	mov	ah,al
	or	ah,11000000b
	mov	al,0D9h
	stos	word [edi]
	jmp	ta_instruction_assembled
ta_fi_instruction:
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	mov	al,[ta_operand_size]
	cmp	al,2
	je	ta_fi_mem_16bit
	cmp	al,4
	je	ta_fi_mem_32bit
	or	al,al
	jnz	ta_invalid_operand_size
	call	ta_recoverable_unknown_size
      ta_fi_mem_32bit:
	mov	[ta_base_code],0DAh
	jmp	ta_instruction_ready
      ta_fi_mem_16bit:
	mov	[ta_base_code],0DEh
	jmp	ta_instruction_ready
ta_fld_instruction:
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_fld_streg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	mov	al,[ta_operand_size]
	cmp	al,4
	je	ta_fld_mem_32bit
	cmp	al,8
	je	ta_fld_mem_64bit
	cmp	al,10
	je	ta_fld_mem_80bit
	or	al,al
	jnz	ta_invalid_operand_size
	call	ta_recoverable_unknown_size
      ta_fld_mem_32bit:
	mov	[ta_base_code],0D9h
	jmp	ta_instruction_ready
      ta_fld_mem_64bit:
	mov	[ta_base_code],0DDh
	jmp	ta_instruction_ready
      ta_fld_mem_80bit:
	mov	al,[ta_postbyte_register]
	cmp	al,0
	je	ta_fld_mem_80bit_store
	dec	[ta_postbyte_register]
	cmp	al,3
	je	ta_fld_mem_80bit_store
	jmp	ta_invalid_operand_size
      ta_fld_mem_80bit_store:
	add	[ta_postbyte_register],5
	mov	[ta_base_code],0DBh
	jmp	ta_instruction_ready
      ta_fld_streg:
	lods	byte [esi]
	call	ta_convert_fpu_register
	mov	bl,al
	cmp	[ta_postbyte_register],2
	jae	ta_fst_streg
	mov	[ta_base_code],0D9h
	jmp	ta_nomem_instruction_ready
      ta_fst_streg:
	mov	[ta_base_code],0DDh
	jmp	ta_nomem_instruction_ready
ta_fild_instruction:
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	mov	al,[ta_operand_size]
	cmp	al,2
	je	ta_fild_mem_16bit
	cmp	al,4
	je	ta_fild_mem_32bit
	cmp	al,8
	je	ta_fild_mem_64bit
	or	al,al
	jnz	ta_invalid_operand_size
	call	ta_recoverable_unknown_size
      ta_fild_mem_32bit:
	mov	[ta_base_code],0DBh
	jmp	ta_instruction_ready
      ta_fild_mem_16bit:
	mov	[ta_base_code],0DFh
	jmp	ta_instruction_ready
      ta_fild_mem_64bit:
	mov	al,[ta_postbyte_register]
	cmp	al,1
	je	ta_fisttp_64bit_store
	jb	ta_fild_mem_64bit_store
	dec	[ta_postbyte_register]
	cmp	al,3
	je	ta_fild_mem_64bit_store
	jmp	ta_invalid_operand_size
      ta_fild_mem_64bit_store:
	add	[ta_postbyte_register],5
	mov	[ta_base_code],0DFh
	jmp	ta_instruction_ready
      ta_fisttp_64bit_store:
	mov	[ta_base_code],0DDh
	jmp	ta_instruction_ready
ta_fbld_instruction:
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	mov	al,[ta_operand_size]
	or	al,al
	jz	ta_fbld_mem_80bit
	cmp	al,10
	je	ta_fbld_mem_80bit
	jmp	ta_invalid_operand_size
      ta_fbld_mem_80bit:
	mov	[ta_base_code],0DFh
	jmp	ta_instruction_ready
ta_faddp_instruction:
	mov	[ta_postbyte_register],al
	mov	[ta_base_code],0DEh
	mov	edx,esi
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_faddp_streg
	mov	esi,edx
	mov	bl,1
	jmp	ta_nomem_instruction_ready
      ta_faddp_streg:
	lods	byte [esi]
	call	ta_convert_fpu_register
	mov	bl,al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_fpu_register
	or	al,al
	jnz	ta_invalid_operand
	jmp	ta_nomem_instruction_ready
ta_fcompp_instruction:
	mov	ax,0D9DEh
	stos	word [edi]
	jmp	ta_instruction_assembled
ta_fucompp_instruction:
	mov	ax,0E9DAh
	stos	word [edi]
	jmp	ta_instruction_assembled
ta_fxch_instruction:
	mov	dx,01D9h
	jmp	ta_fpu_single_operand
ta_ffreep_instruction:
	mov	dx,00DFh
	jmp	ta_fpu_single_operand
ta_ffree_instruction:
	mov	dl,0DDh
	mov	dh,al
      ta_fpu_single_operand:
	mov	ebx,esi
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_fpu_streg
	or	dh,dh
	jz	ta_invalid_operand
	mov	esi,ebx
	shl	dh,3
	or	dh,11000001b
	mov	ax,dx
	stos	word [edi]
	jmp	ta_instruction_assembled
      ta_fpu_streg:
	lods	byte [esi]
	call	ta_convert_fpu_register
	shl	dh,3
	or	dh,al
	or	dh,11000000b
	mov	ax,dx
	stos	word [edi]
	jmp	ta_instruction_assembled

ta_fstenv_instruction:
	mov	byte [edi],9Bh
	inc	edi
ta_fldenv_instruction:
	mov	[ta_base_code],0D9h
	jmp	ta_fpu_mem
ta_fstenv_instruction_16bit:
	mov	byte [edi],9Bh
	inc	edi
ta_fldenv_instruction_16bit:
	call	ta_operand_16bit
	jmp	ta_fldenv_instruction
ta_fstenv_instruction_32bit:
	mov	byte [edi],9Bh
	inc	edi
ta_fldenv_instruction_32bit:
	call	ta_operand_32bit
	jmp	ta_fldenv_instruction
ta_fsave_instruction_32bit:
	mov	byte [edi],9Bh
	inc	edi
ta_fnsave_instruction_32bit:
	call	ta_operand_32bit
	jmp	ta_fnsave_instruction
ta_fsave_instruction_16bit:
	mov	byte [edi],9Bh
	inc	edi
ta_fnsave_instruction_16bit:
	call	ta_operand_16bit
	jmp	ta_fnsave_instruction
ta_fsave_instruction:
	mov	byte [edi],9Bh
	inc	edi
ta_fnsave_instruction:
	mov	[ta_base_code],0DDh
      ta_fpu_mem:
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	cmp	[ta_operand_size],0
	jne	ta_invalid_operand_size
	jmp	ta_instruction_ready
ta_fstcw_instruction:
	mov	byte [edi],9Bh
	inc	edi
ta_fldcw_instruction:
	mov	[ta_postbyte_register],al
	mov	[ta_base_code],0D9h
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	mov	al,[ta_operand_size]
	or	al,al
	jz	ta_fldcw_mem_16bit
	cmp	al,2
	je	ta_fldcw_mem_16bit
	jmp	ta_invalid_operand_size
      ta_fldcw_mem_16bit:
	jmp	ta_instruction_ready
ta_fstsw_instruction:
	mov	al,9Bh
	stos	byte [edi]
ta_fnstsw_instruction:
	mov	[ta_base_code],0DDh
	mov	[ta_postbyte_register],7
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_fstsw_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	mov	al,[ta_operand_size]
	or	al,al
	jz	ta_fstsw_mem_16bit
	cmp	al,2
	je	ta_fstsw_mem_16bit
	jmp	ta_invalid_operand_size
      ta_fstsw_mem_16bit:
	jmp	ta_instruction_ready
      ta_fstsw_reg:
	lods	byte [esi]
	call	ta_convert_register
	cmp	ax,0200h
	jne	ta_invalid_operand
	mov	ax,0E0DFh
	stos	word [edi]
	jmp	ta_instruction_assembled
ta_finit_instruction:
	mov	byte [edi],9Bh
	inc	edi
ta_fninit_instruction:
	mov	ah,al
	mov	al,0DBh
	stos	word [edi]
	jmp	ta_instruction_assembled
ta_fcmov_instruction:
	mov	dh,0DAh
	jmp	ta_fcomi_streg
ta_fcomi_instruction:
	mov	dh,0DBh
	jmp	ta_fcomi_streg
ta_fcomip_instruction:
	mov	dh,0DFh
      ta_fcomi_streg:
	mov	dl,al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_fpu_register
	mov	ah,al
	cmp	byte [esi],','
	je	ta_fcomi_st0_streg
	add	ah,dl
	mov	al,dh
	stos	word [edi]
	jmp	ta_instruction_assembled
      ta_fcomi_st0_streg:
	or	ah,ah
	jnz	ta_invalid_operand
	inc	esi
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_fpu_register
	mov	ah,al
	add	ah,dl
	mov	al,dh
	stos	word [edi]
	jmp	ta_instruction_assembled

ta_basic_mmx_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
      ta_mmx_instruction:
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_mmx_register
	call	ta_make_mmx_prefix
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_mmx_mmreg_mmreg
	cmp	al,'['
	jne	ta_invalid_operand
      ta_mmx_mmreg_mem:
	call	ta_get_address
	jmp	ta_instruction_ready
      ta_mmx_mmreg_mmreg:
	lods	byte [esi]
	call	ta_convert_mmx_register
	mov	bl,al
	jmp	ta_nomem_instruction_ready
ta_mmx_bit_shift_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_mmx_register
	call	ta_make_mmx_prefix
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	mov	[ta_operand_size],0
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_mmx_mmreg_mmreg
	cmp	al,'('
	je	ta_mmx_ps_mmreg_imm8
	cmp	al,'['
	je	ta_mmx_mmreg_mem
	jmp	ta_invalid_operand
      ta_mmx_ps_mmreg_imm8:
	call	ta_get_byte_value
	mov	byte [ta_value],al
	test	[ta_operand_size],not 1
	jnz	ta_invalid_value
	mov	bl,[ta_extended_code]
	mov	al,bl
	shr	bl,4
	and	al,1111b
	add	al,70h
	mov	[ta_extended_code],al
	sub	bl,0Ch
	shl	bl,1
	xchg	bl,[ta_postbyte_register]
	call	ta_store_nomem_instruction
	mov	al,byte [ta_value]
	stos	byte [edi]
	jmp	ta_instruction_assembled
ta_pmovmskb_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	call	ta_take_register
	cmp	ah,4
	je	ta_pmovmskb_reg_size_ok
	cmp	[ta_code_type],64
	jne	ta_invalid_operand_size
	cmp	ah,8
	jnz	ta_invalid_operand_size
      ta_pmovmskb_reg_size_ok:
	mov	[ta_postbyte_register],al
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_mmx_register
	mov	bl,al
	call	ta_make_mmx_prefix
	cmp	[ta_extended_code],0C5h
	je	ta_mmx_nomem_imm8
	jmp	ta_nomem_instruction_ready
      ta_mmx_imm8:
	push	ebx ecx edx
	xor	cl,cl
	xchg	cl,[ta_operand_size]
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	test	ah,not 1
	jnz	ta_invalid_operand_size
	mov	[ta_operand_size],cl
	cmp	al,'('
	jne	ta_invalid_operand
	call	ta_get_byte_value
	mov	byte [ta_value],al
	pop	edx ecx ebx
	call	ta_store_instruction_with_imm8
	jmp	ta_instruction_assembled
      ta_mmx_nomem_imm8:
	call	ta_store_nomem_instruction
	call	ta_append_imm8
	jmp	ta_instruction_assembled
      ta_append_imm8:
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	test	ah,not 1
	jnz	ta_invalid_operand_size
	cmp	al,'('
	jne	ta_invalid_operand
	call	ta_get_byte_value
	stosb
	ret

ta_pinsrw_instruction:
	mov	[ta_extended_code],al
	mov	[ta_base_code],0Fh
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_mmx_register
	call	ta_make_mmx_prefix
	mov	[ta_postbyte_register],al
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_pinsrw_mmreg_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	cmp	[ta_operand_size],0
	je	ta_mmx_imm8
	cmp	[ta_operand_size],2
	jne	ta_invalid_operand_size
	jmp	ta_mmx_imm8
      ta_pinsrw_mmreg_reg:
	lods	byte [esi]
	call	ta_convert_register
	cmp	ah,4
	jne	ta_invalid_operand_size
	mov	bl,al
	jmp	ta_mmx_nomem_imm8
ta_pshufw_instruction:
	mov	[ta_mmx_size],8
	mov	[ta_opcode_prefix],al
	jmp	ta_pshuf_instruction
ta_pshufd_instruction:
	mov	[ta_mmx_size],16
	mov	[ta_opcode_prefix],al
      ta_pshuf_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],70h
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_mmx_register
	cmp	ah,[ta_mmx_size]
	jne	ta_invalid_operand_size
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_pshuf_mmreg_mmreg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	jmp	ta_mmx_imm8
      ta_pshuf_mmreg_mmreg:
	lods	byte [esi]
	call	ta_convert_mmx_register
	mov	bl,al
	jmp	ta_mmx_nomem_imm8
ta_movd_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],7Eh
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_movd_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	test	[ta_operand_size],not 4
	jnz	ta_invalid_operand_size
	call	ta_get_mmx_source_register
	jmp	ta_instruction_ready
      ta_movd_reg:
	lods	byte [esi]
	cmp	al,0B0h
	jae	ta_movd_mmreg
	call	ta_convert_register
	cmp	ah,4
	jne	ta_invalid_operand_size
	mov	bl,al
	call	ta_get_mmx_source_register
	jmp	ta_nomem_instruction_ready
      ta_movd_mmreg:
	mov	[ta_extended_code],6Eh
	call	ta_convert_mmx_register
	mov	[ta_postbyte_register],al
	call	ta_make_mmx_prefix
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_movd_mmreg_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	test	[ta_operand_size],not 4
	jnz	ta_invalid_operand_size
	jmp	ta_instruction_ready
      ta_movd_mmreg_reg:
	lods	byte [esi]
	call	ta_convert_register
	cmp	ah,4
	jne	ta_invalid_operand_size
	mov	bl,al
	jmp	ta_nomem_instruction_ready
      ta_get_mmx_source_register:
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_mmx_register
	mov	[ta_postbyte_register],al
      ta_make_mmx_prefix:
	cmp	[ta_operand_size],16
	jne	ta_no_mmx_prefix
	mov	[ta_operand_prefix],66h
      ta_no_mmx_prefix:
	ret
ta_movq_instruction:
	mov	[ta_base_code],0Fh
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_movq_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	test	[ta_operand_size],not 8
	jnz	ta_invalid_operand_size
	call	ta_get_mmx_source_register
	mov	al,7Fh
	cmp	ah,8
	je	ta_movq_mem_ready
	mov	al,0D6h
     ta_movq_mem_ready:
	mov	[ta_extended_code],al
	jmp	ta_instruction_ready
     ta_movq_reg:
	lods	byte [esi]
	cmp	al,0B0h
	jae	ta_movq_mmreg
	call	ta_convert_register
	cmp	ah,8
	jne	ta_invalid_operand_size
	mov	bl,al
	mov	[ta_extended_code],7Eh
	call	ta_operand_64bit
	call	ta_get_mmx_source_register
	jmp	ta_nomem_instruction_ready
     ta_movq_mmreg:
	call	ta_convert_mmx_register
	mov	[ta_postbyte_register],al
	mov	[ta_extended_code],6Fh
	mov	[ta_mmx_size],ah
	cmp	ah,16
	jne	ta_movq_mmreg_
	mov	[ta_extended_code],7Eh
	mov	[ta_opcode_prefix],0F3h
      ta_movq_mmreg_:
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	mov	[ta_operand_size],0
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_movq_mmreg_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	test	[ta_operand_size],not 8
	jnz	ta_invalid_operand_size
	jmp	ta_instruction_ready
      ta_movq_mmreg_reg:
	lods	byte [esi]
	cmp	al,0B0h
	jae	ta_movq_mmreg_mmreg
	mov	[ta_operand_size],0
	call	ta_convert_register
	cmp	ah,8
	jne	ta_invalid_operand_size
	mov	[ta_extended_code],6Eh
	mov	[ta_opcode_prefix],0
	mov	bl,al
	cmp	[ta_mmx_size],16
	jne	ta_movq_mmreg_reg_store
	mov	[ta_opcode_prefix],66h
      ta_movq_mmreg_reg_store:
	call	ta_operand_64bit
	jmp	ta_nomem_instruction_ready
      ta_movq_mmreg_mmreg:
	call	ta_convert_mmx_register
	cmp	ah,[ta_mmx_size]
	jne	ta_invalid_operand_size
	mov	bl,al
	jmp	ta_nomem_instruction_ready
ta_movdq_instruction:
	mov	[ta_opcode_prefix],al
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],6Fh
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_movdq_mmreg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_xmm_register
	mov	[ta_postbyte_register],al
	mov	[ta_extended_code],7Fh
	jmp	ta_instruction_ready
      ta_movdq_mmreg:
	lods	byte [esi]
	call	ta_convert_xmm_register
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_movdq_mmreg_mmreg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	jmp	ta_instruction_ready
      ta_movdq_mmreg_mmreg:
	lods	byte [esi]
	call	ta_convert_xmm_register
	mov	bl,al
	jmp	ta_nomem_instruction_ready
ta_lddqu_instruction:
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_xmm_register
	push	eax
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	pop	eax
	mov	[ta_postbyte_register],al
	mov	[ta_opcode_prefix],0F2h
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],0F0h
	jmp	ta_instruction_ready

ta_movdq2q_instruction:
	mov	[ta_opcode_prefix],0F2h
	mov	[ta_mmx_size],8
	jmp	ta_movq2dq_
ta_movq2dq_instruction:
	mov	[ta_opcode_prefix],0F3h
	mov	[ta_mmx_size],16
      ta_movq2dq_:
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_mmx_register
	cmp	ah,[ta_mmx_size]
	jne	ta_invalid_operand_size
	mov	[ta_postbyte_register],al
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_mmx_register
	xor	[ta_mmx_size],8+16
	cmp	ah,[ta_mmx_size]
	jne	ta_invalid_operand_size
	mov	bl,al
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],0D6h
	jmp	ta_nomem_instruction_ready

ta_sse_ps_instruction_imm8:
	mov	[ta_immediate_size],1
ta_sse_ps_instruction:
	mov	[ta_mmx_size],16
	jmp	ta_sse_instruction
ta_sse_pd_instruction_imm8:
	mov	[ta_immediate_size],1
ta_sse_pd_instruction:
	mov	[ta_mmx_size],16
	mov	[ta_opcode_prefix],66h
	jmp	ta_sse_instruction
ta_sse_ss_instruction:
	mov	[ta_mmx_size],4
	mov	[ta_opcode_prefix],0F3h
	jmp	ta_sse_instruction
ta_sse_sd_instruction:
	mov	[ta_mmx_size],8
	mov	[ta_opcode_prefix],0F2h
	jmp	ta_sse_instruction
ta_cmp_pd_instruction:
	mov	[ta_opcode_prefix],66h
ta_cmp_ps_instruction:
	mov	[ta_mmx_size],16
	mov	byte [ta_value],al
	mov	al,0C2h
	jmp	ta_sse_instruction
ta_cmp_ss_instruction:
	mov	[ta_mmx_size],4
	mov	[ta_opcode_prefix],0F3h
	jmp	ta_cmp_sx_instruction
ta_cmpsd_instruction:
	mov	al,0A7h
	mov	ah,[esi]
	or	ah,ah
	jz	ta_simple_instruction_32bit
	cmp	ah,0Fh
	je	ta_simple_instruction_32bit
	mov	al,-1
ta_cmp_sd_instruction:
	mov	[ta_mmx_size],8
	mov	[ta_opcode_prefix],0F2h
      ta_cmp_sx_instruction:
	mov	byte [ta_value],al
	mov	al,0C2h
	jmp	ta_sse_instruction
ta_comiss_instruction:
	mov	[ta_mmx_size],4
	jmp	ta_sse_instruction
ta_comisd_instruction:
	mov	[ta_mmx_size],8
	mov	[ta_opcode_prefix],66h
	jmp	ta_sse_instruction
ta_cvtdq2pd_instruction:
	mov	[ta_opcode_prefix],0F3h
ta_cvtps2pd_instruction:
	mov	[ta_mmx_size],8
	jmp	ta_sse_instruction
ta_cvtpd2dq_instruction:
	mov	[ta_mmx_size],16
	mov	[ta_opcode_prefix],0F2h
	jmp	ta_sse_instruction
ta_movshdup_instruction:
	mov	[ta_mmx_size],16
	mov	[ta_opcode_prefix],0F3h
ta_sse_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
      ta_sse_xmmreg:
	lods	byte [esi]
	call	ta_convert_xmm_register
      ta_sse_reg:
	mov	[ta_postbyte_register],al
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_sse_xmmreg_xmmreg
      ta_sse_reg_mem:
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	cmp	[ta_operand_size],0
	je	ta_sse_mem_size_ok
	mov	al,[ta_mmx_size]
	cmp	[ta_operand_size],al
	jne	ta_invalid_operand_size
      ta_sse_mem_size_ok:
	mov	al,[ta_extended_code]
	mov	ah,[ta_supplemental_code]
	cmp	al,0C2h
	je	ta_sse_cmp_mem_ok
	cmp	ax,443Ah
	je	ta_sse_cmp_mem_ok
	cmp	[ta_immediate_size],1
	je	ta_mmx_imm8
	cmp	[ta_immediate_size],-1
	jne	ta_sse_ok
	call	ta_take_additional_xmm0
	mov	[ta_immediate_size],0
      ta_sse_ok:
	jmp	ta_instruction_ready
      ta_sse_cmp_mem_ok:
	cmp	byte [ta_value],-1
	je	ta_mmx_imm8
	call	ta_store_instruction_with_imm8
	jmp	ta_instruction_assembled
      ta_sse_xmmreg_xmmreg:
	cmp	[ta_operand_prefix],66h
	jne	ta_sse_xmmreg_xmmreg_ok
	cmp	[ta_extended_code],12h
	je	ta_invalid_operand
	cmp	[ta_extended_code],16h
	je	ta_invalid_operand
      ta_sse_xmmreg_xmmreg_ok:
	lods	byte [esi]
	call	ta_convert_xmm_register
	mov	bl,al
	mov	al,[ta_extended_code]
	mov	ah,[ta_supplemental_code]
	cmp	al,0C2h
	je	ta_sse_cmp_nomem_ok
	cmp	ax,443Ah
	je	ta_sse_cmp_nomem_ok
	cmp	[ta_immediate_size],1
	je	ta_mmx_nomem_imm8
	cmp	[ta_immediate_size],-1
	jne	ta_sse_nomem_ok
	call	ta_take_additional_xmm0
	mov	[ta_immediate_size],0
      ta_sse_nomem_ok:
	jmp	ta_nomem_instruction_ready
      ta_sse_cmp_nomem_ok:
	cmp	byte [ta_value],-1
	je	ta_mmx_nomem_imm8
	call	ta_store_nomem_instruction
	mov	al,byte [ta_value]
	stosb
	jmp	ta_instruction_assembled
      ta_take_additional_xmm0:
	cmp	byte [esi],','
	jne	ta_additional_xmm0_ok
	inc	esi
	lods	byte [esi]
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_xmm_register
	test	al,al
	jnz	ta_invalid_operand
      ta_additional_xmm0_ok:
	ret

ta_pslldq_instruction:
	mov	[ta_postbyte_register],al
	mov	[ta_opcode_prefix],66h
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],73h
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_xmm_register
	mov	bl,al
	jmp	ta_mmx_nomem_imm8
ta_movpd_instruction:
	mov	[ta_opcode_prefix],66h
ta_movps_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	mov	[ta_mmx_size],16
	jmp	ta_sse_mov_instruction
ta_movss_instruction:
	mov	[ta_mmx_size],4
	mov	[ta_opcode_prefix],0F3h
	jmp	ta_sse_movs
ta_movsd_instruction:
	mov	al,0A5h
	mov	ah,[esi]
	or	ah,ah
	jz	ta_simple_instruction_32bit
	cmp	ah,0Fh
	je	ta_simple_instruction_32bit
	mov	[ta_mmx_size],8
	mov	[ta_opcode_prefix],0F2h
      ta_sse_movs:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],10h
	jmp	ta_sse_mov_instruction
ta_sse_mov_instruction:
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_sse_xmmreg
      ta_sse_mem:
	cmp	al,'['
	jne	ta_invalid_operand
	inc	[ta_extended_code]
	call	ta_get_address
	cmp	[ta_operand_size],0
	je	ta_sse_mem_xmmreg
	mov	al,[ta_mmx_size]
	cmp	[ta_operand_size],al
	jne	ta_invalid_operand_size
	mov	[ta_operand_size],0
      ta_sse_mem_xmmreg:
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_xmm_register
	mov	[ta_postbyte_register],al
	jmp	ta_instruction_ready
ta_movlpd_instruction:
	mov	[ta_opcode_prefix],66h
ta_movlps_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	mov	[ta_mmx_size],8
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_sse_mem
	lods	byte [esi]
	call	ta_convert_xmm_register
	mov	[ta_postbyte_register],al
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	jmp	ta_sse_reg_mem
ta_movhlps_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	mov	[ta_mmx_size],0
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_xmm_register
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_sse_xmmreg_xmmreg_ok
	jmp	ta_invalid_operand
ta_maskmovq_instruction:
	mov	cl,8
	jmp	ta_maskmov_instruction
ta_maskmovdqu_instruction:
	mov	cl,16
	mov	[ta_opcode_prefix],66h
      ta_maskmov_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],0F7h
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_mmx_register
	cmp	ah,cl
	jne	ta_invalid_operand_size
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_mmx_register
	mov	bl,al
	jmp	ta_nomem_instruction_ready
ta_movmskpd_instruction:
	mov	[ta_opcode_prefix],66h
ta_movmskps_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],50h
	call	ta_take_register
	mov	[ta_postbyte_register],al
	cmp	ah,4
	je	ta_movmskps_reg_ok
	cmp	ah,8
	jne	ta_invalid_operand_size
	cmp	[ta_code_type],64
	jne	ta_invalid_operand
      ta_movmskps_reg_ok:
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_sse_xmmreg_xmmreg_ok
	jmp	ta_invalid_operand

ta_cvtpi2pd_instruction:
	mov	[ta_opcode_prefix],66h
ta_cvtpi2ps_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_xmm_register
	mov	[ta_postbyte_register],al
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_cvtpi_xmmreg_xmmreg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	cmp	[ta_operand_size],0
	je	ta_cvtpi_size_ok
	cmp	[ta_operand_size],8
	jne	ta_invalid_operand_size
      ta_cvtpi_size_ok:
	jmp	ta_instruction_ready
      ta_cvtpi_xmmreg_xmmreg:
	lods	byte [esi]
	call	ta_convert_mmx_register
	cmp	ah,8
	jne	ta_invalid_operand_size
	mov	bl,al
	jmp	ta_nomem_instruction_ready
ta_cvtsi2ss_instruction:
	mov	[ta_opcode_prefix],0F3h
	jmp	ta_cvtsi_instruction
ta_cvtsi2sd_instruction:
	mov	[ta_opcode_prefix],0F2h
      ta_cvtsi_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_xmm_register
	mov	[ta_postbyte_register],al
      ta_cvtsi_xmmreg:
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_cvtsi_xmmreg_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	cmp	[ta_operand_size],0
	je	ta_cvtsi_size_ok
	cmp	[ta_operand_size],4
	je	ta_cvtsi_size_ok
	cmp	[ta_operand_size],8
	jne	ta_invalid_operand_size
	call	ta_operand_64bit
      ta_cvtsi_size_ok:
	jmp	ta_instruction_ready
      ta_cvtsi_xmmreg_reg:
	lods	byte [esi]
	call	ta_convert_register
	cmp	ah,4
	je	ta_cvtsi_xmmreg_reg_store
	cmp	ah,8
	jne	ta_invalid_operand_size
	call	ta_operand_64bit
      ta_cvtsi_xmmreg_reg_store:
	mov	bl,al
	jmp	ta_nomem_instruction_ready
ta_cvtps2pi_instruction:
	mov	[ta_mmx_size],8
	jmp	ta_cvtpd_instruction
ta_cvtpd2pi_instruction:
	mov	[ta_opcode_prefix],66h
	mov	[ta_mmx_size],16
      ta_cvtpd_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_mmx_register
	cmp	ah,8
	jne	ta_invalid_operand_size
	mov	[ta_operand_size],0
	jmp	ta_sse_reg
ta_cvtss2si_instruction:
	mov	[ta_opcode_prefix],0F3h
	mov	[ta_mmx_size],4
	jmp	ta_cvt2si_instruction
ta_cvtsd2si_instruction:
	mov	[ta_opcode_prefix],0F2h
	mov	[ta_mmx_size],8
      ta_cvt2si_instruction:
	mov	[ta_extended_code],al
	mov	[ta_base_code],0Fh
	call	ta_take_register
	mov	[ta_operand_size],0
	cmp	ah,4
	je	ta_sse_reg
	cmp	ah,8
	jne	ta_invalid_operand_size
	call	ta_operand_64bit
	jmp	ta_sse_reg

ta_ssse3_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],38h
	mov	[ta_supplemental_code],al
	jmp	ta_mmx_instruction
ta_palignr_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],3Ah
	mov	[ta_supplemental_code],0Fh
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_mmx_register
	call	ta_make_mmx_prefix
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_palignr_mmreg_mmreg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	jmp	ta_mmx_imm8
      ta_palignr_mmreg_mmreg:
	lods	byte [esi]
	call	ta_convert_mmx_register
	mov	bl,al
	jmp	ta_mmx_nomem_imm8

ta_sse4_instruction_38_xmm0:
	mov	[ta_immediate_size],-1
	jmp	ta_sse4_instruction_38
ta_sse4_instruction_66_38_xmm0:
	mov	[ta_immediate_size],-1
ta_sse4_instruction_66_38:
	mov	[ta_opcode_prefix],66h
ta_sse4_instruction_38:
	mov	[ta_mmx_size],16
	mov	[ta_supplemental_code],al
	mov	al,38h
	jmp	ta_sse_instruction
ta_sse4_ss_instruction_66_3a_imm8:
	mov	[ta_immediate_size],1
	mov	cl,4
	jmp	ta_sse4_instruction_66_3a_setup
ta_sse4_sd_instruction_66_3a_imm8:
	mov	[ta_immediate_size],1
	mov	cl,8
	jmp	ta_sse4_instruction_66_3a_setup
ta_sse4_instruction_66_3a_imm8:
	mov	[ta_immediate_size],1
	mov	cl,16
      ta_sse4_instruction_66_3a_setup:
	mov	[ta_opcode_prefix],66h
      ta_sse4_instruction_3a_setup:
	mov	[ta_supplemental_code],al
	mov	al,3Ah
	mov	[ta_mmx_size],cl
	jmp	ta_sse_instruction
ta_sse4_instruction_3a_imm8:
	mov	[ta_immediate_size],1
	mov	cl,16
	jmp	ta_sse4_instruction_3a_setup
ta_pclmulqdq_instruction:
	mov	byte [ta_value],al
	mov	al,44h
	mov	cl,16
	jmp	ta_sse4_instruction_66_3a_setup
ta_extractps_instruction:
	call	ta_setup_66_0f_3a
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_extractps_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	cmp	[ta_operand_size],4
	je	ta_extractps_size_ok
	cmp	[ta_operand_size],0
	jne	ta_invalid_operand_size
      ta_extractps_size_ok:
	push	edx ebx ecx
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_xmm_register
	mov	[ta_postbyte_register],al
	pop	ecx ebx edx
	jmp	ta_mmx_imm8
      ta_extractps_reg:
	lods	byte [esi]
	call	ta_convert_register
	push	eax
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_xmm_register
	mov	[ta_postbyte_register],al
	pop	ebx
	mov	al,bh
	cmp	al,4
	je	ta_mmx_nomem_imm8
	cmp	al,8
	jne	ta_invalid_operand_size
	cmp	[ta_code_type],64
	jne	ta_illegal_instruction
	jmp	ta_mmx_nomem_imm8
      ta_setup_66_0f_3a:
	mov	[ta_extended_code],3Ah
	mov	[ta_supplemental_code],al
	mov	[ta_base_code],0Fh
	mov	[ta_opcode_prefix],66h
	ret
ta_insertps_instruction:
	call	ta_setup_66_0f_3a
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_xmm_register
	mov	[ta_postbyte_register],al
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_insertps_xmmreg_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	cmp	[ta_operand_size],4
	je	ta_insertps_size_ok
	cmp	[ta_operand_size],0
	jne	ta_invalid_operand_size
      ta_insertps_size_ok:
	jmp	ta_mmx_imm8
      ta_insertps_xmmreg_reg:
	lods	byte [esi]
	call	ta_convert_mmx_register
	mov	bl,al
	jmp	ta_mmx_nomem_imm8
ta_pextrq_instruction:
	mov	[ta_mmx_size],8
	jmp	ta_pextr_instruction
ta_pextrd_instruction:
	mov	[ta_mmx_size],4
	jmp	ta_pextr_instruction
ta_pextrw_instruction:
	mov	[ta_mmx_size],2
	jmp	ta_pextr_instruction
ta_pextrb_instruction:
	mov	[ta_mmx_size],1
      ta_pextr_instruction:
	call	ta_setup_66_0f_3a
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_pextr_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	mov	al,[ta_mmx_size]
	cmp	al,[ta_operand_size]
	je	ta_pextr_size_ok
	cmp	[ta_operand_size],0
	jne	ta_invalid_operand_size
      ta_pextr_size_ok:
	cmp	al,8
	jne	ta_pextr_prefix_ok
	call	ta_operand_64bit
      ta_pextr_prefix_ok:
	push	edx ebx ecx
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_xmm_register
	mov	[ta_postbyte_register],al
	pop	ecx ebx edx
	jmp	ta_mmx_imm8
      ta_pextr_reg:
	lods	byte [esi]
	call	ta_convert_register
	cmp	[ta_mmx_size],4
	ja	ta_pextrq_reg
	cmp	ah,4
	je	ta_pextr_reg_size_ok
	cmp	[ta_code_type],64
	jne	ta_pextr_invalid_size
	cmp	ah,8
	je	ta_pextr_reg_size_ok
      ta_pextr_invalid_size:
	jmp	ta_invalid_operand_size
      ta_pextrq_reg:
	cmp	ah,8
	jne	ta_pextr_invalid_size
	call	ta_operand_64bit
      ta_pextr_reg_size_ok:
	mov	[ta_operand_size],0
	push	eax
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_mmx_register
	mov	ebx,eax
	pop	eax
	mov	[ta_postbyte_register],al
	mov	al,ah
	cmp	[ta_mmx_size],2
	jne	ta_pextr_reg_store
	mov	[ta_opcode_prefix],0
	mov	[ta_extended_code],0C5h
	call	ta_make_mmx_prefix
	jmp	ta_mmx_nomem_imm8
      ta_pextr_reg_store:
	cmp	bh,16
	jne	ta_invalid_operand_size
	xchg	bl,[ta_postbyte_register]
	jmp	ta_mmx_nomem_imm8
ta_pinsrb_instruction:
	mov	[ta_mmx_size],1
	jmp	ta_pinsr_instruction
ta_pinsrd_instruction:
	mov	[ta_mmx_size],4
	jmp	ta_pinsr_instruction
ta_pinsrq_instruction:
	mov	[ta_mmx_size],8
	call	ta_operand_64bit
      ta_pinsr_instruction:
	call	ta_setup_66_0f_3a
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_xmm_register
	mov	[ta_postbyte_register],al
      ta_pinsr_xmmreg:
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_pinsr_xmmreg_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	cmp	[ta_operand_size],0
	je	ta_mmx_imm8
	mov	al,[ta_mmx_size]
	cmp	al,[ta_operand_size]
	je	ta_mmx_imm8
	jmp	ta_invalid_operand_size
      ta_pinsr_xmmreg_reg:
	lods	byte [esi]
	call	ta_convert_register
	mov	bl,al
	cmp	[ta_mmx_size],8
	je	ta_pinsrq_xmmreg_reg
	cmp	ah,4
	je	ta_mmx_nomem_imm8
	jmp	ta_invalid_operand_size
      ta_pinsrq_xmmreg_reg:
	cmp	ah,8
	je	ta_mmx_nomem_imm8
	jmp	ta_invalid_operand_size
ta_pmovsxbw_instruction:
	mov	[ta_mmx_size],8
	jmp	ta_pmovsx_instruction
ta_pmovsxbd_instruction:
	mov	[ta_mmx_size],4
	jmp	ta_pmovsx_instruction
ta_pmovsxbq_instruction:
	mov	[ta_mmx_size],2
	jmp	ta_pmovsx_instruction
ta_pmovsxwd_instruction:
	mov	[ta_mmx_size],8
	jmp	ta_pmovsx_instruction
ta_pmovsxwq_instruction:
	mov	[ta_mmx_size],4
	jmp	ta_pmovsx_instruction
ta_pmovsxdq_instruction:
	mov	[ta_mmx_size],8
      ta_pmovsx_instruction:
	call	ta_setup_66_0f_38
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_xmm_register
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	mov	[ta_operand_size],0
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_pmovsx_xmmreg_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	cmp	[ta_operand_size],0
	je	ta_instruction_ready
	mov	al,[ta_mmx_size]
	cmp	al,[ta_operand_size]
	jne	ta_invalid_operand_size
	jmp	ta_instruction_ready
      ta_pmovsx_xmmreg_reg:
	lods	byte [esi]
	call	ta_convert_xmm_register
	mov	bl,al
	jmp	ta_nomem_instruction_ready
      ta_setup_66_0f_38:
	mov	[ta_extended_code],38h
	mov	[ta_supplemental_code],al
	mov	[ta_base_code],0Fh
	mov	[ta_opcode_prefix],66h
	ret

ta_xsaves_instruction_64bit:
	call	ta_operand_64bit
ta_xsaves_instruction:
	mov	ah,0C7h
	jmp	ta_xsave_common
ta_fxsave_instruction_64bit:
	call	ta_operand_64bit
ta_fxsave_instruction:
	mov	ah,0AEh
	xor	cl,cl
      ta_xsave_common:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],ah
	mov	[ta_postbyte_register],al
	mov	[ta_mmx_size],cl
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	xor	ah,ah
	xchg	ah,[ta_operand_size]
	or	ah,ah
	jz	ta_xsave_size_ok
	cmp	ah,[ta_mmx_size]
	jne	ta_invalid_operand_size
      ta_xsave_size_ok:
	jmp	ta_instruction_ready
ta_clflush_instruction:
	mov	ah,0AEh
	mov	cl,1
	jmp	ta_xsave_common
ta_cldemote_instruction:
	mov	ah,1Ch
	mov	cl,1
	jmp	ta_xsave_common
ta_stmxcsr_instruction:
	mov	ah,0AEh
	mov	cl,4
	jmp	ta_xsave_common
ta_prefetch_instruction:
	mov	[ta_extended_code],18h
      ta_prefetch_mem_8bit:
	mov	[ta_base_code],0Fh
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	or	ah,ah
	jz	ta_prefetch_size_ok
	cmp	ah,1
	jne	ta_invalid_operand_size
      ta_prefetch_size_ok:
	call	ta_get_address
	jmp	ta_instruction_ready
ta_amd_prefetch_instruction:
	mov	[ta_extended_code],0Dh
	jmp	ta_prefetch_mem_8bit
ta_clflushopt_instruction:
	mov	[ta_extended_code],0AEh
	mov	[ta_opcode_prefix],66h
	jmp	ta_prefetch_mem_8bit
ta_pcommit_instruction:
	mov	byte [edi],66h
	inc	edi
ta_fence_instruction:
	mov	bl,al
	mov	ax,0AE0Fh
	stos	word [edi]
	mov	al,bl
	stos	byte [edi]
	jmp	ta_instruction_assembled
ta_pause_instruction:
	mov	ax,90F3h
	stos	word [edi]
	jmp	ta_instruction_assembled
ta_movntq_instruction:
	mov	[ta_mmx_size],8
	jmp	ta_movnt_instruction
ta_movntpd_instruction:
	mov	[ta_opcode_prefix],66h
ta_movntps_instruction:
	mov	[ta_mmx_size],16
      ta_movnt_instruction:
	mov	[ta_extended_code],al
	mov	[ta_base_code],0Fh
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_mmx_register
	cmp	ah,[ta_mmx_size]
	jne	ta_invalid_operand_size
	mov	[ta_postbyte_register],al
	jmp	ta_instruction_ready

ta_movntsd_instruction:
	mov	[ta_opcode_prefix],0F2h
	mov	[ta_mmx_size],8
	jmp	ta_movnts_instruction
ta_movntss_instruction:
	mov	[ta_opcode_prefix],0F3h
	mov	[ta_mmx_size],4
      ta_movnts_instruction:
	mov	[ta_extended_code],al
	mov	[ta_base_code],0Fh
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	mov	al,[ta_operand_size]
	cmp	al,[ta_mmx_size]
	je	ta_movnts_size_ok
	test	al,al
	jnz	ta_invalid_operand_size
      ta_movnts_size_ok:
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	mov	[ta_operand_size],0
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_xmm_register
	mov	[ta_postbyte_register],al
	jmp	ta_instruction_ready

ta_movdiri_instruction:
	mov	[ta_supplemental_code],al
	mov	al,38h
ta_movnti_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_register
	cmp	ah,4
	je	ta_movnti_store
	cmp	ah,8
	jne	ta_invalid_operand_size
	call	ta_operand_64bit
      ta_movnti_store:
	mov	[ta_postbyte_register],al
	jmp	ta_instruction_ready
ta_monitor_instruction:
	mov	[ta_postbyte_register],al
	cmp	byte [esi],0
	je	ta_monitor_instruction_store
	cmp	byte [esi],0Fh
	je	ta_monitor_instruction_store
	call	ta_take_register
	cmp	ax,0400h
	jne	ta_invalid_operand
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_register
	cmp	ax,0401h
	jne	ta_invalid_operand
	cmp	[ta_postbyte_register],0C8h
	jne	ta_monitor_instruction_store
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_register
	cmp	ax,0402h
	jne	ta_invalid_operand
      ta_monitor_instruction_store:
	mov	ax,010Fh
	stos	word [edi]
	mov	al,[ta_postbyte_register]
	stos	byte [edi]
	jmp	ta_instruction_assembled
ta_hreset_instruction:
	call	ta_take_byte_value
	mov	dl,al
	cmp	byte [esi],','
	jne	ta_hreset_instruction_store
	lods	byte [esi]
	call	ta_take_register
	cmp	ax,0400h
	jne	ta_invalid_operand
      ta_hreset_instruction_store:
	mov	eax,0F03A0FF3h
	stos	dword [edi]
	mov	al,0C0h
	mov	ah,dl
	stos	word [edi]
	jmp	ta_instruction_assembled

ta_pconfig_instruction:
	mov	[ta_postbyte_register],al
	jmp	ta_monitor_instruction_store
ta_movntdqa_instruction:
	call	ta_setup_66_0f_38
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_xmm_register
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	jmp	ta_instruction_ready

ta_extrq_instruction:
	mov	[ta_opcode_prefix],66h
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],78h
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_xmm_register
	mov	[ta_postbyte_register],al
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_extrq_xmmreg_xmmreg
	test	ah,not 1
	jnz	ta_invalid_operand_size
	cmp	al,'('
	jne	ta_invalid_operand
	xor	bl,bl
	xchg	bl,[ta_postbyte_register]
	call	ta_store_nomem_instruction
	call	ta_get_byte_value
	stosb
	call	ta_append_imm8
	jmp	ta_instruction_assembled
      ta_extrq_xmmreg_xmmreg:
	inc	[ta_extended_code]
	lods	byte [esi]
	call	ta_convert_xmm_register
	mov	bl,al
	jmp	ta_nomem_instruction_ready
ta_insertq_instruction:
	mov	[ta_opcode_prefix],0F2h
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],78h
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_xmm_register
	mov	[ta_postbyte_register],al
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_xmm_register
	mov	bl,al
	cmp	byte [esi],','
	je	ta_insertq_with_imm
	inc	[ta_extended_code]
	jmp	ta_nomem_instruction_ready
      ta_insertq_with_imm:
	call	ta_store_nomem_instruction
	call	ta_append_imm8
	call	ta_append_imm8
	jmp	ta_instruction_assembled

ta_crc32_instruction:
	mov	[ta_opcode_prefix],0F2h
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],38h
	mov	[ta_supplemental_code],0F0h
	call	ta_take_register
	mov	[ta_postbyte_register],al
	cmp	ah,4
	je	ta_crc32_reg_size_ok
	cmp	ah,8
	jne	ta_invalid_operand
	cmp	[ta_code_type],64
	jne	ta_illegal_instruction
      ta_crc32_reg_size_ok:
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	mov	[ta_operand_size],0
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_crc32_reg_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	mov	al,[ta_operand_size]
	test	al,al
	jz	ta_crc32_unknown_size
	cmp	al,1
	je	ta_crc32_reg_mem_store
	inc	[ta_supplemental_code]
	call	ta_operand_autodetect
      ta_crc32_reg_mem_store:
	jmp	ta_instruction_ready
      ta_crc32_unknown_size:
	call	ta_recoverable_unknown_size
	jmp	ta_crc32_reg_mem_store
      ta_crc32_reg_reg:
	lods	byte [esi]
	call	ta_convert_register
	mov	bl,al
	mov	al,ah
	cmp	al,1
	je	ta_crc32_reg_reg_store
	inc	[ta_supplemental_code]
	call	ta_operand_autodetect
      ta_crc32_reg_reg_store:
	jmp	ta_nomem_instruction_ready
ta_popcnt_instruction:
	mov	[ta_opcode_prefix],0F3h
	jmp	ta_bs_instruction
ta_movbe_instruction:
	mov	[ta_supplemental_code],al
	mov	[ta_extended_code],38h
	mov	[ta_base_code],0Fh
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	je	ta_movbe_mem
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_register
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_argument
	call	ta_get_address
	mov	al,[ta_operand_size]
	call	ta_operand_autodetect
	jmp	ta_instruction_ready
      ta_movbe_mem:
	inc	[ta_supplemental_code]
	call	ta_get_address
	push	edx ebx ecx
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_register
	mov	[ta_postbyte_register],al
	pop	ecx ebx edx
	mov	al,[ta_operand_size]
	call	ta_operand_autodetect
	jmp	ta_instruction_ready
ta_adx_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],38h
	mov	[ta_supplemental_code],0F6h
	mov	[ta_operand_prefix],al
	call	ta_get_reg_mem
	jc	ta_adx_reg_reg
	mov	al,[ta_operand_size]
	cmp	al,4
	je	ta_instruction_ready
	cmp	al,8
	jne	ta_invalid_operand_size
	call	ta_operand_64bit
	jmp	ta_instruction_ready
      ta_adx_reg_reg:
	cmp	ah,4
	je	ta_nomem_instruction_ready
	cmp	ah,8
	jne	ta_invalid_operand_size
	call	ta_operand_64bit
	jmp	ta_nomem_instruction_ready
ta_rdpid_instruction:
	mov	[ta_postbyte_register],al
	mov	[ta_extended_code],0C7h
	mov	[ta_base_code],0Fh
	mov	[ta_opcode_prefix],0F3h
	call	ta_take_register
	mov	bl,al
	cmp	[ta_code_type],64
	je	ta_rdpid_64bit
	cmp	ah,4
	jne	ta_invalid_operand_size
	jmp	ta_nomem_instruction_ready
      ta_rdpid_64bit:
	cmp	ah,8
	jne	ta_invalid_operand_size
	jmp	ta_nomem_instruction_ready
ta_ptwrite_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],0AEh
	mov	[ta_postbyte_register],al
	mov	[ta_opcode_prefix],0F3h
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_ptwrite_reg
      ta_ptwrite_mem:
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	mov	al,[ta_operand_size]
	cmp	al,4
	je	ta_ptwrite_mem_store
	cmp	al,8
	je	ta_ptwrite_mem_64bit
	or	al,al
	jnz	ta_invalid_operand_size
	call	ta_recoverable_unknown_size
	jmp	ta_ptwrite_mem_store
      ta_ptwrite_mem_64bit:
	call	ta_operand_64bit
      ta_ptwrite_mem_store:
	mov	al,[ta_operand_size]
	call	ta_operand_autodetect
	jmp	ta_instruction_ready
      ta_ptwrite_reg:
	lods	byte [esi]
	call	ta_convert_register
	mov	bl,al
	mov	al,ah
	cmp	al,4
	je	ta_nomem_instruction_ready
	cmp	al,8
	jne	ta_invalid_operand_size
	call	ta_operand_64bit
	jmp	ta_nomem_instruction_ready

ta_vmclear_instruction:
	mov	[ta_opcode_prefix],66h
	jmp	ta_vmx_instruction
ta_vmxon_instruction:
	mov	[ta_opcode_prefix],0F3h
ta_vmx_instruction:
	mov	[ta_postbyte_register],al
	mov	[ta_extended_code],0C7h
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	mov	al,[ta_operand_size]
	or	al,al
	jz	ta_vmx_size_ok
	cmp	al,8
	jne	ta_invalid_operand_size
      ta_vmx_size_ok:
	mov	[ta_base_code],0Fh
	jmp	ta_instruction_ready
ta_vmread_instruction:
	mov	[ta_extended_code],78h
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_vmread_nomem
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_register
	mov	[ta_postbyte_register],al
	call	ta_vmread_check_size
	jmp	ta_vmx_size_ok
      ta_vmread_nomem:
	lods	byte [esi]
	call	ta_convert_register
	push	eax
	call	ta_vmread_check_size
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_register
	mov	[ta_postbyte_register],al
	call	ta_vmread_check_size
	pop	ebx
	mov	[ta_base_code],0Fh
	jmp	ta_nomem_instruction_ready
      ta_vmread_check_size:
	cmp	[ta_code_type],64
	je	ta_vmread_long
	cmp	[ta_operand_size],4
	jne	ta_invalid_operand_size
	ret
      ta_vmread_long:
	cmp	[ta_operand_size],8
	jne	ta_invalid_operand_size
	ret
ta_vmwrite_instruction:
	mov	[ta_extended_code],79h
	call	ta_take_register
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_vmwrite_nomem
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	call	ta_vmread_check_size
	jmp	ta_vmx_size_ok
      ta_vmwrite_nomem:
	lods	byte [esi]
	call	ta_convert_register
	mov	bl,al
	mov	[ta_base_code],0Fh
	jmp	ta_nomem_instruction_ready
ta_vmx_inv_instruction:
	call	ta_setup_66_0f_38
	call	ta_take_register
	mov	[ta_postbyte_register],al
	call	ta_vmread_check_size
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	mov	al,[ta_operand_size]
	or	al,al
	jz	ta_vmx_size_ok
	cmp	al,16
	jne	ta_invalid_operand_size
	jmp	ta_vmx_size_ok
ta_simple_svm_instruction:
	push	eax
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],1
	call	ta_take_register
	or	al,al
	jnz	ta_invalid_operand
      ta_simple_svm_detect_size:
	cmp	ah,2
	je	ta_simple_svm_16bit
	cmp	ah,4
	je	ta_simple_svm_32bit
	cmp	[ta_code_type],64
	jne	ta_invalid_operand_size
	jmp	ta_simple_svm_store
      ta_simple_svm_16bit:
	cmp	[ta_code_type],16
	je	ta_simple_svm_store
	cmp	[ta_code_type],64
	je	ta_invalid_operand_size
	jmp	ta_prefixed_svm_store
      ta_simple_svm_32bit:
	cmp	[ta_code_type],32
	je	ta_simple_svm_store
      ta_prefixed_svm_store:
	mov	al,67h
	stos	byte [edi]
      ta_simple_svm_store:
	call	ta_store_classic_instruction_code
	pop	eax
	stos	byte [edi]
	jmp	ta_instruction_assembled
ta_skinit_instruction:
	call	ta_take_register
	cmp	ax,0400h
	jne	ta_invalid_operand
	mov	al,0DEh
	jmp	ta_simple_instruction_0f_01
ta_clzero_instruction:
	call	ta_take_register
	or	al,al
	jnz	ta_invalid_operand
	mov	al,0FCh
	cmp	[ta_code_type],64
	je	ta_clzero_64bit
	cmp	ah,4
	jne	ta_invalid_operand
	jmp	ta_simple_instruction_0f_01
      ta_clzero_64bit:
	cmp	ah,8
	jne	ta_invalid_operand
	jmp	ta_simple_instruction_0f_01
ta_invlpga_instruction:
	push	eax
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],1
	call	ta_take_register
	or	al,al
	jnz	ta_invalid_operand
	mov	bl,ah
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_register
	cmp	ax,0401h
	jne	ta_invalid_operand
	mov	ah,bl
	jmp	ta_simple_svm_detect_size

ta_senduipi_instruction:
	cmp	[ta_code_type],64
	jne	ta_illegal_instruction
	mov	[ta_opcode_prefix],0F3h
ta_rdrand_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],0C7h
	mov	[ta_postbyte_register],al
	call	ta_take_register
	mov	bl,al
	cmp	[ta_opcode_prefix],0F3h
	je	ta_senduipi_reg64
	mov	al,ah
	call	ta_operand_autodetect
	jmp	ta_nomem_instruction_ready
      ta_senduipi_reg64:
	cmp	ah,8
	jne	ta_invalid_operand_size
	jmp	ta_nomem_instruction_ready
ta_rdfsbase_instruction:
	cmp	[ta_code_type],64
	jne	ta_illegal_instruction
	mov	[ta_opcode_prefix],0F3h
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],0AEh
	mov	[ta_postbyte_register],al
	call	ta_take_register
	mov	bl,al
	mov	al,ah
	cmp	ah,2
	je	ta_invalid_operand_size
	call	ta_operand_autodetect
	jmp	ta_nomem_instruction_ready

ta_xabort_instruction:
	call	ta_take_byte_value
	mov	dl,al
	mov	ax,0F8C6h
	stos	word [edi]
	mov	al,dl
	stos	byte [edi]
	jmp	ta_instruction_assembled
ta_xbegin_instruction:
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_operand
	mov	al,[ta_code_type]
	cmp	al,64
	je	ta_xbegin_64bit
	cmp	al,32
	je	ta_xbegin_32bit
      ta_xbegin_16bit:
	call	ta_get_address_word_value
	add	edi,4
	mov	ebp,[ta_addressing_space]
	call	ta_calculate_relative_offset
	sub	edi,4
	shl	eax,16
	mov	ax,0F8C7h
	stos	dword [edi]
	jmp	ta_instruction_assembled
      ta_xbegin_32bit:
	call	ta_get_address_dword_value
	jmp	ta_xbegin_address_ok
      ta_xbegin_64bit:
	call	ta_get_address_qword_value
      ta_xbegin_address_ok:
	add	edi,5
	mov	ebp,[ta_addressing_space]
	call	ta_calculate_relative_offset
	sub	edi,5
	mov	edx,eax
	cwde
	cmp	eax,edx
	jne	ta_xbegin_rel32
	mov	al,66h
	stos	byte [edi]
	mov	eax,edx
	shl	eax,16
	mov	ax,0F8C7h
	stos	dword [edi]
	jmp	ta_instruction_assembled
      ta_xbegin_rel32:
	sub	edx,1
	jno	ta_xbegin_rel32_ok
	cmp	[ta_code_type],64
	je	ta_jump_out_of_range
      ta_xbegin_rel32_ok:
	mov	ax,0F8C7h
	stos	word [edi]
	mov	eax,edx
	stos	dword [edi]
	jmp	ta_instruction_assembled

    ta_get_bnd_size:
	mov	al,4
	cmp	[ta_code_type],64
	jne	ta_bnd_size_ok
	add	al,4
      ta_bnd_size_ok:
	mov	[ta_address_size],al
	ret
    ta_get_address_component:
	mov	[ta_free_address_range],0
	call	ta_calculate_address
	mov	[ta_address_high],edx
	mov	edx,eax
	or	bx,bx
	jz	ta_address_component_ok
	mov	al,bl
	or	al,bh
	shr	al,4
	or	al,[ta_address_size]
	cmp	al,4
	je	ta_address_component_ok
	cmp	al,8
	jne	ta_invalid_address
      ta_address_component_ok:
	ret
ta_bndldx_instruction:
     ta_get_sib_address_components:
	lods	byte [esi]
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address_prefixes
	call	ta_get_address_component
	cmp	byte [esi-1],']'
	je	ta_bnd_mib_ok
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_operand
	mov	al,[ta_address_sign]
	push	eax ebx ecx edx
	push	[ta_address_symbol]
	call	ta_get_address_component
	lods	byte [esi]
	cmp	al,']'
	jne	ta_invalid_operand
	or	dl,bl
	or	dl,[ta_address_sign]
	or	edx,[ta_address_high]
	jnz	ta_invalid_address
	mov	[ta_address_register],bh
	pop	[ta_address_symbol]
	pop	edx ecx ebx eax
	mov	[ta_address_sign],al
	or	bl,bl
	jz	ta_mib_place_index
	or	bh,bh
	jnz	ta_invalid_address
	mov	bh,bl
      ta_mib_place_index:
	mov	bl,[ta_address_register]
	xor	cl,cl
	or	bl,bl
	jz	ta_bnd_mib_ok
	inc	cl
      ta_bnd_mib_ok:
	ret

ta_tpause_instruction:
	mov	[ta_postbyte_register],6
	mov	[ta_extended_code],0AEh
	mov	[ta_base_code],0Fh
	mov	[ta_opcode_prefix],al
	call	ta_take_register
	cmp	ah,4
	jne	ta_invalid_operand_size
	mov	bl,al
	cmp	byte [esi],','
	jne	ta_nomem_instruction_ready
	inc	esi
	call	ta_take_register
	cmp	ax,0402h
	jne	ta_invalid_operand
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_register
	cmp	ax,0400h
	jne	ta_invalid_operand
	jmp	ta_nomem_instruction_ready
ta_umonitor_instruction:
	mov	[ta_postbyte_register],6
	mov	[ta_extended_code],0AEh
	mov	[ta_base_code],0Fh
	mov	[ta_opcode_prefix],0F3h
	call	ta_take_register
	mov	bl,al
	cmp	ah,4
	je	ta_umonitor_reg32
	shl	ah,3
	cmp	ah,[ta_code_type]
	je	ta_umonitor_ok
	jmp	ta_invalid_operand_size
      ta_umonitor_reg32:
	call	ta_address_32bit_prefix
      ta_umonitor_ok:
	jmp	ta_nomem_instruction_ready
ta_movdir64b_instruction:
	call	ta_setup_66_0f_38
      ta_movdir64b_reg_mem:
	call	ta_take_register
	mov	[ta_postbyte_register],al
	xor	al,al
	xchg	al,[ta_operand_size]
	push	eax
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	mov	al,[ta_operand_size]
	or	al,al
	jz	ta_movdir64b_ready
	cmp	al,64
	jne	ta_invalid_operand_size
      ta_movdir64b_ready:
	push	edi
	call	ta_store_instruction
	pop	ebx eax
	mov	cl,[ta_code_type]
	cmp	byte [ebx],67h
	jne	ta_movdir64b_size_check
	shr	cl,1
	cmp	cl,16
	jae	ta_movdir64b_size_check
	mov	cl,32
      ta_movdir64b_size_check:
	shl	al,3
	cmp	al,cl
	jne	ta_invalid_operand_size
	jmp	ta_instruction_assembled
ta_enqcmd_instruction:
	mov	[ta_opcode_prefix],al
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],38h
	mov	[ta_supplemental_code],0F8h
	jmp	ta_movdir64b_reg_mem

ta_setssbsy_instruction:
	shl	eax,24
	or	eax,010FF3h
	stos	dword [edi]
	jmp	ta_instruction_assembled
ta_rstorssp_instruction:
	mov	ah,1
	jmp	ta_setup_clrssbsy
ta_clrssbsy_instruction:
	mov	ah,0AEh
      ta_setup_clrssbsy:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],ah
	mov	[ta_postbyte_register],al
	mov	[ta_opcode_prefix],0F3h
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	test	[ta_operand_size],not 8
	jnz	ta_invalid_operand_size
	jmp	ta_instruction_ready
ta_rdsspq_instruction:
	mov	[ta_rex_prefix],48h
ta_rdsspd_instruction:
	mov	ah,1Eh
	jmp	ta_setup_incssp
ta_incsspq_instruction:
	mov	[ta_rex_prefix],48h
ta_incsspd_instruction:
	mov	ah,0AEh
      ta_setup_incssp:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],ah
	mov	[ta_postbyte_register],al
	mov	[ta_opcode_prefix],0F3h
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_register
	mov	bl,al
	call	ta_cet_size_check
	jmp	ta_nomem_instruction_ready
      ta_cet_size_check:
	cmp	[ta_rex_prefix],0
	je	ta_cet_dword
	cmp	[ta_code_type],64
	jne	ta_illegal_instruction
	shr	ah,1
      ta_cet_dword:
	cmp	ah,4
	jne	ta_invalid_operand_size
	ret
ta_wrussq_instruction:
	mov	[ta_opcode_prefix],66h
ta_wrssq_instruction:
	mov	[ta_rex_prefix],48h
	jmp	ta_wrssd_instruction
ta_wrussd_instruction:
	mov	[ta_opcode_prefix],66h
ta_wrssd_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],38h
	mov	[ta_supplemental_code],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_wrss_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	push	edx ebx ecx
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_register
	mov	[ta_postbyte_register],al
	pop	ecx ebx edx
	call	ta_cet_size_check
	jmp	ta_instruction_ready
      ta_wrss_reg:
	lods	byte [esi]
	call	ta_convert_register
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_register
	mov	bl,al
	xchg	bl,[ta_postbyte_register]
	call	ta_cet_size_check
	jmp	ta_nomem_instruction_ready
ta_endbr_instruction:
	shl	eax,24
	or	eax,1E0FF3h
	stos	dword [edi]
	jmp	ta_instruction_assembled

ta_take_register:
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
ta_convert_register:
	mov	ah,al
	shr	ah,4
	and	al,0Fh
	cmp	ah,8
	je	ta_match_register_size
	cmp	ah,4
	ja	ta_invalid_operand
	cmp	ah,1
	ja	ta_match_register_size
	cmp	al,4
	jb	ta_match_register_size
	or	ah,ah
	jz	ta_high_byte_register
	or	[ta_rex_prefix],40h
      ta_match_register_size:
	cmp	ah,[ta_operand_size]
	je	ta_register_size_ok
	cmp	[ta_operand_size],0
	jne	ta_operand_sizes_do_not_match
	mov	[ta_operand_size],ah
      ta_register_size_ok:
	ret
      ta_high_byte_register:
	mov	ah,1
	or	[ta_rex_prefix],10h
	jmp	ta_match_register_size
ta_convert_fpu_register:
	mov	ah,al
	shr	ah,4
	and	al,111b
	cmp	ah,10
	jne	ta_invalid_operand
	jmp	ta_match_register_size
ta_convert_mmx_register:
	mov	ah,al
	shr	ah,4
	cmp	ah,0Ch
	je	ta_xmm_register
	ja	ta_invalid_operand
	and	al,111b
	cmp	ah,0Bh
	jne	ta_invalid_operand
	mov	ah,8
	jmp	ta_match_register_size
      ta_xmm_register:
	and	al,0Fh
	mov	ah,16
	cmp	al,8
	jb	ta_match_register_size
	cmp	[ta_code_type],64
	jne	ta_invalid_operand
	jmp	ta_match_register_size
ta_convert_xmm_register:
	mov	ah,al
	shr	ah,4
	cmp	ah,0Ch
	je	ta_xmm_register
	jmp	ta_invalid_operand
ta_get_size_operator:
	xor	ah,ah
	cmp	al,11h
	jne	ta_no_size_operator
	mov	[ta_size_declared],1
	lods	word [esi]
	xchg	al,ah
	or	[ta_operand_flags],1
	cmp	ah,[ta_operand_size]
	je	ta_size_operator_ok
	cmp	[ta_operand_size],0
	jne	ta_operand_sizes_do_not_match
	mov	[ta_operand_size],ah
      ta_size_operator_ok:
	ret
      ta_no_size_operator:
	mov	[ta_size_declared],0
	cmp	al,'['
	jne	ta_size_operator_ok
	and	[ta_operand_flags],not 1
	ret
ta_get_jump_operator:
	mov	[ta_jump_type],0
	cmp	al,12h
	jne	ta_jump_operator_ok
	lods	word [esi]
	mov	[ta_jump_type],al
	mov	al,ah
      ta_jump_operator_ok:
	ret
ta_get_address:
	and	[ta_address_size],0
    ta_get_address_of_required_size:
	call	ta_get_address_prefixes
	and	[ta_free_address_range],0
	call	ta_calculate_address
	cmp	byte [esi-1],']'
	jne	ta_invalid_address
	mov	[ta_address_high],edx
	mov	edx,eax
	cmp	[ta_address_size_declared],0
	jne	ta_address_ok
	cmp	[ta_segment_register],4
	ja	ta_address_ok
	or	bx,bx
	jnz	ta_clear_address_size
	cmp	[ta_code_type],64
	jne	ta_address_ok
      ta_calculate_relative_address:
	mov	edx,[ta_address_symbol]
	promote_edx
	mov	[ta_symbol_identifier],edx
	mov	edx,[ta_address_high]
	promote_edx
	mov	ebp,[ta_addressing_space]
	call	ta_calculate_relative_offset
	mov	[ta_address_high],edx
	cdq
	cmp	edx,[ta_address_high]
	je	ta_address_high_ok
	call	ta_recoverable_overflow
      ta_address_high_ok:
	mov	edx,eax
	ror	ecx,16
	mov	cl,[ta_value_type]
	rol	ecx,16
	mov	bx,9900h
      ta_clear_address_size:
	and	ch,not 0Fh
      ta_address_ok:
	ret
ta_get_address_prefixes:
	and	[ta_segment_register],0
	and	[ta_address_size_declared],0
	mov	al,[ta_code_type]
	shr	al,3
	mov	[ta_value_size],al
	mov	al,[esi]
	and	al,11110000b
	cmp	al,60h
	jne	ta_get_address_size_prefix
	lods	byte [esi]
	sub	al,60h
	mov	[ta_segment_register],al
	mov	al,[esi]
	and	al,11110000b
      ta_get_address_size_prefix:
	cmp	al,70h
	jne	ta_address_size_prefix_ok
	lods	byte [esi]
	sub	al,70h
	cmp	al,2
	jb	ta_invalid_address_size
	cmp	al,8
	ja	ta_invalid_address_size
	mov	[ta_value_size],al
	or	[ta_address_size_declared],1
	or	[ta_address_size],al
	cmp	al,[ta_address_size]
	jne	ta_invalid_address_size
      ta_address_size_prefix_ok:
	ret
ta_operand_16bit:
	cmp	[ta_code_type],16
	je	ta_size_prefix_ok
	mov	[ta_operand_prefix],66h
	ret
ta_operand_32bit:
	cmp	[ta_code_type],16
	jne	ta_size_prefix_ok
	mov	[ta_operand_prefix],66h
      ta_size_prefix_ok:
	ret
ta_operand_64bit:
	cmp	[ta_code_type],64
	jne	ta_illegal_instruction
	or	[ta_rex_prefix],48h
	ret
ta_operand_autodetect:
	cmp	al,2
	je	ta_operand_16bit
	cmp	al,4
	je	ta_operand_32bit
	cmp	al,8
	je	ta_operand_64bit
	jmp	ta_invalid_operand_size
ta_store_segment_prefix_if_necessary:
	mov	al,[ta_segment_register]
	or	al,al
	jz	ta_segment_prefix_ok
	cmp	al,4
	ja	ta_segment_prefix_386
	cmp	[ta_code_type],64
	je	ta_segment_prefix_ok
	cmp	al,3
	je	ta_ss_prefix
	jb	ta_segment_prefix_86
	cmp	bl,25h
	je	ta_segment_prefix_86
	cmp	bh,25h
	je	ta_segment_prefix_86
	cmp	bh,45h
	je	ta_segment_prefix_86
	cmp	bh,44h
	je	ta_segment_prefix_86
	ret
      ta_ss_prefix:
	cmp	bl,25h
	je	ta_segment_prefix_ok
	cmp	bh,25h
	je	ta_segment_prefix_ok
	cmp	bh,45h
	je	ta_segment_prefix_ok
	cmp	bh,44h
	je	ta_segment_prefix_ok
	jmp	ta_segment_prefix_86
ta_store_segment_prefix:
	mov	al,[ta_segment_register]
	or	al,al
	jz	ta_segment_prefix_ok
	cmp	al,5
	jae	ta_segment_prefix_386
      ta_segment_prefix_86:
	dec	al
	shl	al,3
	add	al,26h
	stos	byte [edi]
	jmp	ta_segment_prefix_ok
      ta_segment_prefix_386:
	add	al,64h-5
	stos	byte [edi]
      ta_segment_prefix_ok:
	ret
ta_store_instruction_code:
	cmp	[ta_vex_required],0
	jne	ta_store_vex_instruction_code
ta_store_classic_instruction_code:
	mov	al,[ta_operand_prefix]
	or	al,al
	jz	ta_operand_prefix_ok
	stos	byte [edi]
      ta_operand_prefix_ok:
	mov	al,[ta_opcode_prefix]
	or	al,al
	jz	ta_opcode_prefix_ok
	stos	byte [edi]
      ta_opcode_prefix_ok:
	mov	al,[ta_rex_prefix]
	test	al,40h
	jz	ta_rex_prefix_ok
	cmp	[ta_code_type],64
	jne	ta_invalid_operand
	test	al,0B0h
	jnz	ta_disallowed_combination_of_registers
	stos	byte [edi]
      ta_rex_prefix_ok:
	mov	al,[ta_base_code]
	stos	byte [edi]
	cmp	al,0Fh
	jne	ta_instruction_code_ok
      ta_store_extended_code:
	mov	al,[ta_extended_code]
	stos	byte [edi]
	cmp	al,38h
	je	ta_store_supplemental_code
	cmp	al,3Ah
	je	ta_store_supplemental_code
      ta_instruction_code_ok:
	ret
      ta_store_supplemental_code:
	mov	al,[ta_supplemental_code]
	stos	byte [edi]
	ret
ta_store_nomem_instruction:
	test	[ta_postbyte_register],10000b
	jz	ta_nomem_reg_high_code_ok
	or	[ta_vex_required],10h
	and	[ta_postbyte_register],1111b
      ta_nomem_reg_high_code_ok:
	test	[ta_postbyte_register],1000b
	jz	ta_nomem_reg_code_ok
	or	[ta_rex_prefix],44h
	and	[ta_postbyte_register],111b
      ta_nomem_reg_code_ok:
	test	bl,10000b
	jz	ta_nomem_rm_high_code_ok
	or	[ta_rex_prefix],42h
	or	[ta_vex_required],8
	and	bl,1111b
      ta_nomem_rm_high_code_ok:
	test	bl,1000b
	jz	ta_nomem_rm_code_ok
	or	[ta_rex_prefix],41h
	and	bl,111b
      ta_nomem_rm_code_ok:
	and	[ta_displacement_compression],0
	call	ta_store_instruction_code
	mov	al,[ta_postbyte_register]
	shl	al,3
	or	al,bl
	or	al,11000000b
	stos	byte [edi]
	ret
ta_store_instruction:
	mov	[ta_current_offset],edi
	and	[ta_displacement_compression],0
	test	[ta_postbyte_register],10000b
	jz	ta_reg_high_code_ok
	or	[ta_vex_required],10h
	and	[ta_postbyte_register],1111b
      ta_reg_high_code_ok:
	test	[ta_postbyte_register],1000b
	jz	ta_reg_code_ok
	or	[ta_rex_prefix],44h
	and	[ta_postbyte_register],111b
      ta_reg_code_ok:
	cmp	[ta_code_type],64
	jne	ta_address_value_ok
	xor	eax,eax
	bt	edx,31
	sbb	eax,[ta_address_high]
	jz	ta_address_value_ok
	cmp	[ta_address_high],0
	jne	ta_address_value_out_of_range
	test	ch,44h
	jnz	ta_address_value_ok
	test	bx,8080h
	jz	ta_address_value_ok
      ta_address_value_out_of_range:
	call	ta_recoverable_overflow
      ta_address_value_ok:
	call	ta_store_segment_prefix_if_necessary
	test	[ta_vex_required],4
	jnz	ta_address_vsib
	or	bx,bx
	jz	ta_address_immediate
	cmp	bx,9800h
	je	ta_address_rip_based
	cmp	bx,9400h
	je	ta_address_eip_based
	cmp	bx,9900h
	je	ta_address_relative
	mov	al,bl
	or	al,bh
	and	al,11110000b
	cmp	al,80h
	je	ta_postbyte_64bit
	cmp	al,40h
	je	ta_postbyte_32bit
	cmp	al,20h
	jne	ta_invalid_address
	cmp	[ta_code_type],64
	je	ta_invalid_address_size
	test	[ta_operand_flags],80h
	jnz	ta_invalid_address_size
	call	ta_address_16bit_prefix
	test	ch,22h
	setz	[ta_displacement_compression]
	call	ta_store_instruction_code
	cmp	bl,bh
	jbe	ta_determine_16bit_address
	xchg	bl,bh
      ta_determine_16bit_address:
	cmp	bx,2600h
	je	ta_address_si
	cmp	bx,2700h
	je	ta_address_di
	cmp	bx,2300h
	je	ta_address_bx
	cmp	bx,2500h
	je	ta_address_bp
	cmp	bx,2625h
	je	ta_address_bp_si
	cmp	bx,2725h
	je	ta_address_bp_di
	cmp	bx,2723h
	je	ta_address_bx_di
	cmp	bx,2623h
	jne	ta_invalid_address
      ta_address_bx_si:
	xor	al,al
	jmp	ta_postbyte_16bit
      ta_address_bx_di:
	mov	al,1
	jmp	ta_postbyte_16bit
      ta_address_bp_si:
	mov	al,10b
	jmp	ta_postbyte_16bit
      ta_address_bp_di:
	mov	al,11b
	jmp	ta_postbyte_16bit
      ta_address_si:
	mov	al,100b
	jmp	ta_postbyte_16bit
      ta_address_di:
	mov	al,101b
	jmp	ta_postbyte_16bit
      ta_address_bx:
	mov	al,111b
	jmp	ta_postbyte_16bit
      ta_address_bp:
	mov	al,110b
      ta_postbyte_16bit:
	test	ch,22h
	jnz	ta_address_16bit_value
	or	ch,ch
	jnz	ta_address_sizes_do_not_agree
	cmp	edx,10000h
	jge	ta_value_out_of_range
	cmp	edx,-8000h
	jl	ta_value_out_of_range
	or	dx,dx
	jz	ta_address
	cmp	[ta_displacement_compression],2
	ja	ta_address_8bit_value
	je	ta_address_16bit_value
	cmp	dx,80h
	jb	ta_address_8bit_value
	cmp	dx,-80h
	jae	ta_address_8bit_value
      ta_address_16bit_value:
	or	al,10000000b
	mov	cl,[ta_postbyte_register]
	shl	cl,3
	or	al,cl
	stos	byte [edi]
	mov	eax,edx
	stos	word [edi]
	ret
      ta_address_8bit_value:
	or	al,01000000b
	mov	cl,[ta_postbyte_register]
	shl	cl,3
	or	al,cl
	stos	byte [edi]
	mov	al,dl
	stos	byte [edi]
	ret
      ta_address:
	cmp	al,110b
	je	ta_address_8bit_value
	mov	cl,[ta_postbyte_register]
	shl	cl,3
	or	al,cl
	stos	byte [edi]
	ret
      ta_address_vsib:
	mov	al,bl
	shr	al,4
	test	al,1
	jz	ta_vsib_high_code_ok
	or	[ta_vex_register],10000b
	or	[ta_vex_required],8
	xor	al,1
      ta_vsib_high_code_ok:
	cmp	al,6
	je	ta_vsib_index_ok
	cmp	al,0Ch
	jb	ta_invalid_address
      ta_vsib_index_ok:
	mov	al,bh
	shr	al,4
	cmp	al,4
	je	ta_postbyte_32bit
	cmp	[ta_code_type],64
	je	ta_address_prefix_ok
	test	al,al
	jnz	ta_invalid_address
      ta_postbyte_32bit:
	call	ta_address_32bit_prefix
	jmp	ta_address_prefix_ok
      ta_postbyte_64bit:
	cmp	[ta_code_type],64
	jne	ta_invalid_address_size
      ta_address_prefix_ok:
	cmp	bl,44h
	je	ta_invalid_address
	cmp	bl,84h
	je	ta_invalid_address
	test	bh,1000b
	jz	ta_base_code_ok
	or	[ta_rex_prefix],41h
      ta_base_code_ok:
	test	bl,1000b
	jz	ta_index_code_ok
	or	[ta_rex_prefix],42h
      ta_index_code_ok:
	test	ch,44h or 88h
	setz	[ta_displacement_compression]
	call	ta_store_instruction_code
	or	cl,cl
	jz	ta_only_base_register
      ta_base_and_index:
	mov	al,100b
	xor	ah,ah
	cmp	cl,1
	je	ta_scale_ok
	cmp	cl,2
	je	ta_scale_1
	cmp	cl,4
	je	ta_scale_2
	or	ah,11000000b
	jmp	ta_scale_ok
      ta_scale_2:
	or	ah,10000000b
	jmp	ta_scale_ok
      ta_scale_1:
	or	ah,01000000b
      ta_scale_ok:
	or	bh,bh
	jz	ta_only_index_register
	and	bl,111b
	shl	bl,3
	or	ah,bl
	and	bh,111b
	or	ah,bh
      ta_sib_ready:
	test	ch,44h or 88h
	jnz	ta_sib_address_32bit_value
	or	ch,ch
	jnz	ta_address_sizes_do_not_agree
	cmp	bh,5
	je	ta_address_value
	or	edx,edx
	jz	ta_sib_address
      ta_address_value:
	cmp	[ta_displacement_compression],2
	ja	ta_sib_address_8bit_value
	je	ta_sib_address_32bit_value
	cmp	edx,80h
	jb	ta_sib_address_8bit_value
	cmp	edx,-80h
	jnb	ta_sib_address_8bit_value
      ta_sib_address_32bit_value:
	or	al,10000000b
	mov	cl,[ta_postbyte_register]
	shl	cl,3
	or	al,cl
	stos	word [edi]
	jmp	ta_store_address_32bit_value
      ta_sib_address_8bit_value:
	or	al,01000000b
	mov	cl,[ta_postbyte_register]
	shl	cl,3
	or	al,cl
	stos	word [edi]
	mov	al,dl
	stos	byte [edi]
	ret
      ta_sib_address:
	mov	cl,[ta_postbyte_register]
	shl	cl,3
	or	al,cl
	stos	word [edi]
	ret
      ta_only_index_register:
	or	ah,101b
	and	bl,111b
	shl	bl,3
	or	ah,bl
	mov	cl,[ta_postbyte_register]
	shl	cl,3
	or	al,cl
	stos	word [edi]
	test	ch,44h or 88h
	jnz	ta_store_address_32bit_value
	or	ch,ch
	jnz	ta_invalid_address_size
	cmp	[ta_displacement_compression],2
	jbe	ta_store_address_32bit_value
	mov	edx,[ta_uncompressed_displacement]
	promote_edx
	jmp	ta_store_address_32bit_value
      ta_zero_index_register:
	mov	bl,4
	mov	cl,1
	jmp	ta_base_and_index
      ta_only_base_register:
	mov	al,bh
	and	al,111b
	cmp	al,4
	je	ta_zero_index_register
	test	[ta_operand_flags],80h
	jnz	ta_zero_index_register
	test	ch,44h or 88h
	jnz	ta_simple_address_32bit_value
	or	ch,ch
	jnz	ta_address_sizes_do_not_agree
	or	edx,edx
	jz	ta_simple_address
	cmp	[ta_displacement_compression],2
	ja	ta_simple_address_8bit_value
	je	ta_simple_address_32bit_value
	cmp	edx,80h
	jb	ta_simple_address_8bit_value
	cmp	edx,-80h
	jnb	ta_simple_address_8bit_value
      ta_simple_address_32bit_value:
	or	al,10000000b
	mov	cl,[ta_postbyte_register]
	shl	cl,3
	or	al,cl
	stos	byte [edi]
	jmp	ta_store_address_32bit_value
      ta_simple_address_8bit_value:
	or	al,01000000b
	mov	cl,[ta_postbyte_register]
	shl	cl,3
	or	al,cl
	stos	byte [edi]
	mov	al,dl
	stos	byte [edi]
	ret
      ta_simple_address:
	cmp	al,5
	je	ta_simple_address_8bit_value
	mov	cl,[ta_postbyte_register]
	shl	cl,3
	or	al,cl
	stos	byte [edi]
	ret
      ta_address_immediate:
	cmp	[ta_code_type],64
	je	ta_address_immediate_sib
	test	ch,44h or 88h
	jnz	ta_address_immediate_32bit
	test	ch,22h
	jnz	ta_address_immediate_16bit
	or	ch,ch
	jnz	ta_invalid_address_size
	cmp	[ta_code_type],16
	je	ta_addressing_16bit
      ta_address_immediate_32bit:
	call	ta_address_32bit_prefix
	call	ta_store_instruction_code
      ta_store_immediate_address:
	mov	al,101b
	mov	cl,[ta_postbyte_register]
	shl	cl,3
	or	al,cl
	stos	byte [edi]
      ta_store_address_32bit_value:
	test	ch,0F0h
	jz	ta_address_32bit_relocation_ok
	mov	eax,ecx
	shr	eax,16
	cmp	al,4
	jne	ta_address_32bit_relocation
	mov	al,2
      ta_address_32bit_relocation:
	xchg	[ta_value_type],al
	mov	ebx,[ta_address_symbol]
	promote_ebx
	xchg	ebx,[ta_symbol_identifier]
	call	ta_mark_relocation
	mov	[ta_value_type],al
	mov	[ta_symbol_identifier],ebx
      ta_address_32bit_relocation_ok:
	mov	eax,edx
	stos	dword [edi]
	ret
      ta_store_address_64bit_value:
	test	ch,0F0h
	jz	ta_address_64bit_relocation_ok
	mov	eax,ecx
	shr	eax,16
	xchg	[ta_value_type],al
	mov	ebx,[ta_address_symbol]
	promote_ebx
	xchg	ebx,[ta_symbol_identifier]
	call	ta_mark_relocation
	mov	[ta_value_type],al
	mov	[ta_symbol_identifier],ebx
      ta_address_64bit_relocation_ok:
	mov	eax,edx
	stos	dword [edi]
	mov	eax,[ta_address_high]
	stos	dword [edi]
	ret
      ta_address_immediate_sib:
	test	ch,44h
	jnz	ta_address_immediate_sib_32bit
	test	ch,not 88h
	jnz	ta_invalid_address_size
	test	edx,80000000h
	jz	ta_address_immediate_sib_store
	cmp	[ta_address_high],0
	je	ta_address_immediate_sib_nosignextend
      ta_address_immediate_sib_store:
	call	ta_store_instruction_code
	mov	al,100b
	mov	ah,100101b
	mov	cl,[ta_postbyte_register]
	shl	cl,3
	or	al,cl
	stos	word [edi]
	jmp	ta_store_address_32bit_value
      ta_address_immediate_sib_32bit:
	test	ecx,0FF0000h
	jnz	ta_address_immediate_sib_nosignextend
	test	edx,80000000h
	jz	ta_address_immediate_sib_store
      ta_address_immediate_sib_nosignextend:
	call	ta_address_32bit_prefix
	jmp	ta_address_immediate_sib_store
      ta_address_eip_based:
	mov	al,67h
	stos	byte [edi]
      ta_address_rip_based:
	cmp	[ta_code_type],64
	jne	ta_invalid_address
	call	ta_store_instruction_code
	jmp	ta_store_immediate_address
      ta_address_relative:
	call	ta_store_instruction_code
	movzx	eax,[ta_immediate_size]
	add	eax,edi
	sub	eax,[ta_current_offset]
	add	eax,5
	sub	edx,eax
	jno	ta_address_relative_ok
	call	ta_recoverable_overflow
      ta_address_relative_ok:
	mov	al,101b
	mov	cl,[ta_postbyte_register]
	shl	cl,3
	or	al,cl
	stos	byte [edi]
	shr	ecx,16
	xchg	[ta_value_type],cl
	mov	ebx,[ta_address_symbol]
	promote_ebx
	xchg	ebx,[ta_symbol_identifier]
	mov	eax,edx
	call	ta_mark_relocation
	mov	[ta_value_type],cl
	mov	[ta_symbol_identifier],ebx
	stos	dword [edi]
	ret
      ta_addressing_16bit:
	cmp	edx,10000h
	jge	ta_address_immediate_32bit
	cmp	edx,-8000h
	jl	ta_address_immediate_32bit
	movzx	edx,dx
      ta_address_immediate_16bit:
	call	ta_address_16bit_prefix
	call	ta_store_instruction_code
	mov	al,110b
	mov	cl,[ta_postbyte_register]
	shl	cl,3
	or	al,cl
	stos	byte [edi]
	mov	eax,edx
	stos	word [edi]
	cmp	edx,10000h
	jge	ta_value_out_of_range
	cmp	edx,-8000h
	jl	ta_value_out_of_range
	ret
      ta_address_16bit_prefix:
	cmp	[ta_code_type],16
	je	ta_instruction_prefix_ok
	mov	al,67h
	stos	byte [edi]
	ret
      ta_address_32bit_prefix:
	cmp	[ta_code_type],32
	je	ta_instruction_prefix_ok
	mov	al,67h
	stos	byte [edi]
      ta_instruction_prefix_ok:
	ret
ta_store_instruction_with_imm8:
	mov	[ta_immediate_size],1
	call	ta_store_instruction
	mov	al,byte [ta_value]
	stos	byte [edi]
	ret
ta_store_instruction_with_imm16:
	mov	[ta_immediate_size],2
	call	ta_store_instruction
	mov	ax,word [ta_value]
	call	ta_mark_relocation
	stos	word [edi]
	ret
ta_store_instruction_with_imm32:
	mov	[ta_immediate_size],4
	call	ta_store_instruction
	mov	eax,dword [ta_value]
	call	ta_mark_relocation
	stos	dword [edi]
	ret
