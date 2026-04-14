ta_calculate_expression:
	mov	[ta_current_offset],edi
	mov	[ta_value_undefined],0
	cmp	byte [esi],0
	je	ta_get_string_value
	cmp	byte [esi],'.'
	je	ta_convert_fp
      ta_calculation_loop:
	mov	eax,[ta_tagged_blocks]
	sub	eax,0Ch
	cmp	eax,edi
	jbe	ta_out_of_memory
	lods	byte [esi]
	cmp	al,1
	je	ta_get_byte_number
	cmp	al,2
	je	ta_get_word_number
	cmp	al,4
	je	ta_get_dword_number
	cmp	al,8
	je	ta_get_qword_number
	cmp	al,0Fh
	je	ta_value_out_of_range
	cmp	al,10h
	je	ta_get_register
	cmp	al,11h
	je	ta_get_label
	cmp	al,')'
	je	ta_expression_calculated
	cmp	al,']'
	je	ta_expression_calculated
	cmp	al,'!'
	je	ta_invalid_expression
	sub	edi,14h
	mov	ebx,edi
	sub	ebx,14h
	cmp	al,0F0h
	je	ta_calculate_rva
	cmp	al,0F1h
	je	ta_calculate_plt
	cmp	al,0D0h
	je	ta_calculate_not
	cmp	al,0E0h
	je	ta_calculate_bsf
	cmp	al,0E1h
	je	ta_calculate_bsr
	cmp	al,083h
	je	ta_calculate_neg
	mov	dx,[ebx+8]
	or	dx,[edi+8]
	cmp	al,80h
	je	ta_calculate_add
	cmp	al,81h
	je	ta_calculate_sub
	mov	ah,[ebx+12]
	or	ah,[edi+12]
	jz	ta_absolute_values_calculation
	call	ta_recoverable_misuse
      ta_absolute_values_calculation:
	cmp	al,90h
	je	ta_calculate_mul
	cmp	al,91h
	je	ta_calculate_div
	or	dx,dx
	jnz	ta_invalid_expression
	cmp	al,0A0h
	je	ta_calculate_mod
	cmp	al,0B0h
	je	ta_calculate_and
	cmp	al,0B1h
	je	ta_calculate_or
	cmp	al,0B2h
	je	ta_calculate_xor
	cmp	al,0C0h
	je	ta_calculate_shl
	cmp	al,0C1h
	je	ta_calculate_shr
	jmp	ta_invalid_expression
      ta_expression_calculated:
	sub	edi,14h
	cmp	[ta_value_undefined],0
	je	ta_expression_value_ok
	xor	eax,eax
	mov	[edi],eax
	mov	[edi+4],eax
	mov	[edi+12],eax
      ta_expression_value_ok:
	ret
      ta_get_byte_number:
	xor	eax,eax
	lods	byte [esi]
	stos	dword [edi]
	xor	al,al
	stos	dword [edi]
      ta_got_number:
	and	word [edi-8+8],0
	and	word [edi-8+12],0
	and	dword [edi-8+16],0
	add	edi,0Ch
	jmp	ta_calculation_loop
      ta_get_word_number:
	xor	eax,eax
	lods	word [esi]
	stos	dword [edi]
	xor	ax,ax
	stos	dword [edi]
	jmp	ta_got_number
      ta_get_dword_number:
	movs	dword [edi],[esi]
	xor	eax,eax
	stos	dword [edi]
	jmp	ta_got_number
      ta_get_qword_number:
	movs	dword [edi],[esi]
	movs	dword [edi],[esi]
	jmp	ta_got_number
      ta_get_register:
	mov	byte [edi+9],0
	and	word [edi+12],0
	lods	byte [esi]
	mov	[edi+8],al
	mov	byte [edi+10],1
	xor	eax,eax
	mov	[edi+16],eax
	stos	dword [edi]
	stos	dword [edi]
	add	edi,0Ch
	jmp	ta_calculation_loop
      ta_get_label:
	xor	eax,eax
	mov	[edi+8],eax
	mov	[edi+12],eax
	mov	[edi+20],eax
	lods	dword [esi]
	cmp	eax,0Fh
	jb	ta_predefined_label
	je	ta_reserved_word_used_as_symbol
	mov	ebx,eax
	mov	ax,[ta_current_pass]
	mov	[ebx+18],ax
	mov	cl,[ebx+9]
	shr	cl,1
	and	cl,1
	neg	cl
	or	byte [ebx+8],8
	test	byte [ebx+8],1
	jz	ta_label_undefined
	cmp	ax,[ebx+16]
	je	ta_unadjusted_label
	test	byte [ebx+8],4
	jnz	ta_label_out_of_scope
	test	byte [ebx+9],1
	jz	ta_unadjusted_label
	mov	eax,[ebx]
	sub	eax,dword [ta_adjustment]
	stos	dword [edi]
	mov	eax,[ebx+4]
	sbb	eax,dword [ta_adjustment+4]
	stos	dword [edi]
	sbb	cl,[ta_adjustment_sign]
	mov	[edi-8+13],cl
	mov	eax,dword [ta_adjustment]
	or	al,[ta_adjustment_sign]
	or	eax,dword [ta_adjustment+4]
	jz	ta_got_label
	or	[ta_next_pass_needed],-1
	jmp	ta_got_label
      ta_unadjusted_label:
	mov	eax,[ebx]
	stos	dword [edi]
	mov	eax,[ebx+4]
	stos	dword [edi]
	mov	[edi-8+13],cl
      ta_got_label:
	test	byte [ebx+9],4
	jnz	ta_invalid_use_of_symbol
	call	ta_store_label_reference
	mov	al,[ebx+11]
	mov	[edi-8+12],al
	mov	eax,[ebx+12]
	mov	[edi-8+8],eax
	cmp	al,ah
	jne	ta_labeled_registers_ok
	shr	eax,16
	add	al,ah
	jo	ta_labeled_registers_ok
	xor	ah,ah
	mov	[edi-8+10],ax
	mov	[edi-8+9],ah
      ta_labeled_registers_ok:
	mov	eax,[ebx+20]
	mov	[edi-8+16],eax
	add	edi,0Ch
	mov	al,[ebx+10]
	or	al,al
	jz	ta_calculation_loop
	test	[ta_operand_flags],1
	jnz	ta_calculation_loop
      ta_check_size:
	xchg	[ta_operand_size],al
	or	al,al
	jz	ta_calculation_loop
	cmp	al,[ta_operand_size]
	jne	ta_operand_sizes_do_not_match
	jmp	ta_calculation_loop
      ta_actual_file_offset_label:
	mov	eax,[ta_undefined_data_end]
	mov	ebp,[ta_addressing_space]
	test	byte [ds:ebp+0Ah],1
	jnz	ta_use_undefined_data_offset
	cmp	eax,[ta_current_offset]
	jne	ta_use_current_offset
       ta_use_undefined_data_offset:
	mov	eax,[ta_undefined_data_start]
	jmp	ta_make_file_offset_label
      ta_current_file_offset_label:
	mov	ebp,[ta_addressing_space]
	test	byte [ds:ebp+0Ah],1
	jz	ta_use_current_offset
	mov	eax,[ta_undefined_data_end]
	jmp	ta_make_file_offset_label
       ta_use_current_offset:
	mov	eax,[ta_current_offset]
       ta_make_file_offset_label:
	cmp	[ta_output_format],2
	jae	ta_invalid_use_of_symbol
	sub	eax,[ta_code_start]
	jmp	ta_make_dword_label_value
      ta_current_offset_label:
	mov	eax,[ta_current_offset]
       ta_make_current_offset_label:
	xor	edx,edx
	xor	ch,ch
	mov	ebp,[ta_addressing_space]
	sub	eax,[ds:ebp]
	sbb	edx,[ds:ebp+4]
	sbb	ch,[ds:ebp+8]
	jp	ta_current_offset_label_ok
	call	ta_recoverable_overflow
       ta_current_offset_label_ok:
	stos	dword [edi]
	mov	eax,edx
	stos	dword [edi]
	mov	eax,[ds:ebp+10h]
	stos	dword [edi]
	mov	cl,[ds:ebp+9]
	mov	[edi-12+12],cx
	mov	eax,[ds:ebp+14h]
	mov	[edi-12+16],eax
	add	edi,8
	jmp	ta_calculation_loop
      ta_org_origin_label:
	mov	eax,[ta_addressing_space]
	mov	eax,[eax+18h]
	jmp	ta_make_current_offset_label
      ta_counter_label:
	mov	eax,[ta_counter]
      ta_make_dword_label_value:
	stos	dword [edi]
	xor	eax,eax
	stos	dword [edi]
	add	edi,0Ch
	jmp	ta_calculation_loop
      ta_timestamp_label:
	call	ta_make_timestamp
      ta_make_qword_label_value:
	stos	dword [edi]
	mov	eax,edx
	stos	dword [edi]
	add	edi,0Ch
	jmp	ta_calculation_loop
      ta_predefined_label:
	or	eax,eax
	jz	ta_current_offset_label
	cmp	eax,1
	je	ta_counter_label
	cmp	eax,2
	je	ta_timestamp_label
	cmp	eax,3
	je	ta_org_origin_label
	cmp	eax,4
	je	ta_current_file_offset_label
	cmp	eax,5
	je	ta_actual_file_offset_label
	mov	edx,ta_invalid_value
	jmp	ta_error_undefined
      ta_label_out_of_scope:
	mov	edx,ta_symbol_out_of_scope
	jmp	ta_error_undefined
      ta_label_undefined:
	mov	edx,ta_undefined_symbol
      ta_error_undefined:
	cmp	[ta_current_pass],1
	ja	ta_undefined_value
      ta_force_next_pass:
	or	[ta_next_pass_needed],-1
      ta_undefined_value:
	or	[ta_value_undefined],-1
	and	word [edi+12],0
	xor	eax,eax
	stos	dword [edi]
	stos	dword [edi]
	add	edi,0Ch
	cmp	[ta_error_line],0
	jne	ta_calculation_loop
	mov	eax,[ta_current_line]
	mov	[ta_error_line],eax
	mov	[ta_error],edx
	mov	[ta_error_info],ebx
	jmp	ta_calculation_loop
      ta_calculate_add:
	xor	ah,ah
	mov	ah,[ebx+12]
	mov	al,[edi+12]
	or	al,al
	jz	ta_add_values
	or	ah,ah
	jz	ta_add_relocatable
	add	ah,al
	jnz	ta_invalid_add
	mov	ecx,[edi+16]
	cmp	ecx,[ebx+16]
	je	ta_add_values
      ta_invalid_add:
	call	ta_recoverable_misuse
	jmp	ta_add_values
      ta_add_relocatable:
	mov	ah,al
	mov	ecx,[edi+16]
	mov	[ebx+16],ecx
      ta_add_values:
	mov	[ebx+12],ah
	mov	eax,[edi]
	add	[ebx],eax
	mov	eax,[edi+4]
	adc	[ebx+4],eax
	mov	al,[edi+13]
	adc	[ebx+13],al
	jp	ta_add_sign_ok
	call	ta_recoverable_overflow
      ta_add_sign_ok:
	or	dx,dx
	jz	ta_calculation_loop
	push	esi
	mov	esi,ebx
	mov	cl,[edi+10]
	mov	al,[edi+8]
	call	ta_add_register
	mov	cl,[edi+11]
	mov	al,[edi+9]
	call	ta_add_register
	pop	esi
	jmp	ta_calculation_loop
      ta_add_register:
	or	al,al
	jz	ta_add_register_done
      ta_add_register_start:
	cmp	[esi+8],al
	jne	ta_add_in_second_slot
	add	[esi+10],cl
	jo	ta_value_out_of_range
	jnz	ta_add_register_done
	mov	byte [esi+8],0
	ret
      ta_add_in_second_slot:
	cmp	[esi+9],al
	jne	ta_create_in_first_slot
	add	[esi+11],cl
	jo	ta_value_out_of_range
	jnz	ta_add_register_done
	mov	byte [esi+9],0
	ret
      ta_create_in_first_slot:
	cmp	byte [esi+8],0
	jne	ta_create_in_second_slot
	mov	[esi+8],al
	mov	[esi+10],cl
	ret
      ta_create_in_second_slot:
	cmp	byte [esi+9],0
	jne	ta_invalid_expression
	mov	[esi+9],al
	mov	[esi+11],cl
      ta_add_register_done:
	ret
      ta_out_of_range:
	jmp	ta_calculation_loop
      ta_calculate_sub:
	xor	ah,ah
	mov	ah,[ebx+12]
	mov	al,[edi+12]
	or	al,al
	jz	ta_sub_values
	or	ah,ah
	jz	ta_negate_relocatable
	cmp	al,ah
	jne	ta_invalid_sub
	xor	ah,ah
	mov	ecx,[edi+16]
	cmp	ecx,[ebx+16]
	je	ta_sub_values
      ta_invalid_sub:
	call	ta_recoverable_misuse
	jmp	ta_sub_values
      ta_negate_relocatable:
	neg	al
	mov	ah,al
	mov	ecx,[edi+16]
	mov	[ebx+16],ecx
      ta_sub_values:
	mov	[ebx+12],ah
	mov	eax,[edi]
	sub	[ebx],eax
	mov	eax,[edi+4]
	sbb	[ebx+4],eax
	mov	al,[edi+13]
	sbb	[ebx+13],al
	jp	ta_sub_sign_ok
	cmp	[ta_error_line],0
	jne	ta_sub_sign_ok
	call	ta_recoverable_overflow
      ta_sub_sign_ok:
	or	dx,dx
	jz	ta_calculation_loop
	push	esi
	mov	esi,ebx
	mov	cl,[edi+10]
	mov	al,[edi+8]
	call	ta_sub_register
	mov	cl,[edi+11]
	mov	al,[edi+9]
	call	ta_sub_register
	pop	esi
	jmp	ta_calculation_loop
      ta_sub_register:
	or	al,al
	jz	ta_add_register_done
	neg	cl
	jo	ta_value_out_of_range
	jmp	ta_add_register_start
      ta_calculate_mul:
	or	dx,dx
	jz	ta_mul_start
	cmp	word [ebx+8],0
	jne	ta_mul_start
	xor	ecx,ecx
      ta_swap_values:
	mov	eax,[ebx+ecx]
	xchg	eax,[edi+ecx]
	mov	[ebx+ecx],eax
	add	ecx,4
	cmp	ecx,16
	jb	ta_swap_values
      ta_mul_start:
	push	esi edx
	mov	esi,ebx
	xor	bl,bl
	cmp	byte [esi+13],0
	je	ta_mul_first_sign_ok
	xor	bl,-1
	mov	eax,[esi]
	mov	edx,[esi+4]
	not	eax
	not	edx
	add	eax,1
	adc	edx,0
	mov	[esi],eax
	mov	[esi+4],edx
	or	eax,edx
	jz	ta_mul_overflow
      ta_mul_first_sign_ok:
	cmp	byte [edi+13],0
	je	ta_mul_second_sign_ok
	xor	bl,-1
	cmp	byte [esi+8],0
	je	ta_mul_first_register_sign_ok
	neg	byte [esi+10]
	jo	ta_invalid_expression
      ta_mul_first_register_sign_ok:
	cmp	byte [esi+9],0
	je	ta_mul_second_register_sign_ok
	neg	byte [esi+11]
	jo	ta_invalid_expression
      ta_mul_second_register_sign_ok:
	mov	eax,[edi]
	mov	edx,[edi+4]
	not	eax
	not	edx
	add	eax,1
	adc	edx,0
	mov	[edi],eax
	mov	[edi+4],edx
	or	eax,edx
	jz	ta_mul_overflow
      ta_mul_second_sign_ok:
	cmp	dword [esi+4],0
	jz	ta_mul_numbers
	cmp	dword [edi+4],0
	jz	ta_mul_numbers
	jnz	ta_mul_overflow
      ta_mul_numbers:
	mov	eax,[esi+4]
	mul	dword [edi]
	or	edx,edx
	jnz	ta_mul_overflow
	mov	ecx,eax
	mov	eax,[esi]
	mul	dword [edi+4]
	or	edx,edx
	jnz	ta_mul_overflow
	add	ecx,eax
	jc	ta_mul_overflow
	mov	eax,[esi]
	mul	dword [edi]
	add	edx,ecx
	jc	ta_mul_overflow
	mov	[esi],eax
	mov	[esi+4],edx
	or	bl,bl
	jz	ta_mul_ok
	not	eax
	not	edx
	add	eax,1
	adc	edx,0
	mov	[esi],eax
	mov	[esi+4],edx
	or	eax,edx
	jnz	ta_mul_ok
	not	bl
      ta_mul_ok:
	mov	[esi+13],bl
	pop	edx
	or	dx,dx
	jz	ta_mul_calculated
	cmp	word [edi+8],0
	jne	ta_invalid_value
	cmp	byte [esi+8],0
	je	ta_mul_first_register_ok
	call	ta_get_byte_scale
	imul	byte [esi+10]
	mov	dl,ah
	cbw
	cmp	ah,dl
	jne	ta_value_out_of_range
	mov	[esi+10],al
	or	al,al
	jnz	ta_mul_first_register_ok
	mov	[esi+8],al
      ta_mul_first_register_ok:
	cmp	byte [esi+9],0
	je	ta_mul_calculated
	call	ta_get_byte_scale
	imul	byte [esi+11]
	mov	dl,ah
	cbw
	cmp	ah,dl
	jne	ta_value_out_of_range
	mov	[esi+11],al
	or	al,al
	jnz	ta_mul_calculated
	mov	[esi+9],al
      ta_mul_calculated:
	pop	esi
	jmp	ta_calculation_loop
      ta_mul_overflow:
	pop	edx esi
	call	ta_recoverable_overflow
	jmp	ta_calculation_loop
      ta_get_byte_scale:
	mov	al,[edi]
	cbw
	cwde
	cdq
	cmp	edx,[edi+4]
	jne	ta_value_out_of_range
	cmp	eax,[edi]
	jne	ta_value_out_of_range
	ret
      ta_calculate_div:
	push	esi edx
	mov	esi,ebx
	call	ta_div_64
	pop	edx
	or	dx,dx
	jz	ta_div_calculated
	cmp	byte [esi+8],0
	je	ta_div_first_register_ok
	call	ta_get_byte_scale
	or	al,al
	jz	ta_value_out_of_range
	mov	al,[esi+10]
	cbw
	idiv	byte [edi]
	or	ah,ah
	jnz	ta_invalid_use_of_symbol
	mov	[esi+10],al
      ta_div_first_register_ok:
	cmp	byte [esi+9],0
	je	ta_div_calculated
	call	ta_get_byte_scale
	or	al,al
	jz	ta_value_out_of_range
	mov	al,[esi+11]
	cbw
	idiv	byte [edi]
	or	ah,ah
	jnz	ta_invalid_use_of_symbol
	mov	[esi+11],al
      ta_div_calculated:
	pop	esi
	jmp	ta_calculation_loop
      ta_calculate_mod:
	push	esi
	mov	esi,ebx
	call	ta_div_64
	mov	[esi],eax
	mov	[esi+4],edx
	mov	[esi+13],bh
	pop	esi
	jmp	ta_calculation_loop
      ta_calculate_and:
	mov	eax,[edi]
	mov	edx,[edi+4]
	mov	cl,[edi+13]
	and	[ebx],eax
	and	[ebx+4],edx
	and	[ebx+13],cl
	jmp	ta_calculation_loop
      ta_calculate_or:
	mov	eax,[edi]
	mov	edx,[edi+4]
	mov	cl,[edi+13]
	or	[ebx],eax
	or	[ebx+4],edx
	or	[ebx+13],cl
	jmp	ta_calculation_loop
      ta_calculate_xor:
	mov	eax,[edi]
	mov	edx,[edi+4]
	mov	cl,[edi+13]
	xor	[ebx],eax
	xor	[ebx+4],edx
	xor	[ebx+13],cl
	jmp	ta_calculation_loop
      ta_shr_negative:
	mov	byte [edi+13],0
	not	dword [edi]
	not	dword [edi+4]
	add	dword [edi],1
	adc	dword [edi+4],0
	jc	ta_shl_over
      ta_calculate_shl:
	cmp	byte [edi+13],0
	jne	ta_shl_negative
	mov	edx,[ebx+4]
	mov	eax,[ebx]
	cmp	dword [edi+4],0
	jne	ta_shl_over
	movsx	ecx,byte [ebx+13]
	xchg	ecx,[edi]
	cmp	ecx,64
	je	ta_shl_max
	ja	ta_shl_over
	cmp	ecx,32
	jae	ta_shl_high
	shld	[edi],edx,cl
	shld	edx,eax,cl
	shl	eax,cl
	mov	[ebx],eax
	mov	[ebx+4],edx
	jmp	ta_shl_done
      ta_shl_over:
	cmp	byte [ebx+13],0
	jne	ta_shl_overflow
      ta_shl_max:
	movsx	ecx,byte [ebx+13]
	cmp	eax,ecx
	jne	ta_shl_overflow
	cmp	edx,ecx
	jne	ta_shl_overflow
	xor	eax,eax
	mov	[ebx],eax
	mov	[ebx+4],eax
	jmp	ta_calculation_loop
      ta_shl_high:
	sub	cl,32
	shld	[edi],edx,cl
	shld	edx,eax,cl
	shl	eax,cl
	mov	[ebx+4],eax
	and	dword [ebx],0
	cmp	edx,[edi]
	jne	ta_shl_overflow
      ta_shl_done:
	movsx	eax,byte [ebx+13]
	cmp	eax,[edi]
	je	ta_calculation_loop
      ta_shl_overflow:
	call	ta_recoverable_overflow
	jmp	ta_calculation_loop
      ta_shl_negative:
	mov	byte [edi+13],0
	not	dword [edi]
	not	dword [edi+4]
	add	dword [edi],1
	adc	dword [edi+4],0
	jnc	ta_calculate_shr
	dec	dword [edi+4]
      ta_calculate_shr:
	cmp	byte [edi+13],0
	jne	ta_shr_negative
	mov	edx,[ebx+4]
	mov	eax,[ebx]
	cmp	dword [edi+4],0
	jne	ta_shr_over
	mov	ecx,[edi]
	cmp	ecx,64
	jae	ta_shr_over
	push	esi
	movsx	esi,byte [ebx+13]
	cmp	ecx,32
	jae	ta_shr_high
	shrd	eax,edx,cl
	shrd	edx,esi,cl
	mov	[ebx],eax
	mov	[ebx+4],edx
	pop	esi
	jmp	ta_calculation_loop
      ta_shr_high:
	sub	cl,32
	shrd	edx,esi,cl
	mov	[ebx],edx
	mov	[ebx+4],esi
	pop	esi
	jmp	ta_calculation_loop
      ta_shr_over:
	movsx	eax,byte [ebx+13]
	mov	dword [ebx],eax
	mov	dword [ebx+4],eax
	jmp	ta_calculation_loop
      ta_calculate_not:
	cmp	word [edi+8],0
	jne	ta_invalid_expression
	cmp	byte [edi+12],0
	je	ta_not_ok
	call	ta_recoverable_misuse
      ta_not_ok:
	not	dword [edi]
	not	dword [edi+4]
	not	byte [edi+13]
	add	edi,14h
	jmp	ta_calculation_loop
      ta_calculate_bsf:
	cmp	word [edi+8],0
	jne	ta_invalid_expression
	cmp	byte [edi+12],0
	je	ta_bsf_ok
	call	ta_recoverable_misuse
      ta_bsf_ok:
	xor	ecx,ecx
	bsf	eax,[edi]
	jnz	ta_finish_bs
	mov	ecx,32
	bsf	eax,[edi+4]
	jnz	ta_finish_bs
	cmp	byte [edi+13],0
	jne	ta_finish_bs
      ta_bs_overflow:
	call	ta_recoverable_overflow
	add	edi,14h
	jmp	ta_calculation_loop
      ta_calculate_bsr:
	cmp	word [edi+8],0
	jne	ta_invalid_expression
	cmp	byte [edi+12],0
	je	ta_bsr_ok
	call	ta_recoverable_misuse
      ta_bsr_ok:
	cmp	byte [edi+13],0
	jne	ta_bs_overflow
	mov	ecx,32
	bsr	eax,[edi+4]
	jnz	ta_finish_bs
	xor	ecx,ecx
	bsr	eax,[edi]
	jz	ta_bs_overflow
      ta_finish_bs:
	add	eax,ecx
	xor	edx,edx
	mov	[edi],eax
	mov	[edi+4],edx
	mov	[edi+13],dl
	add	edi,14h
	jmp	ta_calculation_loop
      ta_calculate_neg:
	cmp	byte [edi+8],0
	je	ta_neg_first_register_ok
	neg	byte [edi+10]
	jo	ta_invalid_expression
      ta_neg_first_register_ok:
	cmp	byte [edi+9],0
	je	ta_neg_second_register_ok
	neg	byte [edi+11]
	jo	ta_invalid_expression
      ta_neg_second_register_ok:
	neg	byte [edi+12]
	xor	eax,eax
	xor	edx,edx
	xor	cl,cl
	xchg	eax,[edi]
	xchg	edx,[edi+4]
	xchg	cl,[edi+13]
	sub	[edi],eax
	sbb	[edi+4],edx
	sbb	[edi+13],cl
	jp	ta_neg_sign_ok
	call	ta_recoverable_overflow
      ta_neg_sign_ok:
	add	edi,14h
	jmp	ta_calculation_loop
      ta_calculate_rva:
	cmp	word [edi+8],0
	jne	ta_invalid_expression
	mov	al,[ta_output_format]
	cmp	al,5
	je	ta_calculate_gotoff
	cmp	al,4
	je	ta_calculate_coff_rva
	cmp	al,3
	jne	ta_invalid_expression
	test	[ta_format_flags],8
	jnz	ta_pe64_rva
	mov	al,2
	bt	[ta_resolver_flags],0
	jc	ta_rva_type_ok
	xor	al,al
      ta_rva_type_ok:
	cmp	byte [edi+12],al
	je	ta_rva_ok
	call	ta_recoverable_misuse
      ta_rva_ok:
	mov	byte [edi+12],0
	mov	eax,[ta_code_start]
	mov	eax,[eax+34h]
	xor	edx,edx
      ta_finish_rva:
	sub	[edi],eax
	sbb	[edi+4],edx
	sbb	byte [edi+13],0
	jp	ta_rva_finished
	call	ta_recoverable_overflow
      ta_rva_finished:
	add	edi,14h
	jmp	ta_calculation_loop
      ta_pe64_rva:
	mov	al,4
	bt	[ta_resolver_flags],0
	jc	ta_pe64_rva_type_ok
	xor	al,al
      ta_pe64_rva_type_ok:
	cmp	byte [edi+12],al
	je	ta_pe64_rva_ok
	call	ta_recoverable_misuse
      ta_pe64_rva_ok:
	mov	byte [edi+12],0
	mov	eax,[ta_code_start]
	mov	edx,[eax+34h]
	mov	eax,[eax+30h]
	jmp	ta_finish_rva
      ta_calculate_gotoff:
	test	[ta_format_flags],1
	jnz	ta_calculate_elf_dyn_rva
	test	[ta_format_flags],8
	jnz	ta_invalid_expression
      ta_calculate_coff_rva:
	mov	dl,5
	cmp	byte [edi+12],2
	je	ta_change_value_type
      ta_incorrect_change_of_value_type:
	call	ta_recoverable_misuse
      ta_change_value_type:
	mov	byte [edi+12],dl
	add	edi,14h
	jmp	ta_calculation_loop
      ta_calculate_elf_dyn_rva:
	xor	dl,dl
	test	byte [edi+12],1
	jnz	ta_incorrect_change_of_value_type
	jmp	ta_change_value_type
      ta_calculate_plt:
	cmp	word [edi+8],0
	jne	ta_invalid_expression
	cmp	[ta_output_format],5
	jne	ta_invalid_expression
	test	[ta_format_flags],1
	jnz	ta_invalid_expression
	mov	dl,6
	mov	dh,2
	test	[ta_format_flags],8
	jz	ta_check_value_for_plt
	mov	dh,4
      ta_check_value_for_plt:
	mov	eax,[edi]
	or	eax,[edi+4]
	jnz	ta_incorrect_change_of_value_type
	cmp	byte [edi+12],dh
	jne	ta_incorrect_change_of_value_type
	mov	eax,[edi+16]
	cmp	byte [eax],80h
	jne	ta_incorrect_change_of_value_type
	jmp	ta_change_value_type
      ta_div_64:
	xor	ebx,ebx
	cmp	dword [edi],0
	jne	ta_divider_ok
	cmp	dword [edi+4],0
	jne	ta_divider_ok
	cmp	[ta_next_pass_needed],0
	je	ta_value_out_of_range
	jmp	ta_div_done
      ta_divider_ok:
	cmp	byte [esi+13],0
	je	ta_div_first_sign_ok
	mov	eax,[esi]
	mov	edx,[esi+4]
	not	eax
	not	edx
	add	eax,1
	adc	edx,0
	mov	[esi],eax
	mov	[esi+4],edx
	or	eax,edx
	jz	ta_value_out_of_range
	xor	bx,-1
      ta_div_first_sign_ok:
	cmp	byte [edi+13],0
	je	ta_div_second_sign_ok
	mov	eax,[edi]
	mov	edx,[edi+4]
	not	eax
	not	edx
	add	eax,1
	adc	edx,0
	mov	[edi],eax
	mov	[edi+4],edx
	or	eax,edx
	jz	ta_value_out_of_range
	xor	bl,-1
      ta_div_second_sign_ok:
	cmp	dword [edi+4],0
	jne	ta_div_high
	mov	ecx,[edi]
	mov	eax,[esi+4]
	xor	edx,edx
	div	ecx
	mov	[esi+4],eax
	mov	eax,[esi]
	div	ecx
	mov	[esi],eax
	mov	eax,edx
	xor	edx,edx
	jmp	ta_div_done
      ta_div_high:
	push	ebx
	mov	eax,[esi+4]
	xor	edx,edx
	div	dword [edi+4]
	mov	ebx,[esi]
	mov	[esi],eax
	and	dword [esi+4],0
	mov	ecx,edx
	mul	dword [edi]
      ta_div_high_loop:
	cmp	ecx,edx
	ja	ta_div_high_done
	jb	ta_div_high_large_correction
	cmp	ebx,eax
	jae	ta_div_high_done
      ta_div_high_correction:
	dec	dword [esi]
	sub	eax,[edi]
	sbb	edx,[edi+4]
	jnc	ta_div_high_loop
      ta_div_high_done:
	sub	ebx,eax
	sbb	ecx,edx
	mov	edx,ecx
	mov	eax,ebx
	pop	ebx
	jmp	ta_div_done
      ta_div_high_large_correction:
	push	eax edx
	mov	eax,edx
	sub	eax,ecx
	xor	edx,edx
	div	dword [edi+4]
	shr	eax,1
	jz	ta_div_high_small_correction
	sub	[esi],eax
	push	eax
	mul	dword [edi+4]
	sub	dword [esp+4],eax
	pop	eax
	mul	dword [edi]
	sub	dword [esp+4],eax
	sbb	dword [esp],edx
	pop	edx eax
	jmp	ta_div_high_loop
      ta_div_high_small_correction:
	pop	edx eax
	jmp	ta_div_high_correction
      ta_div_done:
	or	bh,bh
	jz	ta_remainder_ok
	not	eax
	not	edx
	add	eax,1
	adc	edx,0
	mov	ecx,eax
	or	ecx,edx
	jnz	ta_remainder_ok
	not	bh
      ta_remainder_ok:
	or	bl,bl
	jz	ta_div_ok
	not	dword [esi]
	not	dword [esi+4]
	add	dword [esi],1
	adc	dword [esi+4],0
	mov	ecx,[esi]
	or	ecx,[esi+4]
	jnz	ta_div_ok
	not	bl
      ta_div_ok:
	mov	[esi+13],bl
	ret
      ta_store_label_reference:
	cmp	[ta_symbols_file],0
	je	ta_label_reference_ok
	cmp	[ta_next_pass_needed],0
	jne	ta_label_reference_ok
	mov	eax,[ta_tagged_blocks]
	mov	dword [eax-4],2
	mov	dword [eax-8],4
	sub	eax,8+4
	cmp	eax,edi
	jbe	ta_out_of_memory
	mov	[ta_tagged_blocks],eax
	mov	[eax],ebx
      ta_label_reference_ok:
	ret
      ta_convert_fp:
	inc	esi
	and	word [edi+8],0
	and	word [edi+12],0
	mov	al,[ta_value_size]
	cmp	al,2
	je	ta_convert_fp_word
	cmp	al,4
	je	ta_convert_fp_dword
	test	al,not 8
	jz	ta_convert_fp_qword
	call	ta_recoverable_misuse
      ta_convert_fp_qword:
	xor	eax,eax
	xor	edx,edx
	cmp	word [esi+8],8000h
	je	ta_fp_qword_store
	mov	bx,[esi+8]
	mov	eax,[esi]
	mov	edx,[esi+4]
	add	eax,eax
	adc	edx,edx
	mov	ecx,edx
	shr	edx,12
	shrd	eax,ecx,12
	jnc	ta_fp_qword_ok
	add	eax,1
	adc	edx,0
	bt	edx,20
	jnc	ta_fp_qword_ok
	and	edx,1 shl 20 - 1
	inc	bx
	shr	edx,1
	rcr	eax,1
      ta_fp_qword_ok:
	add	bx,3FFh
	cmp	bx,7FFh
	jge	ta_value_out_of_range
	cmp	bx,0
	jg	ta_fp_qword_exp_ok
	or	edx,1 shl 20
	mov	cx,bx
	neg	cx
	inc	cx
	cmp	cx,52+1
	ja	ta_value_out_of_range
	cmp	cx,32
	jb	ta_fp_qword_small_shift
	sub	cx,32
	mov	eax,edx
	xor	edx,edx
	shr	eax,cl
	jmp	ta_fp_qword_shift_done
      ta_fp_qword_small_shift:
	mov	ebx,edx
	shr	edx,cl
	shrd	eax,ebx,cl
      ta_fp_qword_shift_done:
	mov	bx,0
	jnc	ta_fp_qword_exp_ok
	add	eax,1
	adc	edx,0
	test	edx,1 shl 20
	jz	ta_fp_qword_exp_ok
	and	edx,1 shl 20 - 1
	inc	bx
      ta_fp_qword_exp_ok:
	shl	ebx,20
	or	edx,ebx
	jnz	ta_fp_qword_store
	or	eax,eax
	jz	ta_value_out_of_range
      ta_fp_qword_store:
	mov	bl,[esi+11]
	shl	ebx,31
	or	edx,ebx
	mov	[edi],eax
	mov	[edi+4],edx
	add	esi,13
	ret
      ta_convert_fp_word:
	xor	eax,eax
	cmp	word [esi+8],8000h
	je	ta_fp_word_store
	mov	bx,[esi+8]
	mov	ax,[esi+6]
	shl	ax,1
	shr	ax,6
	jnc	ta_fp_word_ok
	inc	ax
	bt	ax,10
	jnc	ta_fp_word_ok
	and	ax,1 shl 10 - 1
	inc	bx
	shr	ax,1
      ta_fp_word_ok:
	add	bx,0Fh
	cmp	bx,01Fh
	jge	ta_value_out_of_range
	cmp	bx,0
	jg	ta_fp_word_exp_ok
	or	ax,1 shl 10
	mov	cx,bx
	neg	cx
	inc	cx
	cmp	cx,10+1
	ja	ta_value_out_of_range
	xor	bx,bx
	shr	ax,cl
	jnc	ta_fp_word_exp_ok
	inc	ax
	test	ax,1 shl 10
	jz	ta_fp_word_exp_ok
	and	ax,1 shl 10 - 1
	inc	bx
      ta_fp_word_exp_ok:
	shl	bx,10
	or	ax,bx
	jz	ta_value_out_of_range
      ta_fp_word_store:
	mov	bl,[esi+11]
	shl	bx,15
	or	ax,bx
	mov	[edi],eax
	xor	eax,eax
	mov	[edi+4],eax
	add	esi,13
	ret
      ta_convert_fp_dword:
	xor	eax,eax
	cmp	word [esi+8],8000h
	je	ta_fp_dword_store
	mov	bx,[esi+8]
	mov	eax,[esi+4]
	shl	eax,1
	shr	eax,9
	jnc	ta_fp_dword_ok
	inc	eax
	bt	eax,23
	jnc	ta_fp_dword_ok
	and	eax,1 shl 23 - 1
	inc	bx
	shr	eax,1
      ta_fp_dword_ok:
	add	bx,7Fh
	cmp	bx,0FFh
	jge	ta_value_out_of_range
	cmp	bx,0
	jg	ta_fp_dword_exp_ok
	or	eax,1 shl 23
	mov	cx,bx
	neg	cx
	inc	cx
	cmp	cx,23+1
	ja	ta_value_out_of_range
	xor	bx,bx
	shr	eax,cl
	jnc	ta_fp_dword_exp_ok
	inc	eax
	test	eax,1 shl 23
	jz	ta_fp_dword_exp_ok
	and	eax,1 shl 23 - 1
	inc	bx
      ta_fp_dword_exp_ok:
	shl	ebx,23
	or	eax,ebx
	jz	ta_value_out_of_range
      ta_fp_dword_store:
	mov	bl,[esi+11]
	shl	ebx,31
	or	eax,ebx
	mov	[edi],eax
	xor	eax,eax
	mov	[edi+4],eax
	add	esi,13
	ret
      ta_get_string_value:
	inc	esi
	lods	dword [esi]
	mov	ecx,eax
	cmp	ecx,8
	ja	ta_value_out_of_range
	mov	edx,edi
	xor	eax,eax
	stos	dword [edi]
	stos	dword [edi]
	mov	edi,edx
	rep	movs byte [edi],[esi]
	mov	edi,edx
	inc	esi
	and	word [edi+8],0
	and	word [edi+12],0
	ret

ta_get_byte_value:
	mov	[ta_value_size],1
	or	[ta_operand_flags],1
	call	ta_calculate_value
	or	al,al
	jz	ta_check_byte_value
	call	ta_recoverable_misuse
      ta_check_byte_value:
	mov	eax,[edi]
	mov	edx,[edi+4]
	cmp	byte [edi+13],0
	je	ta_byte_positive
	cmp	edx,-1
	jne	ta_range_exceeded
	cmp	eax,-100h
	jb	ta_range_exceeded
	ret
      ta_byte_positive:
	test	edx,edx
	jnz	ta_range_exceeded
	cmp	eax,100h
	jae	ta_range_exceeded
      ta_return_byte_value:
	ret
      ta_range_exceeded:
	xor	eax,eax
	xor	edx,edx
      ta_recoverable_overflow:
	cmp	[ta_error_line],0
	jne	ta_ignore_overflow
	push	[ta_current_line]
	pop	[ta_error_line]
	mov	[ta_error],ta_value_out_of_range
	or	[ta_value_undefined],-1
      ta_ignore_overflow:
	ret
      ta_recoverable_misuse:
	cmp	[ta_error_line],0
	jne	ta_ignore_misuse
	push	[ta_current_line]
	pop	[ta_error_line]
	mov	[ta_error],ta_invalid_use_of_symbol
      ta_ignore_misuse:
	ret
ta_get_word_value:
	mov	[ta_value_size],2
	or	[ta_operand_flags],1
	call	ta_calculate_value
	cmp	al,2
	jb	ta_check_word_value
	call	ta_recoverable_misuse
      ta_check_word_value:
	mov	eax,[edi]
	mov	edx,[edi+4]
	cmp	byte [edi+13],0
	je	ta_word_positive
	cmp	edx,-1
	jne	ta_range_exceeded
	cmp	eax,-10000h
	jb	ta_range_exceeded
	ret
      ta_word_positive:
	test	edx,edx
	jnz	ta_range_exceeded
	cmp	eax,10000h
	jae	ta_range_exceeded
	ret
ta_get_dword_value:
	mov	[ta_value_size],4
	or	[ta_operand_flags],1
	call	ta_calculate_value
	cmp	al,4
	jne	ta_check_dword_value
	mov	[ta_value_type],2
	mov	eax,[edi]
	cdq
	cmp	edx,[edi+4]
	jne	ta_range_exceeded
	mov	ecx,edx
	sar	ecx,31
	cmp	cl,[ta_value_sign]
	jne	ta_range_exceeded
	ret
      ta_check_dword_value:
	mov	eax,[edi]
	mov	edx,[edi+4]
	cmp	byte [edi+13],0
	je	ta_dword_positive
	cmp	edx,-1
	jne	ta_range_exceeded
	ret
      ta_dword_positive:
	test	edx,edx
	jne	ta_range_exceeded
	ret
ta_get_pword_value:
	mov	[ta_value_size],6
	or	[ta_operand_flags],1
	call	ta_calculate_value
	cmp	al,4
	jne	ta_check_pword_value
	call	ta_recoverable_misuse
      ta_check_pword_value:
	mov	eax,[edi]
	mov	edx,[edi+4]
	cmp	byte [edi+13],0
	je	ta_pword_positive
	cmp	edx,-10000h
	jb	ta_range_exceeded
	ret
      ta_pword_positive:
	cmp	edx,10000h
	jae	ta_range_exceeded
	ret
ta_get_qword_value:
	mov	[ta_value_size],8
	or	[ta_operand_flags],1
	call	ta_calculate_value
      ta_check_qword_value:
	mov	eax,[edi]
	mov	edx,[edi+4]
	ret
ta_get_count_value:
	mov	[ta_value_size],8
	or	[ta_operand_flags],1
	call	ta_calculate_expression
	cmp	word [edi+8],0
	jne	ta_invalid_value
	mov	[ta_value_sign],0
	mov	al,[edi+12]
	or	al,al
	jz	ta_check_count_value
	call	ta_recoverable_misuse
      ta_check_count_value:
	cmp	byte [edi+13],0
	jne	ta_invalid_count_value
	mov	eax,[edi]
	mov	edx,[edi+4]
	or	edx,edx
	jnz	ta_invalid_count_value
	ret
      ta_invalid_count_value:
	cmp	[ta_error_line],0
	jne	ta_zero_count
	mov	eax,[ta_current_line]
	mov	[ta_error_line],eax
	mov	[ta_error],ta_invalid_value
      ta_zero_count:
	xor	eax,eax
	ret
ta_get_value:
	mov	[ta_operand_size],0
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'('
	jne	ta_invalid_value
	mov	al,[ta_operand_size]
	cmp	al,1
	je	ta_value_byte
	cmp	al,2
	je	ta_value_word
	cmp	al,4
	je	ta_value_dword
	cmp	al,6
	je	ta_value_pword
	cmp	al,8
	je	ta_value_qword
	or	al,al
	jnz	ta_invalid_value
	mov	[ta_value_size],al
	call	ta_calculate_value
	mov	eax,[edi]
	mov	edx,[edi+4]
	ret
      ta_calculate_value:
	call	ta_calculate_expression
	cmp	word [edi+8],0
	jne	ta_invalid_value
	mov	eax,[edi+16]
	mov	[ta_symbol_identifier],eax
	mov	al,[edi+13]
	mov	[ta_value_sign],al
	mov	al,[edi+12]
	mov	[ta_value_type],al
	ret
      ta_value_qword:
	call	ta_get_qword_value
      ta_truncated_value:
	mov	[ta_value_sign],0
	ret
      ta_value_pword:
	call	ta_get_pword_value
	movzx	edx,dx
	jmp	ta_truncated_value
      ta_value_dword:
	call	ta_get_dword_value
	xor	edx,edx
	jmp	ta_truncated_value
      ta_value_word:
	call	ta_get_word_value
	xor	edx,edx
	movzx	eax,ax
	jmp	ta_truncated_value
      ta_value_byte:
	call	ta_get_byte_value
	xor	edx,edx
	movzx	eax,al
	jmp	ta_truncated_value
ta_get_address_word_value:
	mov	[ta_address_size],2
	mov	[ta_value_size],2
	mov	[ta_free_address_range],0
	jmp	ta_calculate_address
ta_get_address_dword_value:
	mov	[ta_address_size],4
	mov	[ta_value_size],4
	mov	[ta_free_address_range],0
	jmp	ta_calculate_address
ta_get_address_qword_value:
	mov	[ta_address_size],8
	mov	[ta_value_size],8
	mov	[ta_free_address_range],0
	jmp	ta_calculate_address
ta_get_address_value:
	mov	[ta_address_size],0
	mov	[ta_value_size],8
	or	[ta_free_address_range],-1
      ta_calculate_address:
	cmp	byte [esi],'.'
	je	ta_invalid_address
	call	ta_calculate_expression
	mov	eax,[edi+16]
	mov	[ta_address_symbol],eax
	mov	al,[edi+13]
	mov	[ta_address_sign],al
	mov	al,[edi+12]
	mov	[ta_value_type],al
	cmp	al,0
	je	ta_address_size_ok
	jg	ta_get_address_symbol_size
	neg	al
      ta_get_address_symbol_size:
	cmp	al,6
	je	ta_special_address_type_32bit
	cmp	al,5
	je	ta_special_address_type_32bit
	ja	ta_invalid_address_type
	test	al,1
	jnz	ta_invalid_address_type
	shl	al,5
	jmp	ta_address_symbol_ok
      ta_invalid_address_type:
	call	ta_recoverable_misuse
      ta_special_address_type_32bit:
	mov	al,40h
      ta_address_symbol_ok:
	mov	ah,[ta_address_size]
	or	[ta_address_size],al
	shr	al,4
	or	ah,ah
	jz	ta_address_size_ok
	cmp	al,ah
	je	ta_address_size_ok
	cmp	ax,0408h
	je	ta_address_sizes_mixed
	cmp	ax,0804h
	jne	ta_address_sizes_do_not_agree
      ta_address_sizes_mixed:
	cmp	[ta_value_type],4
	jne	ta_address_sizes_mixed_type_ok
	mov	[ta_value_type],2
      ta_address_sizes_mixed_type_ok:
	mov	eax,[edi]
	cdq
	cmp	edx,[edi+4]
	je	ta_address_size_ok
	cmp	[ta_error_line],0
	jne	ta_address_size_ok
	call	ta_recoverable_overflow
      ta_address_size_ok:
	xor	ebx,ebx
	xor	ecx,ecx
	mov	cl,[ta_value_type]
	shl	ecx,16
	mov	ch,[ta_address_size]
	cmp	word [edi+8],0
	je	ta_check_immediate_address
	mov	al,[edi+8]
	mov	dl,[edi+10]
	call	ta_get_address_register
	mov	al,[edi+9]
	mov	dl,[edi+11]
	call	ta_get_address_register
	mov	ax,bx
	shr	ah,4
	shr	al,4
	or	bh,bh
	jz	ta_check_address_registers
	or	bl,bl
	jz	ta_check_address_registers
	cmp	al,ah
	jne	ta_check_vsib
      ta_check_address_registers:
	or	al,ah
	cmp	al,0Ch
	jae	ta_check_vsib
	cmp	al,6
	je	ta_check_vsib
	cmp	al,7
	je	ta_check_vsib
	mov	ah,[ta_address_size]
	and	ah,0Fh
	jz	ta_address_registers_sizes_ok
	cmp	al,ah
	jne	ta_invalid_address
      ta_address_registers_sizes_ok:
	cmp	al,4
	je	ta_sib_allowed
	cmp	al,8
	je	ta_sib_allowed
	cmp	al,9
	je	ta_check_ip_relative_address
	cmp	cl,1
	ja	ta_invalid_address
	cmp	[ta_free_address_range],0
	jne	ta_check_qword_value
	jmp	ta_check_word_value
      ta_address_sizes_do_not_match:
	cmp	al,0Fh
	jne	ta_invalid_address
	mov	al,bh
	and	al,0Fh
	cmp	al,ah
	jne	ta_invalid_address
      ta_check_ip_relative_address:
	or	bl,bl
	jnz	ta_invalid_address
	cmp	bh,98h
	je	ta_check_rip_relative_address
	cmp	bh,94h
	jne	ta_invalid_address
	cmp	[ta_free_address_range],0
	je	ta_check_dword_value
	mov	eax,[edi]
	mov	edx,[edi+4]
	ret
      ta_check_rip_relative_address:
	mov	eax,[edi]
	cdq
	cmp	edx,[edi+4]
	jne	ta_range_exceeded
	cmp	dl,[edi+13]
	jne	ta_range_exceeded
	ret
      ta_get_address_register:
	or	al,al
	jz	ta_address_register_ok
	cmp	dl,1
	jne	ta_scaled_register
	or	bh,bh
	jnz	ta_scaled_register
	mov	bh,al
      ta_address_register_ok:
	ret
      ta_scaled_register:
	or	bl,bl
	jnz	ta_invalid_address
	mov	bl,al
	mov	cl,dl
	jmp	ta_address_register_ok
      ta_sib_allowed:
	or	bh,bh
	jnz	ta_check_index_with_base
	cmp	cl,3
	je	ta_special_index_scale
	cmp	cl,5
	je	ta_special_index_scale
	cmp	cl,9
	je	ta_special_index_scale
	cmp	cl,2
	jne	ta_check_index_scale
	cmp	bl,45h
	jne	ta_special_index_scale
	cmp	[ta_code_type],64
	je	ta_special_index_scale
	cmp	[ta_segment_register],4
	jne	ta_special_index_scale
	cmp	[ta_value_type],0
	jne	ta_check_index_scale
	mov	al,[edi]
	cbw
	cwde
	cmp	eax,[edi]
	jne	ta_check_index_scale
	cdq
	cmp	edx,[edi+4]
	jne	ta_check_immediate_address
      ta_special_index_scale:
	mov	bh,bl
	dec	cl
      ta_check_immediate_address:
	cmp	[ta_free_address_range],0
	jne	ta_check_qword_value
	mov	al,[ta_address_size]
	and	al,0Fh
	cmp	al,2
	je	ta_check_word_value
	cmp	al,4
	je	ta_check_dword_value
	cmp	al,8
	je	ta_check_qword_value
	or	al,al
	jnz	ta_invalid_value
	cmp	[ta_code_type],64
	jne	ta_check_dword_value
	jmp	ta_check_qword_value
      ta_check_index_with_base:
	cmp	cl,1
	jne	ta_check_index_scale
	cmp	bl,44h
	je	ta_swap_base_with_index
	cmp	bl,84h
	je	ta_swap_base_with_index
	cmp	[ta_code_type],64
	je	ta_check_for_rbp_base
	cmp	bl,45h
	jne	ta_check_for_ebp_base
	cmp	[ta_segment_register],3
	je	ta_swap_base_with_index
	jmp	ta_check_immediate_address
      ta_check_for_ebp_base:
	cmp	bh,45h
	jne	ta_check_immediate_address
	cmp	[ta_segment_register],4
	jne	ta_check_immediate_address
      ta_swap_base_with_index:
	xchg	bl,bh
	jmp	ta_check_immediate_address
      ta_check_for_rbp_base:
	cmp	bh,45h
	je	ta_swap_base_with_index
	cmp	bh,85h
	je	ta_swap_base_with_index
	jmp	ta_check_immediate_address
      ta_check_index_scale:
	test	cl,not 1111b
	jnz	ta_invalid_address
	mov	al,cl
	dec	al
	and	al,cl
	jz	ta_check_immediate_address
	jmp	ta_invalid_address
      ta_check_vsib:
	xor	ah,ah
      ta_check_vsib_base:
	test	bh,bh
	jz	ta_check_vsib_index
	mov	al,bh
	shr	al,4
	cmp	al,4
	je	ta_check_vsib_base_size
	cmp	[ta_code_type],64
	jne	ta_swap_vsib_registers
	cmp	al,8
	jne	ta_swap_vsib_registers
      ta_check_vsib_base_size:
	mov	ah,[ta_address_size]
	and	ah,0Fh
	jz	ta_check_vsib_index
	cmp	al,ah
	jne	ta_invalid_address
      ta_check_vsib_index:
	mov	al,bl
	and	al,0E0h
	cmp	al,0C0h
	jae	ta_check_index_scale
	cmp	al,60h
	je	ta_check_index_scale
	jmp	ta_invalid_address
      ta_swap_vsib_registers:
	xor	ah,-1
	jz	ta_invalid_address
	cmp	cl,1
	ja	ta_invalid_address
	xchg	bl,bh
	mov	cl,1
	jmp	ta_check_vsib_base

ta_calculate_relative_offset:
	cmp	[ta_value_undefined],0
	jne	ta_relative_offset_ok
	test	bh,bh
	setne	ch
	cmp	bx,[ds:ebp+10h]
	je	ta_origin_registers_ok
	xchg	bh,bl
	xchg	ch,cl
	cmp	bx,[ds:ebp+10h]
	jne	ta_invalid_value
      ta_origin_registers_ok:
	cmp	cx,[ds:ebp+10h+2]
	jne	ta_invalid_value
	mov	bl,[ta_address_sign]
	add	eax,[ds:ebp]
	adc	edx,[ds:ebp+4]
	adc	bl,[ds:ebp+8]
	sub	eax,edi
	sbb	edx,0
	sbb	bl,0
	mov	[ta_value_sign],bl
	mov	bl,[ta_value_type]
	mov	ecx,[ta_address_symbol]
	mov	[ta_symbol_identifier],ecx
	test	bl,1
	jnz	ta_relative_offset_unallowed
	cmp	bl,6
	je	ta_plt_relative_offset
	mov	bh,[ds:ebp+9]
	cmp	bl,bh
	je	ta_set_relative_offset_type
	cmp	bx,0402h
	je	ta_set_relative_offset_type
      ta_relative_offset_unallowed:
	call	ta_recoverable_misuse
      ta_set_relative_offset_type:
	cmp	[ta_value_type],0
	je	ta_relative_offset_ok
	mov	[ta_value_type],0
	cmp	ecx,[ds:ebp+14h]
	je	ta_relative_offset_ok
	mov	[ta_value_type],3
      ta_relative_offset_ok:
	ret
      ta_plt_relative_offset:
	mov	[ta_value_type],7
	cmp	byte [ds:ebp+9],2
	je	ta_relative_offset_ok
	cmp	byte [ds:ebp+9],4
	jne	ta_recoverable_misuse
	ret

ta_calculate_logical_expression:
	xor	al,al
  ta_calculate_embedded_logical_expression:
	mov	[ta_logical_value_wrapping],al
	call	ta_get_logical_value
      ta_logical_loop:
	cmp	byte [esi],'|'
	je	ta_logical_or
	cmp	byte [esi],'&'
	je	ta_logical_and
	ret
      ta_logical_or:
	inc	esi
	or	al,al
	jnz	ta_logical_value_already_determined
	push	eax
	call	ta_get_logical_value
	pop	ebx
	or	al,bl
	jmp	ta_logical_loop
      ta_logical_and:
	inc	esi
	or	al,al
	jz	ta_logical_value_already_determined
	push	eax
	call	ta_get_logical_value
	pop	ebx
	and	al,bl
	jmp	ta_logical_loop
      ta_logical_value_already_determined:
	push	eax
	call	ta_skip_logical_value
	jc	ta_invalid_expression
	pop	eax
	jmp	ta_logical_loop
  ta_get_value_for_comparison:
	mov	[ta_value_size],8
	or	[ta_operand_flags],1
	lods	byte [esi]
	call	ta_calculate_expression
	cmp	byte [edi+8],0
	jne	ta_first_register_size_ok
	mov	byte [edi+10],0
      ta_first_register_size_ok:
	cmp	byte [edi+9],0
	jne	ta_second_register_size_ok
	mov	byte [edi+11],0
      ta_second_register_size_ok:
	mov	eax,[edi+16]
	mov	[ta_symbol_identifier],eax
	mov	al,[edi+13]
	mov	[ta_value_sign],al
	mov	bl,[edi+12]
	mov	eax,[edi]
	mov	edx,[edi+4]
	mov	ecx,[edi+8]
	ret
  ta_get_logical_value:
	xor	al,al
      ta_check_for_negation:
	cmp	byte [esi],'~'
	jne	ta_negation_ok
	inc	esi
	xor	al,-1
	jmp	ta_check_for_negation
      ta_negation_ok:
	push	eax
	mov	al,[esi]
	cmp	al,91h
	je	ta_logical_expression
	cmp	al,0FFh
	je	ta_invalid_expression
	cmp	al,88h
	je	ta_check_for_defined
	cmp	al,8Ah
	je	ta_check_for_earlier_defined
	cmp	al,89h
	je	ta_check_for_used
	cmp	al,'0'
	je	ta_given_false
	cmp	al,'1'
	je	ta_given_true
	cmp	al,'('
	jne	ta_invalid_value
	call	ta_get_value_for_comparison
	mov	bh,[ta_value_sign]
	push	eax edx
	push	[ta_symbol_identifier]
	push	ebx ecx
	mov	al,[esi]
	or	al,al
	jz	ta_logical_number
	cmp	al,0Fh
	je	ta_logical_number
	cmp	al,92h
	je	ta_logical_number
	cmp	al,'&'
	je	ta_logical_number
	cmp	al,'|'
	je	ta_logical_number
	inc	esi
	mov	[ta_compare_type],al
	cmp	byte [esi],'('
	jne	ta_invalid_value
	call	ta_get_value_for_comparison
	cmp	bl,[esp+4]
	jne	ta_values_not_relative
	or	bl,bl
	jz	ta_check_values_registers
	mov	ebx,[ta_symbol_identifier]
	cmp	ebx,[esp+8]
	jne	ta_values_not_relative
      ta_check_values_registers:
	cmp	ecx,[esp]
	je	ta_values_relative
	ror	ecx,16
	xchg	ch,cl
	ror	ecx,16
	xchg	ch,cl
	cmp	ecx,[esp]
	je	ta_values_relative
      ta_values_not_relative:
	cmp	[ta_compare_type],0F8h
	jne	ta_invalid_comparison
	add	esp,12+8
	jmp	ta_return_false
      ta_invalid_comparison:
	call	ta_recoverable_misuse
      ta_values_relative:
	pop	ebx
	shl	ebx,16
	mov	bx,[esp]
	add	esp,8
	pop	ecx ebp
	cmp	[ta_compare_type],'='
	je	ta_check_equal
	cmp	[ta_compare_type],0F1h
	je	ta_check_not_equal
	cmp	[ta_compare_type],0F8h
	je	ta_return_true
	test	ebx,0FFFF0000h
	jz	ta_check_less_or_greater
	call	ta_recoverable_misuse
      ta_check_less_or_greater:
	cmp	[ta_compare_type],'>'
	je	ta_check_greater
	cmp	[ta_compare_type],'<'
	je	ta_check_less
	cmp	[ta_compare_type],0F2h
	je	ta_check_not_less
	cmp	[ta_compare_type],0F3h
	je	ta_check_not_greater
	jmp	ta_invalid_expression
      ta_check_equal:
	cmp	bh,[ta_value_sign]
	jne	ta_return_false
	cmp	eax,ebp
	jne	ta_return_false
	cmp	edx,ecx
	jne	ta_return_false
	jmp	ta_return_true
      ta_check_greater:
	cmp	bh,[ta_value_sign]
	jg	ta_return_true
	jl	ta_return_false
	cmp	edx,ecx
	jb	ta_return_true
	ja	ta_return_false
	cmp	eax,ebp
	jb	ta_return_true
	jae	ta_return_false
      ta_check_less:
	cmp	bh,[ta_value_sign]
	jg	ta_return_false
	jl	ta_return_true
	cmp	edx,ecx
	jb	ta_return_false
	ja	ta_return_true
	cmp	eax,ebp
	jbe	ta_return_false
	ja	ta_return_true
      ta_check_not_less:
	cmp	bh,[ta_value_sign]
	jg	ta_return_true
	jl	ta_return_false
	cmp	edx,ecx
	jb	ta_return_true
	ja	ta_return_false
	cmp	eax,ebp
	jbe	ta_return_true
	ja	ta_return_false
      ta_check_not_greater:
	cmp	bh,[ta_value_sign]
	jg	ta_return_false
	jl	ta_return_true
	cmp	edx,ecx
	jb	ta_return_false
	ja	ta_return_true
	cmp	eax,ebp
	jb	ta_return_false
	jae	ta_return_true
      ta_check_not_equal:
	cmp	bh,[ta_value_sign]
	jne	ta_return_true
	cmp	eax,ebp
	jne	ta_return_true
	cmp	edx,ecx
	jne	ta_return_true
	jmp	ta_return_false
      ta_logical_number:
	pop	ecx ebx eax edx eax
	or	bl,bl
	jnz	ta_invalid_logical_number
	or	cx,cx
	jz	ta_logical_number_ok
      ta_invalid_logical_number:
	call	ta_recoverable_misuse
      ta_logical_number_ok:
	test	bh,bh
	jnz	ta_return_true
	or	eax,edx
	jnz	ta_return_true
	jmp	ta_return_false
      ta_check_for_earlier_defined:
	or	bh,-1
	jmp	ta_check_if_expression_defined
      ta_check_for_defined:
	xor	bh,bh
      ta_check_if_expression_defined:
	or	bl,-1
	lods	word [esi]
	cmp	ah,'('
	jne	ta_invalid_expression
      ta_check_expression:
	lods	byte [esi]
	or	al,al
	jz	ta_defined_string
	cmp	al,'.'
	je	ta_defined_fp_value
	cmp	al,')'
	je	ta_expression_checked
	cmp	al,'!'
	je	ta_invalid_expression
	cmp	al,0Fh
	je	ta_check_expression
	cmp	al,10h
	je	ta_defined_register
	cmp	al,11h
	je	ta_check_if_symbol_defined
	cmp	al,80h
	jae	ta_check_expression
	movzx	eax,al
	add	esi,eax
	jmp	ta_check_expression
      ta_defined_register:
	inc	esi
	jmp	ta_check_expression
      ta_defined_fp_value:
	add	esi,12+1
	jmp	ta_expression_checked
      ta_defined_string:
	lods	dword [esi]
	add	esi,eax
	inc	esi
	jmp	ta_expression_checked
      ta_check_if_symbol_defined:
	lods	dword [esi]
	cmp	eax,-1
	je	ta_invalid_expression
	cmp	eax,0Fh
	jb	ta_check_expression
	je	ta_reserved_word_used_as_symbol
	test	bh,bh
	jnz	ta_no_prediction
	test	byte [eax+8],4
	jnz	ta_no_prediction
	test	byte [eax+8],1
	jz	ta_symbol_predicted_undefined
	mov	cx,[ta_current_pass]
	sub	cx,[eax+16]
	jz	ta_check_expression
	cmp	cx,1
	ja	ta_symbol_predicted_undefined
	or	byte [eax+8],40h+80h
	jmp	ta_check_expression
      ta_no_prediction:
	test	byte [eax+8],1
	jz	ta_symbol_undefined
	mov	cx,[ta_current_pass]
	sub	cx,[eax+16]
	jz	ta_check_expression
	jmp	ta_symbol_undefined
      ta_symbol_predicted_undefined:
	or	byte [eax+8],40h
	and	byte [eax+8],not 80h
      ta_symbol_undefined:
	xor	bl,bl
	jmp	ta_check_expression
      ta_expression_checked:
	mov	al,bl
	jmp	ta_logical_value_ok
      ta_check_for_used:
	lods	word [esi]
	cmp	ah,2
	jne	ta_invalid_expression
	lods	dword [esi]
	cmp	eax,0Fh
	jb	ta_invalid_use_of_symbol
	je	ta_reserved_word_used_as_symbol
	inc	esi
	test	byte [eax+8],8
	jz	ta_not_used
	mov	cx,[ta_current_pass]
	sub	cx,[eax+18]
	jz	ta_return_true
	cmp	cx,1
	ja	ta_not_used
	or	byte [eax+8],10h+20h
	jmp	ta_return_true
      ta_not_used:
	or	byte [eax+8],10h
	and	byte [eax+8],not 20h
	jmp	ta_return_false
      ta_given_false:
	inc	esi
      ta_return_false:
	xor	al,al
	jmp	ta_logical_value_ok
      ta_given_true:
	inc	esi
      ta_return_true:
	or	al,-1
	jmp	ta_logical_value_ok
      ta_logical_expression:
	lods	byte [esi]
	mov	dl,[ta_logical_value_wrapping]
	push	edx
	call	ta_calculate_embedded_logical_expression
	pop	edx
	mov	[ta_logical_value_wrapping],dl
	push	eax
	lods	byte [esi]
	cmp	al,92h
	jne	ta_invalid_expression
	pop	eax
      ta_logical_value_ok:
	pop	ebx
	xor	al,bl
	ret

ta_skip_symbol:
	lods	byte [esi]
	or	al,al
	jz	ta_nothing_to_skip
	cmp	al,0Fh
	je	ta_nothing_to_skip
	cmp	al,1
	je	ta_skip_instruction
	cmp	al,2
	je	ta_skip_label
	cmp	al,3
	je	ta_skip_label
	cmp	al,4
	je	ta_skip_special_label
	cmp	al,20h
	jb	ta_skip_assembler_symbol
	cmp	al,'('
	je	ta_skip_expression
	cmp	al,'['
	je	ta_skip_address
      ta_skip_done:
	clc
	ret
      ta_skip_label:
	add	esi,2
      ta_skip_instruction:
	add	esi,2
      ta_skip_assembler_symbol:
	inc	esi
	jmp	ta_skip_done
      ta_skip_special_label:
	add	esi,4
	jmp	ta_skip_done
      ta_skip_address:
	mov	al,[esi]
	and	al,11110000b
	cmp	al,60h
	jb	ta_skip_expression
	cmp	al,70h
	ja	ta_skip_expression
	inc	esi
	jmp	ta_skip_address
      ta_skip_expression:
	lods	byte [esi]
	or	al,al
	jz	ta_skip_string
	cmp	al,'.'
	je	ta_skip_fp_value
	cmp	al,')'
	je	ta_skip_done
	cmp	al,']'
	je	ta_skip_done
	cmp	al,'!'
	je	ta_skip_expression
	cmp	al,0Fh
	je	ta_skip_expression
	cmp	al,10h
	je	ta_skip_register
	cmp	al,11h
	je	ta_skip_label_value
	cmp	al,80h
	jae	ta_skip_expression
	movzx	eax,al
	add	esi,eax
	jmp	ta_skip_expression
      ta_skip_label_value:
	add	esi,3
      ta_skip_register:
	inc	esi
	jmp	ta_skip_expression
      ta_skip_fp_value:
	add	esi,12
	jmp	ta_skip_done
      ta_skip_string:
	lods	dword [esi]
	add	esi,eax
	inc	esi
	jmp	ta_skip_done
      ta_nothing_to_skip:
	dec	esi
	stc
	ret

ta_expand_path:
	lods	byte [esi]
	cmp	al,'%'
	je	ta_environment_variable
	stos	byte [edi]
	or	al,al
	jnz	ta_expand_path
	cmp	edi,[ta_memory_end]
	ja	ta_out_of_memory
	ret
      ta_environment_variable:
	mov	ebx,esi
      ta_find_variable_end:
	lods	byte [esi]
	or	al,al
	jz	ta_not_environment_variable
	cmp	al,'%'
	jne	ta_find_variable_end
	mov	byte [esi-1],0
	push	esi
	mov	esi,ebx
	call	ta_get_environment_variable
	pop	esi
	mov	byte [esi-1],'%'
	jmp	ta_expand_path
      ta_not_environment_variable:
	mov	al,'%'
	stos	byte [edi]
	mov	esi,ebx
	jmp	ta_expand_path
ta_get_include_directory:
	lods	byte [esi]
	cmp	al,';'
	je	ta_include_directory_ok
	stos	byte [edi]
	or	al,al
	jnz	ta_get_include_directory
	dec	esi
	dec	edi
      ta_include_directory_ok:
	cmp	byte [edi-1],'/'
	je	ta_path_separator_ok
	cmp	byte [edi-1],'\'
	je	ta_path_separator_ok
	mov	al,'/'
	stos	byte [edi]
      ta_path_separator_ok:
	ret
