ta_assembler:
	xor	eax,eax
	mov	[ta_stub_size],eax
	mov	[ta_current_pass],ax
	mov	[ta_resolver_flags],eax
	mov	[ta_number_of_sections],eax
	mov	[ta_actual_fixups_size],eax
      ta_assembler_loop:
	mov	eax,[ta_labels_list]
	mov	[ta_tagged_blocks],eax
	mov	eax,[ta_additional_memory]
	mov	[ta_free_additional_memory],eax
	mov	eax,[ta_additional_memory_end]
	mov	[ta_structures_buffer],eax
	mov	esi,[ta_source_start]
	promote_esi
	mov	edi,[ta_code_start]
	promote_edi
	xor	eax,eax
	mov	dword [ta_adjustment],eax
	mov	dword [ta_adjustment+4],eax
	mov	[ta_addressing_space],eax
	mov	[ta_error_line],eax
	mov	[ta_counter],eax
	mov	[ta_format_flags],eax
	mov	[ta_number_of_relocations],eax
	mov	[ta_undefined_data_end],eax
	mov	[ta_file_extension],eax
	mov	[ta_next_pass_needed],al
	mov	[ta_output_format],al
	mov	[ta_adjustment_sign],al
	mov	[ta_evex_mode],al
	mov	[ta_code_type],16
	call	ta_init_addressing_space
      ta_pass_loop:
	call	ta_assemble_line
	jnc	ta_pass_loop
	mov	eax,[ta_additional_memory_end]
	cmp	eax,[ta_structures_buffer]
	je	ta_pass_done
	sub	eax,18h
	mov	eax,[eax+4]
	mov	[ta_current_line],eax
	jmp	ta_missing_end_directive
      ta_pass_done:
	call	ta_close_pass
	mov	eax,[ta_labels_list]
      ta_check_symbols:
	cmp	eax,[ta_memory_end]
	jae	ta_symbols_checked
	test	byte [eax+8],8
	jz	ta_symbol_defined_ok
	mov	cx,[ta_current_pass]
	cmp	cx,[eax+18]
	jne	ta_symbol_defined_ok
	test	byte [eax+8],1
	jz	ta_symbol_defined_ok
	sub	cx,[eax+16]
	cmp	cx,1
	jne	ta_symbol_defined_ok
	and	byte [eax+8],not 1
	or	[ta_next_pass_needed],-1
      ta_symbol_defined_ok:
	test	byte [eax+8],10h
	jz	ta_use_prediction_ok
	mov	cx,[ta_current_pass]
	and	byte [eax+8],not 10h
	test	byte [eax+8],20h
	jnz	ta_check_use_prediction
	cmp	cx,[eax+18]
	jne	ta_use_prediction_ok
	test	byte [eax+8],8
	jz	ta_use_prediction_ok
	jmp	ta_use_misprediction
      ta_check_use_prediction:
	test	byte [eax+8],8
	jz	ta_use_misprediction
	cmp	cx,[eax+18]
	je	ta_use_prediction_ok
      ta_use_misprediction:
	or	[ta_next_pass_needed],-1
      ta_use_prediction_ok:
	test	byte [eax+8],40h
	jz	ta_check_next_symbol
	and	byte [eax+8],not 40h
	test	byte [eax+8],4
	jnz	ta_define_misprediction
	mov	cx,[ta_current_pass]
	test	byte [eax+8],80h
	jnz	ta_check_define_prediction
	cmp	cx,[eax+16]
	jne	ta_check_next_symbol
	test	byte [eax+8],1
	jz	ta_check_next_symbol
	jmp	ta_define_misprediction
      ta_check_define_prediction:
	test	byte [eax+8],1
	jz	ta_define_misprediction
	cmp	cx,[eax+16]
	je	ta_check_next_symbol
      ta_define_misprediction:
	or	[ta_next_pass_needed],-1
      ta_check_next_symbol:
	add	eax,LABEL_STRUCTURE_SIZE
	jmp	ta_check_symbols
      ta_symbols_checked:
	cmp	[ta_next_pass_needed],0
	jne	ta_next_pass
	mov	eax,[ta_error_line]
	or	eax,eax
	jz	ta_assemble_ok
	mov	[ta_current_line],eax
	cmp	[ta_error],ta_undefined_symbol
	jne	ta_error_confirmed
	mov	eax,[ta_error_info]
	or	eax,eax
	jz	ta_error_confirmed
	test	byte [eax+8],1
	jnz	ta_next_pass
      ta_error_confirmed:
	call	ta_error_handler
      ta_error_handler:
	mov	eax,[ta_error]
	sub	eax,ta_error_handler
	add	[esp],eax
	ret
      ta_next_pass:
	inc	[ta_current_pass]
	mov	ax,[ta_current_pass]
	cmp	ax,[ta_passes_limit]
	je	ta_code_cannot_be_generated
	jmp	ta_assembler_loop
      ta_assemble_ok:
	ret

ta_create_addressing_space:
	mov	ebx,[ta_addressing_space]
	promote_ebx
	test	ebx,ebx
	jz	ta_init_addressing_space
	test	byte [ebx+0Ah],1
	jnz	ta_illegal_instruction
	mov	eax,edi
	sub	eax,[ebx+18h]
	mov	[ebx+1Ch],eax
      ta_init_addressing_space:
	mov	ebx,[ta_tagged_blocks]
	promote_ebx
	mov	dword [ebx-4],10h
	mov	dword [ebx-8],24h
	sub	ebx,8+24h
	cmp	ebx,edi
	jbe	ta_out_of_memory
	mov	[ta_tagged_blocks],ebx
	mov	[ta_addressing_space],ebx
	xor	eax,eax
	mov	[ebx],edi
	mov	[ebx+4],eax
	mov	[ebx+8],eax
	mov	[ebx+10h],eax
	mov	[ebx+14h],eax
	mov	[ebx+18h],edi
	mov	[ebx+1Ch],eax
	mov	[ebx+20h],eax
	ret

ta_assemble_line:
	mov	eax,[ta_tagged_blocks]
	sub	eax,100h
	cmp	edi,eax
	ja	ta_out_of_memory
	lods	byte [esi]
	cmp	al,1
	je	ta_assemble_instruction
	jb	ta_source_end
	cmp	al,3
	jb	ta_define_label
	je	ta_define_constant
	cmp	al,4
	je	ta_label_addressing_space
	cmp	al,0Fh
	je	ta_new_line
	cmp	al,13h
	je	ta_code_type_setting
	cmp	al,10h
	jne	ta_illegal_instruction
	lods	byte [esi]
	jmp	ta_segment_prefix
      ta_code_type_setting:
	lods	byte [esi]
	mov	[ta_code_type],al
	jmp	ta_instruction_assembled
      ta_new_line:
	lods	dword [esi]
	mov	[ta_current_line],eax
	and	[ta_prefix_flags],0
	cmp	[ta_symbols_file],0
	je	ta_continue_line
	cmp	[ta_next_pass_needed],0
	jne	ta_continue_line
	mov	ebx,[ta_tagged_blocks]
	promote_ebx
	mov	dword [ebx-4],1
	mov	dword [ebx-8],14h
	sub	ebx,8+14h
	cmp	ebx,edi
	jbe	ta_out_of_memory
	mov	[ta_tagged_blocks],ebx
	mov	[ebx],eax
	mov	[ebx+4],edi
	mov	eax,[ta_addressing_space]
	mov	[ebx+8],eax
	mov	al,[ta_code_type]
	mov	[ebx+10h],al
      ta_continue_line:
	cmp	byte [esi],0Fh
	je	ta_line_assembled
	jmp	ta_assemble_line
      ta_define_label:
	lods	dword [esi]
	cmp	eax,0Fh
	jb	ta_invalid_use_of_symbol
	je	ta_reserved_word_used_as_symbol
	mov	ebx,eax
	lods	byte [esi]
	mov	[ta_label_size],al
	call	ta_make_label
	jmp	ta_continue_line
      ta_make_label:
	mov	eax,edi
	xor	edx,edx
	xor	cl,cl
	mov	ebp,[ta_addressing_space]
	sub	eax,[ds:ebp]
	sbb	edx,[ds:ebp+4]
	sbb	cl,[ds:ebp+8]
	jp	ta_label_value_ok
	call	ta_recoverable_overflow
      ta_label_value_ok:
	mov	[ta_address_sign],cl
	test	byte [ds:ebp+0Ah],1
	jnz	ta_make_virtual_label
	or	byte [ebx+9],1
	xchg	eax,[ebx]
	xchg	edx,[ebx+4]
	mov	ch,[ebx+9]
	shr	ch,1
	and	ch,1
	neg	ch
	sub	eax,[ebx]
	sbb	edx,[ebx+4]
	sbb	ch,cl
	mov	dword [ta_adjustment],eax
	mov	dword [ta_adjustment+4],edx
	mov	[ta_adjustment_sign],ch
	or	al,ch
	or	eax,edx
	setnz	ah
	jmp	ta_finish_label
      ta_make_virtual_label:
	and	byte [ebx+9],not 1
	cmp	eax,[ebx]
	mov	[ebx],eax
	setne	ah
	cmp	edx,[ebx+4]
	mov	[ebx+4],edx
	setne	al
	or	ah,al
      ta_finish_label:
	mov	ebp,[ta_addressing_space]
	mov	ch,[ds:ebp+9]
	mov	cl,[ta_label_size]
	mov	edx,[ds:ebp+14h]
	mov	ebp,[ds:ebp+10h]
      ta_finish_label_symbol:
	mov	al,[ta_address_sign]
	xor	al,[ebx+9]
	and	al,10b
	or	ah,al
	xor	[ebx+9],al
	cmp	cl,[ebx+10]
	mov	[ebx+10],cl
	setne	al
	or	ah,al
	cmp	ch,[ebx+11]
	mov	[ebx+11],ch
	setne	al
	or	ah,al
	cmp	ebp,[ebx+12]
	mov	[ebx+12],ebp
	setne	al
	or	ah,al
	or	ch,ch
	jz	ta_label_symbol_ok
	cmp	edx,[ebx+20]
	mov	[ebx+20],edx
	setne	al
	or	ah,al
      ta_label_symbol_ok:
	mov	cx,[ta_current_pass]
	xchg	[ebx+16],cx
	mov	edx,[ta_current_line]
	promote_edx
	mov	[ebx+28],edx
	and	byte [ebx+8],not 2
	test	byte [ebx+8],1
	jz	ta_new_label
	cmp	cx,[ebx+16]
	je	ta_symbol_already_defined
	btr	dword [ebx+8],10
	jc	ta_requalified_label
	inc	cx
	sub	cx,[ebx+16]
	setnz	al
	or	ah,al
	jz	ta_label_made
	test	byte [ebx+8],8
	jz	ta_label_made
	mov	cx,[ta_current_pass]
	cmp	cx,[ebx+18]
	jne	ta_label_made
      ta_requalified_label:
	or	[ta_next_pass_needed],-1
      ta_label_made:
	ret
      ta_new_label:
	or	byte [ebx+8],1
	ret
      ta_define_constant:
	lods	dword [esi]
	inc	esi
	cmp	eax,0Fh
	jb	ta_invalid_use_of_symbol
	je	ta_reserved_word_used_as_symbol
	push	eax
	or	[ta_operand_flags],1
	call	ta_get_value
	pop	ebx
	xor	cl,cl
	mov	ch,[ta_value_type]
	cmp	ch,3
	je	ta_invalid_use_of_symbol
      ta_make_constant:
	and	byte [ebx+9],not 1
	cmp	eax,[ebx]
	mov	[ebx],eax
	setne	ah
	cmp	edx,[ebx+4]
	mov	[ebx+4],edx
	setne	al
	or	ah,al
	mov	al,[ta_value_sign]
	xor	al,[ebx+9]
	and	al,10b
	or	ah,al
	xor	[ebx+9],al
	cmp	cl,[ebx+10]
	mov	[ebx+10],cl
	setne	al
	or	ah,al
	cmp	ch,[ebx+11]
	mov	[ebx+11],ch
	setne	al
	or	ah,al
	xor	edx,edx
	cmp	edx,[ebx+12]
	mov	[ebx+12],edx
	setne	al
	or	ah,al
	or	ch,ch
	jz	ta_constant_symbol_ok
	mov	edx,[ta_symbol_identifier]
	promote_edx
	cmp	edx,[ebx+20]
	mov	[ebx+20],edx
	setne	al
	or	ah,al
      ta_constant_symbol_ok:
	mov	cx,[ta_current_pass]
	xchg	[ebx+16],cx
	mov	edx,[ta_current_line]
	promote_edx
	mov	[ebx+28],edx
	test	byte [ebx+8],1
	jz	ta_new_constant
	cmp	cx,[ebx+16]
	jne	ta_redeclare_constant
	test	byte [ebx+8],2
	jz	ta_symbol_already_defined
	or	byte [ebx+8],4
	and	byte [ebx+9],not 4
	jmp	ta_instruction_assembled
      ta_redeclare_constant:
	btr	dword [ebx+8],10
	jc	ta_requalified_constant
	inc	cx
	sub	cx,[ebx+16]
	setnz	al
	or	ah,al
	jz	ta_instruction_assembled
	test	byte [ebx+8],4
	jnz	ta_instruction_assembled
	test	byte [ebx+8],8
	jz	ta_instruction_assembled
	mov	cx,[ta_current_pass]
	cmp	cx,[ebx+18]
	jne	ta_instruction_assembled
      ta_requalified_constant:
	or	[ta_next_pass_needed],-1
	jmp	ta_instruction_assembled
      ta_new_constant:
	or	byte [ebx+8],1+2
	jmp	ta_instruction_assembled
      ta_label_addressing_space:
	lods	dword [esi]
	cmp	eax,0Fh
	jb	ta_invalid_use_of_symbol
	je	ta_reserved_word_used_as_symbol
	mov	cx,[ta_current_pass]
	test	byte [eax+8],1
	jz	ta_make_addressing_space_label
	cmp	cx,[eax+16]
	je	ta_symbol_already_defined
	test	byte [eax+9],4
	jnz	ta_make_addressing_space_label
	or	[ta_next_pass_needed],-1
      ta_make_addressing_space_label:
	mov	dx,[eax+8]
	and	dx,not (2 or 100h)
	or	dx,1 or 4 or 400h
	mov	[eax+8],dx
	mov	[eax+16],cx
	mov	edx,[ta_current_line]
	promote_edx
	mov	[eax+28],edx
	mov	ebx,[ta_addressing_space]
	promote_ebx
	mov	[eax],ebx
	or	byte [ebx+0Ah],2
	jmp	ta_continue_line
      ta_assemble_instruction:
;	 mov	 [ta_operand_size],0
;	 mov	 [ta_operand_flags],0
;	 mov	 [ta_operand_prefix],0
;	 mov	 [ta_rex_prefix],0
	and	dword [ta_operand_size],0
;	 mov	 [ta_opcode_prefix],0
;	 mov	 [ta_vex_required],0
;	 mov	 [ta_vex_register],0
;	 mov	 [ta_immediate_size],0
	and	dword [ta_opcode_prefix],0
	call	ta_instruction_handler
      ta_instruction_handler:
	movzx	ebx,word [esi]
	mov	al,[esi+2]
	add	esi,3
	add	[esp],ebx
	ret
      ta_instruction_assembled:
	test	[ta_prefix_flags],not 1
	jnz	ta_illegal_instruction
	mov	al,[esi]
	cmp	al,0Fh
	je	ta_line_assembled
	or	al,al
	jnz	ta_extra_characters_on_line
      ta_line_assembled:
	clc
	ret
      ta_source_end:
	dec	esi
	stc
	ret

ta_org_directive:
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_argument
	cmp	byte [esi],'.'
	je	ta_invalid_value
	call	ta_get_qword_value
	mov	cl,[ta_value_type]
	test	cl,1
	jnz	ta_invalid_use_of_symbol
	push	eax
	mov	ebx,[ta_addressing_space]
	promote_ebx
	mov	eax,edi
	sub	eax,[ebx+18h]
	mov	[ebx+1Ch],eax
	test	byte [ebx+0Ah],1
	jnz	ta_in_virtual
	call	ta_init_addressing_space
	jmp	ta_org_space_ok
      ta_in_virtual:
	call	ta_close_virtual_addressing_space
	call	ta_init_addressing_space
	or	byte [ebx+0Ah],1
      ta_org_space_ok:
	pop	eax
	mov	[ebx+9],cl
	mov	cl,[ta_value_sign]
	sub	[ebx],eax
	sbb	[ebx+4],edx
	sbb	byte [ebx+8],cl
	jp	ta_org_value_ok
	call	ta_recoverable_overflow
      ta_org_value_ok:
	mov	edx,[ta_symbol_identifier]
	promote_edx
	mov	[ebx+14h],edx
	cmp	[ta_output_format],1
	ja	ta_instruction_assembled
	cmp	edi,[ta_code_start]
	jne	ta_instruction_assembled
	cmp	eax,100h
	jne	ta_instruction_assembled
	bts	[ta_format_flags],0
	jmp	ta_instruction_assembled
ta_label_directive:
	lods	byte [esi]
	cmp	al,2
	jne	ta_invalid_argument
	lods	dword [esi]
	cmp	eax,0Fh
	jb	ta_invalid_use_of_symbol
	je	ta_reserved_word_used_as_symbol
	inc	esi
	mov	ebx,eax
	mov	[ta_label_size],0
	lods	byte [esi]
	cmp	al,':'
	je	ta_get_label_size
	dec	esi
	cmp	al,11h
	jne	ta_label_size_ok
      ta_get_label_size:
	lods	word [esi]
	cmp	al,11h
	jne	ta_invalid_argument
	mov	[ta_label_size],ah
      ta_label_size_ok:
	cmp	byte [esi],80h
	je	ta_get_free_label_value
	call	ta_make_label
	jmp	ta_instruction_assembled
      ta_get_free_label_value:
	inc	esi
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_argument
	push	ebx ecx
	or	byte [ebx+8],4
	cmp	byte [esi],'.'
	je	ta_invalid_value
	call	ta_get_address_value
	or	bh,bh
	setnz	ch
	xchg	ch,cl
	mov	bp,cx
	shl	ebp,16
	xchg	bl,bh
	mov	bp,bx
	pop	ecx ebx
	and	byte [ebx+8],not 4
	mov	ch,[ta_value_type]
	test	ch,1
	jnz	ta_invalid_use_of_symbol
      ta_make_free_label:
	and	byte [ebx+9],not 1
	cmp	eax,[ebx]
	mov	[ebx],eax
	setne	ah
	cmp	edx,[ebx+4]
	mov	[ebx+4],edx
	setne	al
	or	ah,al
	mov	edx,[ta_address_symbol]
	promote_edx
	mov	cl,[ta_label_size]
	call	ta_finish_label_symbol
	jmp	ta_instruction_assembled
ta_load_directive:
	lods	byte [esi]
	cmp	al,2
	jne	ta_invalid_argument
	lods	dword [esi]
	cmp	eax,0Fh
	jb	ta_invalid_use_of_symbol
	je	ta_reserved_word_used_as_symbol
	inc	esi
	push	eax
	mov	al,1
	cmp	byte [esi],11h
	jne	ta_load_size_ok
	lods	byte [esi]
	lods	byte [esi]
      ta_load_size_ok:
	cmp	al,8
	ja	ta_invalid_value
	mov	[ta_operand_size],al
	and	dword [ta_value],0
	and	dword [ta_value+4],0
	lods	byte [esi]
	cmp	al,82h
	jne	ta_invalid_argument
	call	ta_get_data_point
	jc	ta_value_loaded
	push	esi edi
	mov	esi,ebx
	mov	edi,ta_value
	rep	movs byte [edi],[esi]
	pop	edi esi
      ta_value_loaded:
	mov	[ta_value_sign],0
	mov	eax,dword [ta_value]
	mov	edx,dword [ta_value+4]
	pop	ebx
	xor	cx,cx
	jmp	ta_make_constant
      ta_get_data_point:
	lods	byte [esi]
	cmp	al,':'
	je	ta_get_data_offset
	cmp	al,'('
	jne	ta_invalid_argument
	mov	ebx,[ta_addressing_space]
	promote_ebx
	mov	ecx,edi
	sub	ecx,[ebx+18h]
	mov	[ebx+1Ch],ecx
	cmp	byte [esi],11h
	jne	ta_get_data_address
	cmp	word [esi+1+4],'):'
	jne	ta_get_data_address
	inc	esi
	lods	dword [esi]
	add	esi,2
	cmp	byte [esi],'('
	jne	ta_invalid_argument
	inc	esi
	cmp	eax,0Fh
	jbe	ta_reserved_word_used_as_symbol
	mov	edx,ta_undefined_symbol
	test	byte [eax+8],1
	jz	ta_addressing_space_unavailable
	mov	edx,ta_symbol_out_of_scope
	mov	cx,[eax+16]
	cmp	cx,[ta_current_pass]
	jne	ta_addressing_space_unavailable
	test	byte [eax+9],4
	jz	ta_invalid_use_of_symbol
	mov	ebx,eax
	mov	ax,[ta_current_pass]
	mov	[ebx+18],ax
	or	byte [ebx+8],8
	call	ta_store_label_reference
      ta_get_addressing_space:
	mov	ebx,[ebx]
      ta_get_data_address:
	push	ebx
	cmp	byte [esi],'.'
	je	ta_invalid_value
	or	[ta_operand_flags],1
	call	ta_get_address_value
	pop	ebp
	call	ta_calculate_relative_offset
	cmp	[ta_next_pass_needed],0
	jne	ta_data_address_type_ok
	cmp	[ta_value_type],0
	jne	ta_invalid_use_of_symbol
      ta_data_address_type_ok:
	mov	ebx,edi
	xor	ecx,ecx
	add	ebx,eax
	adc	edx,ecx
	mov	eax,ebx
	sub	eax,[ds:ebp+18h]
	sbb	edx,ecx
	jnz	ta_bad_data_address
	mov	cl,[ta_operand_size]
	add	eax,ecx
	cmp	eax,[ds:ebp+1Ch]
	ja	ta_bad_data_address
	clc
	ret
      ta_addressing_space_unavailable:
	cmp	[ta_error_line],0
	jne	ta_get_data_address
	push	[ta_current_line]
	pop	[ta_error_line]
	mov	[ta_error],edx
	mov	[ta_error_info],eax
	jmp	ta_get_data_address
      ta_bad_data_address:
	call	ta_recoverable_overflow
	stc
	ret
      ta_get_data_offset:
	cmp	[ta_output_format],2
	jae	ta_invalid_operand
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_argument
	cmp	byte [esi],'.'
	je	ta_invalid_value
	call	ta_get_dword_value
	cmp	[ta_value_type],0
	je	ta_data_offset_ok
	call	ta_recoverable_invalid_address
      ta_data_offset_ok:
	add	eax,[ta_code_start]
	jc	ta_bad_data_address
	mov	ebx,eax
	movzx	ecx,[ta_operand_size]
	add	eax,ecx
	jc	ta_bad_data_address
	mov	edx,[ta_addressing_space]
	promote_edx
	test	byte [edx+0Ah],1
	jnz	ta_data_offset_from_virtual
	cmp	eax,edi
	ja	ta_bad_data_address
	clc
	ret
      ta_data_offset_from_virtual:
	cmp	eax,[ta_undefined_data_end]
	ja	ta_bad_data_address
	clc
	ret

ta_store_directive:
	cmp	byte [esi],11h
	je	ta_sized_store
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_argument
	call	ta_get_byte_value
	xor	edx,edx
	movzx	eax,al
	mov	[ta_operand_size],1
	jmp	ta_store_value_ok
      ta_sized_store:
	or	[ta_operand_flags],1
	call	ta_get_value
      ta_store_value_ok:
	cmp	[ta_value_type],0
	jne	ta_invalid_use_of_symbol
	mov	dword [ta_value],eax
	mov	dword [ta_value+4],edx
	lods	byte [esi]
	cmp	al,80h
	jne	ta_invalid_argument
	call	ta_get_data_point
	jc	ta_instruction_assembled
	push	esi edi
	mov	esi,ta_value
	mov	edi,ebx
	rep	movs byte [edi],[esi]
	mov	eax,edi
	pop	edi esi
	cmp	ebx,[ta_undefined_data_end]
	jae	ta_instruction_assembled
	cmp	eax,[ta_undefined_data_start]
	jbe	ta_instruction_assembled
	mov	[ta_undefined_data_start],eax
	jmp	ta_instruction_assembled

ta_display_directive:
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_argument
	cmp	byte [esi],0
	jne	ta_display_byte
	inc	esi
	lods	dword [esi]
	mov	ecx,eax
	push	edi
	mov	edi,[ta_tagged_blocks]
	promote_edi
	sub	edi,8
	sub	edi,eax
	cmp	edi,[esp]
	jbe	ta_out_of_memory
	mov	[ta_tagged_blocks],edi
	rep	movs byte [edi],[esi]
	stos	dword [edi]
	xor	eax,eax
	stos	dword [edi]
	pop	edi
	inc	esi
	jmp	ta_display_next
      ta_display_byte:
	call	ta_get_byte_value
	push	edi
	mov	edi,[ta_tagged_blocks]
	promote_edi
	sub	edi,8+1
	mov	[ta_tagged_blocks],edi
	stos	byte [edi]
	mov	eax,1
	stos	dword [edi]
	dec	eax
	stos	dword [edi]
	pop	edi
      ta_display_next:
	cmp	edi,[ta_tagged_blocks]
	ja	ta_out_of_memory
	lods	byte [esi]
	cmp	al,','
	je	ta_display_directive
	dec	esi
	jmp	ta_instruction_assembled
ta_show_display_buffer:
	mov	eax,[ta_tagged_blocks]
	or	eax,eax
	jz	ta_display_done
	mov	esi,[ta_labels_list]
	promote_esi
	cmp	esi,eax
	je	ta_display_done
      ta_display_messages:
	sub	esi,8
	mov	eax,[esi+4]
	mov	ecx,[esi]
	sub	esi,ecx
	cmp	eax,10h
	je	ta_write_addressing_space
	test	eax,eax
	jnz	ta_skip_block
	push	esi
	call	ta_display_block
	pop	esi
      ta_skip_block:
	cmp	esi,[ta_tagged_blocks]
	jne	ta_display_messages
      ta_display_done:
	ret
      ta_write_addressing_space:
	mov	ecx,[esi+20h]
	jecxz	ta_skip_block
	push	esi
	mov	edi,[ta_free_additional_memory]
	promote_edi
	mov	esi,[ta_output_file]
	promote_esi
	test	esi,esi
	jz	ta_addressing_space_written
	xor	ebx,ebx
      ta_copy_output_path:
	lodsb
	cmp	edi,[ta_structures_buffer]
	jae	ta_out_of_memory
	stosb
	test	al,al
	jz	ta_output_path_copied
	cmp	al,'/'
	je	ta_new_path_segment
	cmp	al,'\'
	je	ta_new_path_segment
	cmp	al,'.'
	jne	ta_copy_output_path
	mov	ebx,edi
	jmp	ta_copy_output_path
      ta_new_path_segment:
	xor	ebx,ebx
	jmp	ta_copy_output_path
      ta_output_path_copied:
	test	ebx,ebx
	jnz	ta_append_extension
	mov	byte [edi-1],'.'
	mov	ebx,edi
      ta_append_extension:
	mov	edi,ebx
	add	ebx,ecx
	inc	ebx
	cmp	ebx,[ta_structures_buffer]
	jae	ta_out_of_memory
	mov	esi,[esp]
	mov	esi,[esi+18h]
	sub	esi,ecx
	rep	movs byte [edi],[esi]
	xor	al,al
	stos	byte [edi]
	mov	edx,[ta_free_additional_memory]
	promote_edx
	call	ta_create
	jc	ta_write_failed
	mov	esi,[esp]
	mov	edx,[esi+18h]
	mov	ecx,[esi+1Ch]
	call	ta_write
	jc	ta_write_failed
	call	ta_close
      ta_addressing_space_written:
	pop	esi
	jmp	ta_skip_block

ta_times_directive:
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_argument
	cmp	byte [esi],'.'
	je	ta_invalid_value
	call	ta_get_count_value
	cmp	eax,0
	je	ta_zero_times
	cmp	byte [esi],':'
	jne	ta_times_argument_ok
	inc	esi
      ta_times_argument_ok:
	push	[ta_counter]
	push	[ta_counter_limit]
	mov	[ta_counter_limit],eax
	mov	[ta_counter],1
      ta_times_loop:
	mov	eax,esp
	sub	eax,[ta_stack_limit]
	cmp	eax,100h
	jb	ta_stack_overflow
	push	esi
	or	[ta_prefix_flags],1
	call	ta_continue_line
	mov	eax,[ta_counter_limit]
	cmp	[ta_counter],eax
	je	ta_times_done
	inc	[ta_counter]
	pop	esi
	jmp	ta_times_loop
      ta_times_done:
	pop	eax
	pop	[ta_counter_limit]
	pop	[ta_counter]
	jmp	ta_instruction_assembled
      ta_zero_times:
	call	ta_skip_symbol
	jnc	ta_zero_times
	jmp	ta_instruction_assembled

ta_virtual_directive:
	lods	byte [esi]
	cmp	al,'('
	je	ta_continue_virtual_area
	cmp	al,80h
	jne	ta_virtual_at_current
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_argument
	cmp	byte [esi],'.'
	je	ta_invalid_value
	call	ta_get_address_value
	mov	ebp,[ta_address_symbol]
	or	bh,bh
	setnz	ch
	jmp	ta_set_virtual
      ta_virtual_at_current:
	dec	esi
      ta_virtual_fallback:
	mov	ebp,[ta_addressing_space]
	mov	al,[ds:ebp+9]
	mov	[ta_value_type],al
	mov	eax,edi
	xor	edx,edx
	xor	cl,cl
	sub	eax,[ds:ebp]
	sbb	edx,[ds:ebp+4]
	sbb	cl,[ds:ebp+8]
	mov	[ta_address_sign],cl
	mov	bx,[ds:ebp+10h]
	mov	cx,[ds:ebp+10h+2]
	xchg	bh,bl
	xchg	ch,cl
	mov	ebp,[ds:ebp+14h]
      ta_set_virtual:
	xchg	bl,bh
	xchg	cl,ch
	shl	ecx,16
	mov	cx,bx
	push	ecx eax
	mov	ebx,[ta_addressing_space]
	promote_ebx
	test	byte [ebx+0Ah],1
	jnz	ta_non_virtual_end_ok
	mov	eax,edi
	xchg	eax,[ta_undefined_data_end]
	cmp	eax,edi
	je	ta_non_virtual_end_ok
	mov	[ta_undefined_data_start],edi
      ta_non_virtual_end_ok:
	call	ta_allocate_virtual_structure_data
	call	ta_init_addressing_space
	or	byte [ebx+0Ah],1
	cmp	byte [esi],86h
	jne	ta_addressing_space_extension_ok
	cmp	word [esi+1],'('
	jne	ta_invalid_argument
	mov	ecx,[esi+3]
	add	esi,3+4
	add	[ebx+18h],ecx
	mov	[ebx+20h],ecx
	or	byte [ebx+0Ah],2
	push	ebx
	mov	ebx,ta_characters
      ta_get_extension:
	lods	byte [esi]
	stos	byte [edi]
	xlat	byte [ebx]
	test	al,al
	jz	ta_invalid_argument
	loop	ta_get_extension
	inc	esi
	pop	ebx
      ta_addressing_space_extension_ok:
	pop	eax
	mov	cl,[ta_address_sign]
	not	eax
	not	edx
	not	cl
	add	eax,1
	adc	edx,0
	adc	cl,0
	add	eax,edi
	adc	edx,0
	adc	cl,0
	mov	[ebx],eax
	mov	[ebx+4],edx
	mov	[ebx+8],cl
	pop	dword [ebx+10h]
	mov	[ebx+14h],ebp
	mov	al,[ta_value_type]
	test	al,1
	jnz	ta_invalid_use_of_symbol
	mov	[ebx+9],al
	jmp	ta_instruction_assembled
      ta_allocate_structure_data:
	mov	ebx,[ta_structures_buffer]
	promote_ebx
	sub	ebx,18h
	cmp	ebx,[ta_free_additional_memory]
	jb	ta_out_of_memory
	mov	[ta_structures_buffer],ebx
	ret
      ta_find_structure_data:
	mov	ebx,[ta_structures_buffer]
	promote_ebx
      ta_scan_structures:
	cmp	ebx,[ta_additional_memory_end]
	je	ta_no_such_structure
	cmp	ax,[ebx]
	je	ta_structure_data_found
	add	ebx,18h
	jmp	ta_scan_structures
      ta_structure_data_found:
	ret
      ta_no_such_structure:
	stc
	ret
      ta_allocate_virtual_structure_data:
	call	ta_allocate_structure_data
	mov	word [ebx],ta_virtual_directive-ta_instruction_handler
	mov	ecx,[ta_addressing_space]
	promote_ecx
	mov	[ebx+12],ecx
	mov	[ebx+8],edi
	mov	ecx,[ta_current_line]
	promote_ecx
	mov	[ebx+4],ecx
	mov	ebx,[ta_addressing_space]
	promote_ebx
	mov	eax,edi
	sub	eax,[ebx+18h]
	mov	[ebx+1Ch],eax
	ret
      ta_continue_virtual_area:
	cmp	byte [esi],11h
	jne	ta_invalid_argument
	cmp	byte [esi+1+4],')'
	jne	ta_invalid_argument
	inc	esi
	lods	dword [esi]
	inc	esi
	cmp	eax,0Fh
	jbe	ta_reserved_word_used_as_symbol
	mov	edx,ta_undefined_symbol
	test	byte [eax+8],1
	jz	ta_virtual_area_unavailable
	mov	edx,ta_symbol_out_of_scope
	mov	cx,[eax+16]
	cmp	cx,[ta_current_pass]
	jne	ta_virtual_area_unavailable
	mov	edx,ta_invalid_use_of_symbol
	test	byte [eax+9],4
	jz	ta_virtual_area_unavailable
	mov	ebx,eax
	mov	ax,[ta_current_pass]
	mov	[ebx+18],ax
	or	byte [ebx+8],8
	call	ta_store_label_reference
	mov	ebx,[ebx]
	test	byte [ebx+0Ah],4
	jz	ta_virtual_area_unavailable
	and	byte [ebx+0Ah],not 4
	mov	edx,ebx
	call	ta_allocate_virtual_structure_data
	mov	[ta_addressing_space],edx
	push	esi
	mov	esi,[edx+18h]
	mov	ecx,[edx+1Ch]
	mov	eax,[edx+20h]
	sub	esi,eax
	add	ecx,eax
	lea	eax,[edi+ecx]
	cmp	eax,[ta_tagged_blocks]
	jae	ta_out_of_memory
	mov	eax,esi
	sub	eax,edi
	sub	[edx+18h],eax
	sub	[edx],eax
	sbb	dword [edx+4],0
	sbb	byte [edx+8],0
	mov	al,cl
	shr	ecx,2
	rep	movs dword [edi],[esi]
	mov	cl,al
	and	cl,11b
	rep	movs byte [edi],[esi]
	pop	esi
	jmp	ta_instruction_assembled
      ta_virtual_area_unavailable:
	cmp	[ta_error_line],0
	jne	ta_virtual_fallback
	push	[ta_current_line]
	pop	[ta_error_line]
	mov	[ta_error],edx
	mov	[ta_error_info],eax
	jmp	ta_virtual_fallback
      ta_end_virtual:
	call	ta_find_structure_data
	jc	ta_unexpected_instruction
	push	ebx
	call	ta_close_virtual_addressing_space
	pop	ebx
	mov	eax,[ebx+12]
	mov	[ta_addressing_space],eax
	mov	edi,[ebx+8]
      ta_remove_structure_data:
	push	esi edi
	mov	ecx,ebx
	sub	ecx,[ta_structures_buffer]
	shr	ecx,2
	lea	esi,[ebx-4]
	lea	edi,[esi+18h]
	std
	rep	movs dword [edi],[esi]
	cld
	add	[ta_structures_buffer],18h
	pop	edi esi
	ret
      ta_close_virtual_addressing_space:
	mov	ebx,[ta_addressing_space]
	promote_ebx
	mov	eax,edi
	sub	eax,[ebx+18h]
	mov	[ebx+1Ch],eax
	add	eax,[ebx+20h]
	test	byte [ebx+0Ah],2
	jz	ta_addressing_space_closed
	or	byte [ebx+0Ah],4
	push	esi edi ecx edx
	mov	ecx,eax
	mov	eax,[ta_tagged_blocks]
	mov	dword [eax-4],11h
	mov	dword [eax-8],ecx
	sub	eax,8
	sub	eax,ecx
	mov	[ta_tagged_blocks],eax
	lea	edi,[eax+ecx-1]
	add	eax,[ebx+20h]
	xchg	eax,[ebx+18h]
	sub	eax,[ebx+20h]
	lea	esi,[eax+ecx-1]
	mov	eax,edi
	sub	eax,esi
	std
	shr	ecx,1
	jnc	ta_virtual_byte_ok
	movs	byte [edi],[esi]
      ta_virtual_byte_ok:
	dec	esi
	dec	edi
	shr	ecx,1
	jnc	ta_virtual_word_ok
	movs	word [edi],[esi]
      ta_virtual_word_ok:
	sub	esi,2
	sub	edi,2
	rep	movs dword [edi],[esi]
	cld
	xor	edx,edx
	add	[ebx],eax
	adc	dword [ebx+4],edx
	adc	byte [ebx+8],dl
	pop	edx ecx edi esi
      ta_addressing_space_closed:
	ret
ta_repeat_directive:
	test	[ta_prefix_flags],1
	jnz	ta_unexpected_instruction
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_argument
	cmp	byte [esi],'.'
	je	ta_invalid_value
	call	ta_get_count_value
	cmp	eax,0
	je	ta_zero_repeat
	call	ta_allocate_structure_data
	mov	word [ebx],ta_repeat_directive-ta_instruction_handler
	xchg	eax,[ta_counter_limit]
	mov	[ebx+10h],eax
	mov	eax,1
	xchg	eax,[ta_counter]
	mov	[ebx+14h],eax
	mov	[ebx+8],esi
	mov	eax,[ta_current_line]
	mov	[ebx+4],eax
	jmp	ta_instruction_assembled
      ta_end_repeat:
	test	[ta_prefix_flags],1
	jnz	ta_unexpected_instruction
	call	ta_find_structure_data
	jc	ta_unexpected_instruction
	mov	eax,[ta_counter_limit]
	inc	[ta_counter]
	cmp	[ta_counter],eax
	jbe	ta_continue_repeating
      ta_stop_repeat:
	mov	eax,[ebx+10h]
	mov	[ta_counter_limit],eax
	mov	eax,[ebx+14h]
	mov	[ta_counter],eax
	call	ta_remove_structure_data
	jmp	ta_instruction_assembled
      ta_continue_repeating:
	mov	esi,[ebx+8]
	jmp	ta_instruction_assembled
      ta_zero_repeat:
	mov	al,[esi]
	or	al,al
	jz	ta_missing_end_directive
	cmp	al,0Fh
	jne	ta_extra_characters_on_line
	call	ta_find_end_repeat
	jmp	ta_instruction_assembled
      ta_find_end_repeat:
	call	ta_find_structure_end
	cmp	ax,ta_repeat_directive-ta_instruction_handler
	jne	ta_unexpected_instruction
	ret
ta_while_directive:
	test	[ta_prefix_flags],1
	jnz	ta_unexpected_instruction
	call	ta_allocate_structure_data
	mov	word [ebx],ta_while_directive-ta_instruction_handler
	mov	eax,1
	xchg	eax,[ta_counter]
	mov	[ebx+10h],eax
	mov	[ebx+8],esi
	mov	eax,[ta_current_line]
	mov	[ebx+4],eax
      ta_do_while:
	push	ebx
	call	ta_calculate_logical_expression
	or	al,al
	jnz	ta_while_true
	mov	al,[esi]
	or	al,al
	jz	ta_missing_end_directive
	cmp	al,0Fh
	jne	ta_extra_characters_on_line
      ta_stop_while:
	call	ta_find_end_while
	pop	ebx
	mov	eax,[ebx+10h]
	mov	[ta_counter],eax
	call	ta_remove_structure_data
	jmp	ta_instruction_assembled
      ta_while_true:
	pop	ebx
	jmp	ta_instruction_assembled
      ta_end_while:
	test	[ta_prefix_flags],1
	jnz	ta_unexpected_instruction
	call	ta_find_structure_data
	jc	ta_unexpected_instruction
	mov	eax,[ebx+4]
	mov	[ta_current_line],eax
	inc	[ta_counter]
	jz	ta_too_many_repeats
	mov	esi,[ebx+8]
	jmp	ta_do_while
      ta_find_end_while:
	call	ta_find_structure_end
	cmp	ax,ta_while_directive-ta_instruction_handler
	jne	ta_unexpected_instruction
	ret
ta_if_directive:
	test	[ta_prefix_flags],1
	jnz	ta_unexpected_instruction
	call	ta_calculate_logical_expression
	mov	dl,al
	mov	al,[esi]
	or	al,al
	jz	ta_missing_end_directive
	cmp	al,0Fh
	jne	ta_extra_characters_on_line
	or	dl,dl
	jnz	ta_if_true
	call	ta_find_else
	jc	ta_instruction_assembled
	mov	al,[esi]
	cmp	al,1
	jne	ta_else_true
	cmp	word [esi+1],ta_if_directive-ta_instruction_handler
	jne	ta_else_true
	add	esi,4
	jmp	ta_if_directive
      ta_if_true:
	xor	al,al
      ta_make_if_structure:
	call	ta_allocate_structure_data
	mov	word [ebx],ta_if_directive-ta_instruction_handler
	mov	byte [ebx+2],al
	mov	eax,[ta_current_line]
	mov	[ebx+4],eax
	jmp	ta_instruction_assembled
      ta_else_true:
	or	al,al
	jz	ta_missing_end_directive
	cmp	al,0Fh
	jne	ta_extra_characters_on_line
	or	al,-1
	jmp	ta_make_if_structure
      ta_else_directive:
	test	[ta_prefix_flags],1
	jnz	ta_unexpected_instruction
	mov	ax,ta_if_directive-ta_instruction_handler
	call	ta_find_structure_data
	jc	ta_unexpected_instruction
	cmp	byte [ebx+2],0
	jne	ta_unexpected_instruction
      ta_found_else:
	mov	al,[esi]
	cmp	al,1
	jne	ta_skip_else
	cmp	word [esi+1],ta_if_directive-ta_instruction_handler
	jne	ta_skip_else
	add	esi,4
	call	ta_find_else
	jnc	ta_found_else
	call	ta_remove_structure_data
	jmp	ta_instruction_assembled
      ta_skip_else:
	or	al,al
	jz	ta_missing_end_directive
	cmp	al,0Fh
	jne	ta_extra_characters_on_line
	call	ta_find_end_if
	call	ta_remove_structure_data
	jmp	ta_instruction_assembled
      ta_end_if:
	test	[ta_prefix_flags],1
	jnz	ta_unexpected_instruction
	call	ta_find_structure_data
	jc	ta_unexpected_instruction
	call	ta_remove_structure_data
	jmp	ta_instruction_assembled
      ta_find_else:
	call	ta_find_structure_end
	cmp	ax,ta_else_directive-ta_instruction_handler
	je	ta_else_found
	cmp	ax,ta_if_directive-ta_instruction_handler
	jne	ta_unexpected_instruction
	stc
	ret
      ta_else_found:
	clc
	ret
      ta_find_end_if:
	call	ta_find_structure_end
	cmp	ax,ta_if_directive-ta_instruction_handler
	jne	ta_unexpected_instruction
	ret
      ta_find_structure_end:
	push	[ta_error_line]
	mov	eax,[ta_current_line]
	mov	[ta_error_line],eax
      ta_find_end_directive:
	call	ta_skip_symbol
	jnc	ta_find_end_directive
	lods	byte [esi]
	cmp	al,0Fh
	jne	ta_no_end_directive
	lods	dword [esi]
	mov	[ta_current_line],eax
      ta_skip_labels:
	cmp	byte [esi],2
	jne	ta_labels_ok
	add	esi,6
	jmp	ta_skip_labels
      ta_labels_ok:
	cmp	byte [esi],1
	jne	ta_find_end_directive
	mov	ax,[esi+1]
	cmp	ax,ta_prefix_instruction-ta_instruction_handler
	je	ta_find_end_directive
	add	esi,4
	cmp	ax,ta_repeat_directive-ta_instruction_handler
	je	ta_skip_repeat
	cmp	ax,ta_while_directive-ta_instruction_handler
	je	ta_skip_while
	cmp	ax,ta_if_directive-ta_instruction_handler
	je	ta_skip_if
	cmp	ax,ta_else_directive-ta_instruction_handler
	je	ta_structure_end
	cmp	ax,ta_end_directive-ta_instruction_handler
	jne	ta_find_end_directive
	cmp	byte [esi],1
	jne	ta_find_end_directive
	mov	ax,[esi+1]
	add	esi,4
	cmp	ax,ta_repeat_directive-ta_instruction_handler
	je	ta_structure_end
	cmp	ax,ta_while_directive-ta_instruction_handler
	je	ta_structure_end
	cmp	ax,ta_if_directive-ta_instruction_handler
	jne	ta_find_end_directive
      ta_structure_end:
	pop	[ta_error_line]
	ret
      ta_no_end_directive:
	mov	eax,[ta_error_line]
	mov	[ta_current_line],eax
	jmp	ta_missing_end_directive
      ta_skip_repeat:
	call	ta_find_end_repeat
	jmp	ta_find_end_directive
      ta_skip_while:
	call	ta_find_end_while
	jmp	ta_find_end_directive
      ta_skip_if:
	call	ta_skip_if_block
	jmp	ta_find_end_directive
      ta_skip_if_block:
	call	ta_find_else
	jc	ta_if_block_skipped
	cmp	byte [esi],1
	jne	ta_skip_after_else
	cmp	word [esi+1],ta_if_directive-ta_instruction_handler
	jne	ta_skip_after_else
	add	esi,4
	jmp	ta_skip_if_block
      ta_skip_after_else:
	call	ta_find_end_if
      ta_if_block_skipped:
	ret
ta_end_directive:
	lods	byte [esi]
	cmp	al,1
	jne	ta_invalid_argument
	lods	word [esi]
	inc	esi
	cmp	ax,ta_virtual_directive-ta_instruction_handler
	je	ta_end_virtual
	cmp	ax,ta_repeat_directive-ta_instruction_handler
	je	ta_end_repeat
	cmp	ax,ta_while_directive-ta_instruction_handler
	je	ta_end_while
	cmp	ax,ta_if_directive-ta_instruction_handler
	je	ta_end_if
	cmp	ax,ta_data_directive-ta_instruction_handler
	je	ta_end_data
	jmp	ta_invalid_argument
ta_break_directive:
	mov	ebx,[ta_structures_buffer]
	promote_ebx
	mov	al,[esi]
	or	al,al
	jz	ta_find_breakable_structure
	cmp	al,0Fh
	jne	ta_extra_characters_on_line
      ta_find_breakable_structure:
	cmp	ebx,[ta_additional_memory_end]
	je	ta_unexpected_instruction
	mov	ax,[ebx]
	cmp	ax,ta_repeat_directive-ta_instruction_handler
	je	ta_break_repeat
	cmp	ax,ta_while_directive-ta_instruction_handler
	je	ta_break_while
	cmp	ax,ta_if_directive-ta_instruction_handler
	je	ta_break_if
	add	ebx,18h
	jmp	ta_find_breakable_structure
      ta_break_if:
	push	[ta_current_line]
	mov	eax,[ebx+4]
	mov	[ta_current_line],eax
	call	ta_remove_structure_data
	call	ta_skip_if_block
	pop	[ta_current_line]
	mov	ebx,[ta_structures_buffer]
	promote_ebx
	jmp	ta_find_breakable_structure
      ta_break_repeat:
	push	ebx
	call	ta_find_end_repeat
	pop	ebx
	jmp	ta_stop_repeat
      ta_break_while:
	push	ebx
	jmp	ta_stop_while

ta_define_data:
	cmp	edi,[ta_tagged_blocks]
	jae	ta_out_of_memory
	cmp	byte [esi],'('
	jne	ta_simple_data_value
	mov	ebx,esi
	inc	esi
	call	ta_skip_expression
	xchg	esi,ebx
	cmp	byte [ebx],81h
	jne	ta_simple_data_value
	inc	esi
	call	ta_get_count_value
	inc	esi
	or	eax,eax
	jz	ta_duplicate_zero_times
	cmp	byte [esi],91h
	jne	ta_duplicate_single_data_value
	inc	esi
      ta_duplicate_data:
	push	eax esi
      ta_duplicated_values:
	cmp	edi,[ta_tagged_blocks]
	jae	ta_out_of_memory
	clc
	call	near dword [esp+8]
	lods	byte [esi]
	cmp	al,','
	je	ta_duplicated_values
	cmp	al,92h
	jne	ta_invalid_argument
	pop	ebx eax
	dec	eax
	jz	ta_data_defined
	mov	esi,ebx
	jmp	ta_duplicate_data
      ta_duplicate_single_data_value:
	cmp	edi,[ta_tagged_blocks]
	jae	ta_out_of_memory
	push	eax esi
	clc
	call	near dword [esp+8]
	pop	ebx eax
	dec	eax
	jz	ta_data_defined
	mov	esi,ebx
	jmp	ta_duplicate_single_data_value
      ta_duplicate_zero_times:
	cmp	byte [esi],91h
	jne	ta_skip_single_data_value
	inc	esi
      ta_skip_data_value:
	call	ta_skip_symbol
	jc	ta_invalid_argument
	cmp	byte [esi],92h
	jne	ta_skip_data_value
	inc	esi
	jmp	ta_data_defined
      ta_skip_single_data_value:
	call	ta_skip_symbol
	jmp	ta_data_defined
      ta_simple_data_value:
	cmp	edi,[ta_tagged_blocks]
	jae	ta_out_of_memory
	clc
	call	near dword [esp]
      ta_data_defined:
	lods	byte [esi]
	cmp	al,','
	je	ta_define_data
	dec	esi
	stc
	ret
ta_data_bytes:
	call	ta_define_data
	jc	ta_instruction_assembled
	lods	byte [esi]
	cmp	al,'('
	je	ta_get_byte
	cmp	al,'?'
	jne	ta_invalid_argument
	mov	eax,edi
	mov	byte [edi],0
	inc	edi
	jmp	ta_undefined_data
      ta_get_byte:
	cmp	byte [esi],0
	je	ta_get_string
	call	ta_get_byte_value
	stos	byte [edi]
	ret
      ta_get_string:
	inc	esi
	lods	dword [esi]
	mov	ecx,eax
	lea	eax,[edi+ecx]
	cmp	eax,[ta_tagged_blocks]
	ja	ta_out_of_memory
	rep	movs byte [edi],[esi]
	inc	esi
	ret
      ta_undefined_data:
	mov	ebp,[ta_addressing_space]
	test	byte [ds:ebp+0Ah],1
	jz	ta_mark_undefined_data
	ret
      ta_mark_undefined_data:
	cmp	eax,[ta_undefined_data_end]
	je	ta_undefined_data_ok
	mov	[ta_undefined_data_start],eax
      ta_undefined_data_ok:
	mov	[ta_undefined_data_end],edi
	ret
ta_data_unicode:
	or	[ta_base_code],-1
	jmp	ta_define_words
ta_data_words:
	mov	[ta_base_code],0
    ta_define_words:
	call	ta_define_data
	jc	ta_instruction_assembled
	lods	byte [esi]
	cmp	al,'('
	je	ta_get_word
	cmp	al,'?'
	jne	ta_invalid_argument
	mov	eax,edi
	and	word [edi],0
	scas	word [edi]
	jmp	ta_undefined_data
	ret
      ta_get_word:
	cmp	[ta_base_code],0
	je	ta_word_data_value
	cmp	byte [esi],0
	je	ta_word_string
      ta_word_data_value:
	call	ta_get_word_value
	call	ta_mark_relocation
	stos	word [edi]
	ret
      ta_word_string:
	inc	esi
	lods	dword [esi]
	mov	ecx,eax
	jecxz	ta_word_string_ok
	lea	eax,[edi+ecx*2]
	cmp	eax,[ta_tagged_blocks]
	ja	ta_out_of_memory
	xor	ah,ah
      ta_copy_word_string:
	lods	byte [esi]
	stos	word [edi]
	loop	ta_copy_word_string
      ta_word_string_ok:
	inc	esi
	ret
ta_data_dwords:
	call	ta_define_data
	jc	ta_instruction_assembled
	lods	byte [esi]
	cmp	al,'('
	je	ta_get_dword
	cmp	al,'?'
	jne	ta_invalid_argument
	mov	eax,edi
	and	dword [edi],0
	scas	dword [edi]
	jmp	ta_undefined_data
      ta_get_dword:
	push	esi
	call	ta_get_dword_value
	pop	ebx
	cmp	byte [esi],':'
	je	ta_complex_dword
	call	ta_mark_relocation
	stos	dword [edi]
	ret
      ta_complex_dword:
	mov	esi,ebx
	cmp	byte [esi],'.'
	je	ta_invalid_value
	call	ta_get_word_value
	push	eax
	inc	esi
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_operand
	mov	al,[ta_value_type]
	push	eax
	cmp	byte [esi],'.'
	je	ta_invalid_value
	call	ta_get_word_value
	call	ta_mark_relocation
	stos	word [edi]
	pop	eax
	mov	[ta_value_type],al
	pop	eax
	call	ta_mark_relocation
	stos	word [edi]
	ret
ta_data_pwords:
	call	ta_define_data
	jc	ta_instruction_assembled
	lods	byte [esi]
	cmp	al,'('
	je	ta_get_pword
	cmp	al,'?'
	jne	ta_invalid_argument
	mov	eax,edi
	and	dword [edi],0
	scas	dword [edi]
	and	word [edi],0
	scas	word [edi]
	jmp	ta_undefined_data
      ta_get_pword:
	push	esi
	call	ta_get_pword_value
	pop	ebx
	cmp	byte [esi],':'
	je	ta_complex_pword
	call	ta_mark_relocation
	stos	dword [edi]
	mov	ax,dx
	stos	word [edi]
	ret
      ta_complex_pword:
	mov	esi,ebx
	cmp	byte [esi],'.'
	je	ta_invalid_value
	call	ta_get_word_value
	push	eax
	inc	esi
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_operand
	mov	al,[ta_value_type]
	push	eax
	cmp	byte [esi],'.'
	je	ta_invalid_value
	call	ta_get_dword_value
	call	ta_mark_relocation
	stos	dword [edi]
	pop	eax
	mov	[ta_value_type],al
	pop	eax
	call	ta_mark_relocation
	stos	word [edi]
	ret
ta_data_qwords:
	call	ta_define_data
	jc	ta_instruction_assembled
	lods	byte [esi]
	cmp	al,'('
	je	ta_get_qword
	cmp	al,'?'
	jne	ta_invalid_argument
	mov	eax,edi
	and	dword [edi],0
	scas	dword [edi]
	and	dword [edi],0
	scas	dword [edi]
	jmp	ta_undefined_data
      ta_get_qword:
	call	ta_get_qword_value
	call	ta_mark_relocation
	stos	dword [edi]
	mov	eax,edx
	stos	dword [edi]
	ret
ta_data_twords:
	call	ta_define_data
	jc	ta_instruction_assembled
	lods	byte [esi]
	cmp	al,'('
	je	ta_get_tword
	cmp	al,'?'
	jne	ta_invalid_argument
	mov	eax,edi
	and	dword [edi],0
	scas	dword [edi]
	and	dword [edi],0
	scas	dword [edi]
	and	word [edi],0
	scas	word [edi]
	jmp	ta_undefined_data
      ta_get_tword:
	cmp	byte [esi],'.'
	jne	ta_complex_tword
	inc	esi
	cmp	word [esi+8],8000h
	je	ta_fp_zero_tword
	mov	eax,[esi]
	stos	dword [edi]
	mov	eax,[esi+4]
	stos	dword [edi]
	mov	ax,[esi+8]
	add	ax,3FFFh
	jo	ta_value_out_of_range
	cmp	ax,7FFFh
	jge	ta_value_out_of_range
	cmp	ax,0
	jg	ta_tword_exp_ok
	mov	cx,ax
	neg	cx
	inc	cx
	cmp	cx,64
	jae	ta_value_out_of_range
	cmp	cx,32
	ja	ta_large_shift
	mov	eax,[esi]
	mov	edx,[esi+4]
	mov	ebx,edx
	shr	edx,cl
	shrd	eax,ebx,cl
	jmp	ta_tword_mantissa_shift_done
      ta_large_shift:
	sub	cx,32
	xor	edx,edx
	mov	eax,[esi+4]
	shr	eax,cl
      ta_tword_mantissa_shift_done:
	jnc	ta_store_shifted_mantissa
	add	eax,1
	adc	edx,0
      ta_store_shifted_mantissa:
	mov	[edi-8],eax
	mov	[edi-4],edx
	xor	ax,ax
	test	edx,1 shl 31
	jz	ta_tword_exp_ok
	inc	ax
      ta_tword_exp_ok:
	mov	bl,[esi+11]
	shl	bx,15
	or	ax,bx
	stos	word [edi]
	add	esi,13
	ret
      ta_fp_zero_tword:
	xor	eax,eax
	stos	dword [edi]
	stos	dword [edi]
	mov	al,[esi+11]
	shl	ax,15
	stos	word [edi]
	add	esi,13
	ret
      ta_complex_tword:
	call	ta_get_word_value
	push	eax
	cmp	byte [esi],':'
	jne	ta_invalid_operand
	inc	esi
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_operand
	mov	al,[ta_value_type]
	push	eax
	cmp	byte [esi],'.'
	je	ta_invalid_value
	call	ta_get_qword_value
	call	ta_mark_relocation
	stos	dword [edi]
	mov	eax,edx
	stos	dword [edi]
	pop	eax
	mov	[ta_value_type],al
	pop	eax
	call	ta_mark_relocation
	stos	word [edi]
	ret
ta_data_file:
	lods	word [esi]
	cmp	ax,'('
	jne	ta_invalid_argument
	add	esi,4
	call	ta_open_binary_file
	mov	eax,[esi-4]
	lea	esi,[esi+eax+1]
	mov	al,2
	xor	edx,edx
	call	ta_lseek
	push	eax
	xor	edx,edx
	cmp	byte [esi],':'
	jne	ta_position_ok
	inc	esi
	cmp	byte [esi],'('
	jne	ta_invalid_argument
	inc	esi
	cmp	byte [esi],'.'
	je	ta_invalid_value
	push	ebx
	call	ta_get_count_value
	pop	ebx
	mov	edx,eax
	sub	[esp],edx
	jc	ta_value_out_of_range
      ta_position_ok:
	cmp	byte [esi],','
	jne	ta_size_ok
	inc	esi
	cmp	byte [esi],'('
	jne	ta_invalid_argument
	inc	esi
	cmp	byte [esi],'.'
	je	ta_invalid_value
	push	ebx edx
	call	ta_get_count_value
	pop	edx ebx
	cmp	eax,[esp]
	ja	ta_value_out_of_range
	mov	[esp],eax
      ta_size_ok:
	xor	al,al
	call	ta_lseek
	pop	ecx
	mov	edx,edi
	add	edi,ecx
	jc	ta_out_of_memory
	cmp	edi,[ta_tagged_blocks]
	ja	ta_out_of_memory
	call	ta_read
	jc	ta_error_reading_file
	call	ta_close
	lods	byte [esi]
	cmp	al,','
	je	ta_data_file
	dec	esi
	jmp	ta_instruction_assembled
      ta_open_binary_file:
	push	esi
	push	edi
	mov	eax,[ta_current_line]
      ta_find_current_source_path: 
	mov	esi,[eax] 
	test	byte [eax+7],80h 
	jz	ta_get_current_path 
	mov	eax,[eax+8]
	jmp	ta_find_current_source_path
      ta_get_current_path:
	lodsb
	stosb
	or	al,al
	jnz	ta_get_current_path
      ta_cut_current_path:
	cmp	edi,[esp]
	je	ta_current_path_ok
	cmp	byte [edi-1],'\'
	je	ta_current_path_ok
	cmp	byte [edi-1],'/'
	je	ta_current_path_ok
	dec	edi
	jmp	ta_cut_current_path
      ta_current_path_ok:
	mov	esi,[esp+4]
	call	ta_expand_path
	pop	edx
	mov	esi,edx
	call	ta_open
	jnc	ta_file_opened
	mov	edx,[ta_include_paths]
	promote_edx
      ta_search_in_include_paths:
	push	edx esi
	mov	edi,esi
	mov	esi,[esp+4]
	call	ta_get_include_directory
	mov	[esp+4],esi
	mov	esi,[esp+8]
	call	ta_expand_path
	pop	edx
	mov	esi,edx
	call	ta_open
	pop	edx
	jnc	ta_file_opened
	cmp	byte [edx],0
	jne	ta_search_in_include_paths
	mov	edi,esi
	mov	esi,[esp]
	push	edi
	call	ta_expand_path
	pop	edx
	mov	esi,edx
	call	ta_open
	jc	ta_file_not_found
      ta_file_opened:
	mov	edi,esi
	pop	esi
	ret
ta_reserve_bytes:
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_argument
	cmp	byte [esi],'.'
	je	ta_invalid_value
	call	ta_get_count_value
	mov	ecx,eax
	mov	edx,ecx
	add	edx,edi
	jc	ta_out_of_memory
	cmp	edx,[ta_tagged_blocks]
	ja	ta_out_of_memory
	push	edi
	cmp	[ta_next_pass_needed],0
	je	ta_zero_bytes
	add	edi,ecx
	jmp	ta_reserved_data
      ta_zero_bytes:
	xor	eax,eax
	shr	ecx,1
	jnc	ta_bytes_stosb_ok
	stos	byte [edi]
      ta_bytes_stosb_ok:
	shr	ecx,1
	jnc	ta_bytes_stosw_ok
	stos	word [edi]
      ta_bytes_stosw_ok:
	rep	stos dword [edi]
      ta_reserved_data:
	pop	eax
	call	ta_undefined_data
	jmp	ta_instruction_assembled
ta_reserve_words:
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_argument
	cmp	byte [esi],'.'
	je	ta_invalid_value
	call	ta_get_count_value
	mov	ecx,eax
	mov	edx,ecx
	shl	edx,1
	jc	ta_out_of_memory
	add	edx,edi
	jc	ta_out_of_memory
	cmp	edx,[ta_tagged_blocks]
	ja	ta_out_of_memory
	push	edi
	cmp	[ta_next_pass_needed],0
	je	ta_zero_words
	lea	edi,[edi+ecx*2]
	jmp	ta_reserved_data
      ta_zero_words:
	xor	eax,eax
	shr	ecx,1
	jnc	ta_words_stosw_ok
	stos	word [edi]
      ta_words_stosw_ok:
	rep	stos dword [edi]
	jmp	ta_reserved_data
ta_reserve_dwords:
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_argument
	cmp	byte [esi],'.'
	je	ta_invalid_value
	call	ta_get_count_value
	mov	ecx,eax
	mov	edx,ecx
	shl	edx,1
	jc	ta_out_of_memory
	shl	edx,1
	jc	ta_out_of_memory
	add	edx,edi
	jc	ta_out_of_memory
	cmp	edx,[ta_tagged_blocks]
	ja	ta_out_of_memory
	push	edi
	cmp	[ta_next_pass_needed],0
	je	ta_zero_dwords
	lea	edi,[edi+ecx*4]
	jmp	ta_reserved_data
      ta_zero_dwords:
	xor	eax,eax
	rep	stos dword [edi]
	jmp	ta_reserved_data
ta_reserve_pwords:
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_argument
	cmp	byte [esi],'.'
	je	ta_invalid_value
	call	ta_get_count_value
	mov	ecx,eax
	shl	ecx,1
	jc	ta_out_of_memory
	add	ecx,eax
	mov	edx,ecx
	shl	edx,1
	jc	ta_out_of_memory
	add	edx,edi
	jc	ta_out_of_memory
	cmp	edx,[ta_tagged_blocks]
	ja	ta_out_of_memory
	push	edi
	cmp	[ta_next_pass_needed],0
	je	ta_zero_words
	lea	edi,[edi+ecx*2]
	jmp	ta_reserved_data
ta_reserve_qwords:
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_argument
	cmp	byte [esi],'.'
	je	ta_invalid_value
	call	ta_get_count_value
	mov	ecx,eax
	shl	ecx,1
	jc	ta_out_of_memory
	mov	edx,ecx
	shl	edx,1
	jc	ta_out_of_memory
	shl	edx,1
	jc	ta_out_of_memory
	add	edx,edi
	jc	ta_out_of_memory
	cmp	edx,[ta_tagged_blocks]
	ja	ta_out_of_memory
	push	edi
	cmp	[ta_next_pass_needed],0
	je	ta_zero_dwords
	lea	edi,[edi+ecx*4]
	jmp	ta_reserved_data
ta_reserve_twords:
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_argument
	cmp	byte [esi],'.'
	je	ta_invalid_value
	call	ta_get_count_value
	mov	ecx,eax
	shl	ecx,2
	jc	ta_out_of_memory
	add	ecx,eax
	mov	edx,ecx
	shl	edx,1
	jc	ta_out_of_memory
	add	edx,edi
	jc	ta_out_of_memory
	cmp	edx,[ta_tagged_blocks]
	ja	ta_out_of_memory
	push	edi
	cmp	[ta_next_pass_needed],0
	je	ta_zero_words
	lea	edi,[edi+ecx*2]
	jmp	ta_reserved_data
ta_align_directive:
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_argument
	cmp	byte [esi],'.'
	je	ta_invalid_value
	call	ta_get_count_value
	mov	edx,eax
	dec	edx
	test	eax,edx
	jnz	ta_invalid_align_value
	or	eax,eax
	jz	ta_invalid_align_value
	cmp	eax,1
	je	ta_instruction_assembled
	mov	ecx,edi
	mov	ebp,[ta_addressing_space]
	sub	ecx,[ds:ebp]
	cmp	dword [ds:ebp+10h],0
	jne	ta_section_not_aligned_enough
	cmp	byte [ds:ebp+9],0
	je	ta_make_alignment
	cmp	[ta_output_format],3
	je	ta_pe_alignment
	cmp	[ta_output_format],5
	jne	ta_object_alignment
	test	[ta_format_flags],1
	jnz	ta_pe_alignment
      ta_object_alignment:
	mov	ebx,[ds:ebp+14h]
	cmp	byte [ebx],0
	jne	ta_section_not_aligned_enough
	cmp	eax,[ebx+10h]
	jbe	ta_make_alignment
	jmp	ta_section_not_aligned_enough
      ta_pe_alignment:
	cmp	eax,1000h
	ja	ta_section_not_aligned_enough
      ta_make_alignment:
	dec	eax
	and	ecx,eax
	jz	ta_instruction_assembled
	neg	ecx
	add	ecx,eax
	inc	ecx
	mov	edx,ecx
	add	edx,edi
	jc	ta_out_of_memory
	cmp	edx,[ta_tagged_blocks]
	ja	ta_out_of_memory
	push	edi
	cmp	[ta_next_pass_needed],0
	je	ta_nops
	add	edi,ecx
	jmp	ta_reserved_data
      ta_invalid_align_value:
	cmp	[ta_error_line],0
	jne	ta_instruction_assembled
	mov	eax,[ta_current_line]
	mov	[ta_error_line],eax
	mov	[ta_error],ta_invalid_value
	jmp	ta_instruction_assembled
      ta_nops:
	mov	eax,90909090h
	shr	ecx,1
	jnc	ta_nops_stosb_ok
	stos	byte [edi]
      ta_nops_stosb_ok:
	shr	ecx,1
	jnc	ta_nops_stosw_ok
	stos	word [edi]
      ta_nops_stosw_ok:
	rep	stos dword [edi]
	jmp	ta_reserved_data
ta_err_directive:
	mov	al,[esi]
	cmp	al,0Fh
	je	ta_invoked_error
	or	al,al
	jz	ta_invoked_error
	jmp	ta_extra_characters_on_line
ta_assert_directive:
	call	ta_calculate_logical_expression
	or	al,al
	jnz	ta_instruction_assembled
	cmp	[ta_error_line],0
	jne	ta_instruction_assembled
	mov	eax,[ta_current_line]
	mov	[ta_error_line],eax
	mov	[ta_error],ta_assertion_failed
	jmp	ta_instruction_assembled
