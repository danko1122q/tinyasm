ta_convert_expression:
	push	ebp
	call	ta_get_fp_value
	jnc	ta_fp_expression
	mov	[ta_current_offset],esp
      ta_expression_loop:
	push	edi
	mov	edi,ta_single_operand_operators
	call	ta_get_operator
	pop	edi
	or	al,al
	jz	ta_expression_element
	cmp	al,82h
	je	ta_expression_loop
	push	eax
	jmp	ta_expression_loop
      ta_expression_element:
	mov	al,[esi]
	cmp	al,1Ah
	je	ta_expression_number
	cmp	al,22h
	je	ta_expression_number
	cmp	al,'('
	je	ta_expression_number
	mov	al,'!'
	stos	byte [edi]
	jmp	ta_expression_operator
      ta_expression_number:
	call	ta_convert_number
      ta_expression_operator:
	push	edi
	mov	edi,ta_operators
	call	ta_get_operator
	pop	edi
	or	al,al
	jz	ta_expression_end
      ta_operators_loop:
	cmp	esp,[ta_current_offset]
	je	ta_push_operator
	mov	bl,al
	and	bl,0F0h
	mov	bh,byte [esp]
	and	bh,0F0h
	cmp	bl,bh
	ja	ta_push_operator
	pop	ebx
	mov	byte [edi],bl
	inc	edi
	jmp	ta_operators_loop
      ta_push_operator:
	push	eax
	jmp	ta_expression_loop
      ta_expression_end:
	cmp	esp,[ta_current_offset]
	je	ta_expression_converted
	pop	eax
	stos	byte [edi]
	jmp	ta_expression_end
      ta_expression_converted:
	pop	ebp
	ret
      ta_fp_expression:
	mov	al,'.'
	stos	byte [edi]
	mov	eax,[ta_fp_value]
	stos	dword [edi]
	mov	eax,[ta_fp_value+4]
	stos	dword [edi]
	mov	eax,[ta_fp_value+8]
	stos	dword [edi]
	pop	ebp
	ret

ta_convert_number:
	lea	eax,[edi+20h]
	mov	edx,[ta_memory_end]
	cmp	[ta_source_start],0
	je	ta_check_memory_for_number
	mov	edx,[ta_labels_list]
      ta_check_memory_for_number:
	cmp	eax,edx
	jae	ta_out_of_memory
	mov	eax,esp
	sub	eax,[ta_stack_limit]
	cmp	eax,100h
	jb	ta_stack_overflow
	cmp	byte [esi],'('
	je	ta_expression_value
	inc	edi
	call	ta_get_number
	jc	ta_symbol_value
	or	ebp,ebp
	jz	ta_valid_number
	mov	byte [edi-1],0Fh
	ret
      ta_valid_number:
	cmp	dword [edi+4],0
	jne	ta_qword_number
	cmp	word [edi+2],0
	jne	ta_dword_number
	cmp	byte [edi+1],0
	jne	ta_word_number
      ta_byte_number:
	mov	byte [edi-1],1
	inc	edi
	ret
      ta_qword_number:
	mov	byte [edi-1],8
	add	edi,8
	ret
      ta_dword_number:
	mov	byte [edi-1],4
	scas	dword [edi]
	ret
      ta_word_number:
	mov	byte [edi-1],2
	scas	word [edi]
	ret
      ta_expression_value:
	inc	esi
	push	[ta_current_offset]
	call	ta_convert_expression
	pop	[ta_current_offset]
	lods	byte [esi]
	cmp	al,')'
	je	ta_subexpression_closed
	dec	esi
	mov	al,'!'
	stosb
      ta_subexpression_closed:
	ret
      ta_symbol_value:
	mov	eax,[ta_source_start]
	test	eax,eax
	jz	ta_preprocessor_value
	cmp	eax,-1
	je	ta_invalid_value
	push	edi esi
	lods	word [esi]
	cmp	al,1Ah
	jne	ta_no_address_register
	movzx	ecx,ah
	call	ta_get_symbol
	jc	ta_no_address_register
	cmp	al,10h
	jne	ta_no_address_register
	mov	al,ah
	shr	ah,4
	cmp	ah,4
	je	ta_register_value
	and	ah,not 1
	cmp	ah,8
	je	ta_register_value
	cmp	ah,0Ch
	jae	ta_register_value
	cmp	ah,6
	je	ta_register_value
	cmp	al,23h
	je	ta_register_value
	cmp	al,25h
	je	ta_register_value
	cmp	al,26h
	je	ta_register_value
	cmp	al,27h
	je	ta_register_value
      ta_no_address_register:
	pop	esi
	mov	edi,ta_directive_operators
	call	ta_get_operator
	pop	edi
	or	al,al
	jnz	ta_broken_value
	lods	byte [esi]
	cmp	al,1Ah
	jne	ta_invalid_value
	lods	byte [esi]
	movzx	ecx,al
	call	ta_get_label_id
      ta_store_label_value:
	mov	byte [edi-1],11h
	stos	dword [edi]
	ret
      ta_broken_value:
	mov	eax,0Fh
	jmp	ta_store_label_value
      ta_register_value:
	pop	edx edi
	mov	byte [edi-1],10h
	stos	byte [edi]
	ret
      ta_preprocessor_value:
	dec	edi
	lods	byte [esi]
	cmp	al,1Ah
	jne	ta_invalid_value
	lods	byte [esi]
	mov	cl,al
	mov	ch,10b
	call	ta_get_preprocessor_symbol
	jc	ta_invalid_value
	test	edx,edx
	jz	ta_special_preprocessor_value
	push	esi
	mov	esi,[edx+8]
	push	[ta_current_offset]
	call	ta_convert_expression
	pop	[ta_current_offset]
	pop	esi
	ret
      ta_special_preprocessor_value:
	cmp	eax,ta_preprocessed_line_value
	jne	ta_invalid_value
	call	ta_get_current_line_from_file
	mov	al,4
	stos	byte [edi]
	mov	eax,[ebx+4]
	stos	dword [edi]
	ret

ta_get_number:
	xor	ebp,ebp
	lods	byte [esi]
	cmp	al,22h
	je	ta_get_text_number
	cmp	al,1Ah
	jne	ta_not_number
	lods	byte [esi]
	movzx	ecx,al
	mov	[ta_number_start],esi
	mov	al,[esi]
	cmp	al,'$'
	je	ta_number_begin
	sub	al,30h
	cmp	al,9
	ja	ta_invalid_number
      ta_number_begin:
	mov	ebx,esi
	add	esi,ecx
	push	esi
	dec	esi
	mov	dword [edi],0
	mov	dword [edi+4],0
	cmp	byte [ebx],'$'
	je	ta_pascal_hex_number
	cmp	word [ebx],'0x'
	je	ta_get_hex_number
	mov	al,[esi]
	dec	esi
	cmp	al,'h'
	je	ta_get_hex_number
	cmp	al,'b'
	je	ta_get_bin_number
	cmp	al,'d'
	je	ta_get_dec_number
	cmp	al,'o'
	je	ta_get_oct_number
	cmp	al,'q'
	je	ta_get_oct_number
	cmp	al,'H'
	je	ta_get_hex_number
	cmp	al,'B'
	je	ta_get_bin_number
	cmp	al,'D'
	je	ta_get_dec_number
	cmp	al,'O'
	je	ta_get_oct_number
	cmp	al,'Q'
	je	ta_get_oct_number
	inc	esi
      ta_get_dec_number:
	mov	ebx,esi
	mov	esi,[ta_number_start]
      ta_get_dec_digit:
	cmp	esi,ebx
	ja	ta_number_ok
	cmp	byte [esi],27h
	je	ta_next_dec_digit
	cmp	byte [esi],'_'
	je	ta_next_dec_digit
	xor	edx,edx
	mov	eax,[edi]
	shld	edx,eax,2
	shl	eax,2
	add	eax,[edi]
	adc	edx,0
	add	eax,eax
	adc	edx,edx
	mov	[edi],eax
	mov	eax,[edi+4]
	add	eax,eax
	jc	ta_dec_out_of_range
	add	eax,eax
	jc	ta_dec_out_of_range
	add	eax,[edi+4]
	jc	ta_dec_out_of_range
	add	eax,eax
	jc	ta_dec_out_of_range
	add	eax,edx
	jc	ta_dec_out_of_range
	mov	[edi+4],eax
	movzx	eax,byte [esi]
	sub	al,30h
	jc	ta_bad_number
	cmp	al,9
	ja	ta_bad_number
	add	[edi],eax
	adc	dword [edi+4],0
	jc	ta_dec_out_of_range
      ta_next_dec_digit:
	inc	esi
	jmp	ta_get_dec_digit
      ta_dec_out_of_range:
	cmp	esi,ebx
	ja	ta_dec_out_of_range_finished
	lods	byte [esi]
	cmp	al,27h
	je	ta_bad_number
	cmp	al,'_'
	je	ta_bad_number
	sub	al,30h
	jc	ta_bad_number
	cmp	al,9
	ja	ta_bad_number
	jmp	ta_dec_out_of_range
      ta_dec_out_of_range_finished:
	or	ebp,-1
	jmp	ta_number_ok
      ta_bad_number:
	pop	eax
      ta_invalid_number:
	mov	esi,[ta_number_start]
	dec	esi
      ta_not_number:
	dec	esi
	stc
	ret
      ta_get_bin_number:
	xor	bl,bl
      ta_get_bin_digit:
	cmp	esi,[ta_number_start]
	jb	ta_number_ok
	movzx	eax,byte [esi]
	cmp	al,27h
	je	ta_bin_digit_skip
	cmp	al,'_'
	je	ta_bin_digit_skip
	sub	al,30h
	cmp	al,1
	ja	ta_bad_number
	xor	edx,edx
	mov	cl,bl
	dec	esi
	cmp	bl,64
	je	ta_bin_out_of_range
	inc	bl
	cmp	cl,32
	jae	ta_bin_digit_high
	shl	eax,cl
	or	dword [edi],eax
	jmp	ta_get_bin_digit
      ta_bin_digit_high:
	sub	cl,32
	shl	eax,cl
	or	dword [edi+4],eax
	jmp	ta_get_bin_digit
      ta_bin_out_of_range:
	or	al,al
	jz	ta_get_bin_digit
	or	ebp,-1
	jmp	ta_get_bin_digit
      ta_bin_digit_skip:
	dec	esi
	jmp	ta_get_bin_digit
      ta_pascal_hex_number:
	cmp	cl,1
	je	ta_bad_number
      ta_get_hex_number:
	xor	bl,bl
      ta_get_hex_digit:
	cmp	esi,[ta_number_start]
	jb	ta_number_ok
	movzx	eax,byte [esi]
	cmp	al,27h
	je	ta_hex_digit_skip
	cmp	al,'_'
	je	ta_hex_digit_skip
	cmp	al,'x'
	je	ta_hex_number_ok
	cmp	al,'$'
	je	ta_pascal_hex_ok
	sub	al,30h
	cmp	al,9
	jbe	ta_hex_digit_ok
	sub	al,7
	cmp	al,15
	jbe	ta_hex_letter_digit_ok
	sub	al,20h
	cmp	al,15
	ja	ta_bad_number
      ta_hex_letter_digit_ok:
	cmp	al,10
	jb	ta_bad_number
      ta_hex_digit_ok:
	xor	edx,edx
	mov	cl,bl
	dec	esi
	cmp	bl,64
	je	ta_hex_out_of_range
	add	bl,4
	cmp	cl,32
	jae	ta_hex_digit_high
	shl	eax,cl
	or	dword [edi],eax
	jmp	ta_get_hex_digit
      ta_hex_digit_high:
	sub	cl,32
	shl	eax,cl
	or	dword [edi+4],eax
	jmp	ta_get_hex_digit
      ta_hex_out_of_range:
	or	al,al
	jz	ta_get_hex_digit
	or	ebp,-1
	jmp	ta_get_hex_digit
      ta_hex_digit_skip:
	dec	esi
	jmp	ta_get_hex_digit
      ta_get_oct_number:
	xor	bl,bl
      ta_get_oct_digit:
	cmp	esi,[ta_number_start]
	jb	ta_number_ok
	movzx	eax,byte [esi]
	cmp	al,27h
	je	ta_oct_digit_skip
	cmp	al,'_'
	je	ta_oct_digit_skip
	sub	al,30h
	cmp	al,7
	ja	ta_bad_number
      ta_oct_digit_ok:
	xor	edx,edx
	mov	cl,bl
	dec	esi
	cmp	bl,63
	ja	ta_oct_out_of_range
	jne	ta_oct_range_ok
	cmp	al,1
	ja	ta_oct_out_of_range
      ta_oct_range_ok:
	add	bl,3
	cmp	cl,30
	je	ta_oct_digit_wrap
	ja	ta_oct_digit_high
	shl	eax,cl
	or	dword [edi],eax
	jmp	ta_get_oct_digit
      ta_oct_digit_wrap:
	shl	eax,cl
	adc	dword [edi+4],0
	or	dword [edi],eax
	jmp	ta_get_oct_digit
      ta_oct_digit_high:
	sub	cl,32
	shl	eax,cl
	or	dword [edi+4],eax
	jmp	ta_get_oct_digit
      ta_oct_digit_skip:
	dec	esi
	jmp	ta_get_oct_digit
      ta_oct_out_of_range:
	or	al,al
	jz	ta_get_oct_digit
	or	ebp,-1
	jmp	ta_get_oct_digit
      ta_hex_number_ok:
	dec	esi
      ta_pascal_hex_ok:
	cmp	esi,[ta_number_start]
	jne	ta_bad_number
      ta_number_ok:
	pop	esi
      ta_number_done:
	clc
	ret
      ta_get_text_number:
	lods	dword [esi]
	mov	edx,eax
	xor	bl,bl
	mov	dword [edi],0
	mov	dword [edi+4],0
      ta_get_text_character:
	sub	edx,1
	jc	ta_number_done
	movzx	eax,byte [esi]
	inc	esi
	mov	cl,bl
	cmp	bl,64
	je	ta_text_out_of_range
	add	bl,8
	cmp	cl,32
	jae	ta_text_character_high
	shl	eax,cl
	or	dword [edi],eax
	jmp	ta_get_text_character
      ta_text_character_high:
	sub	cl,32
	shl	eax,cl
	or	dword [edi+4],eax
	jmp	ta_get_text_character
      ta_text_out_of_range:
	or	ebp,-1
	jmp	ta_get_text_character

ta_get_fp_value:
	push	edi esi
      ta_fp_value_start:
	lods	byte [esi]
	cmp	al,'-'
	je	ta_fp_value_start
	cmp	al,'+'
	je	ta_fp_value_start
	cmp	al,1Ah
	jne	ta_not_fp_value
	lods	byte [esi]
	movzx	ecx,al
	cmp	cl,1
	jbe	ta_not_fp_value
	lea	edx,[esi+1]
	xor	ah,ah
      ta_check_fp_value:
	lods	byte [esi]
	cmp	al,'.'
	je	ta_fp_character_dot
	cmp	al,'E'
	je	ta_fp_character_exp
	cmp	al,'e'
	je	ta_fp_character_exp
	cmp	al,'F'
	je	ta_fp_last_character
	cmp	al,'f'
	je	ta_fp_last_character
      ta_digit_expected:
	cmp	al,'0'
	jb	ta_not_fp_value
	cmp	al,'9'
	ja	ta_not_fp_value
	jmp	ta_fp_character_ok
      ta_fp_character_dot:
	cmp	esi,edx
	je	ta_not_fp_value
	or	ah,ah
	jnz	ta_not_fp_value
	or	ah,1
	lods	byte [esi]
	loop	ta_digit_expected
      ta_not_fp_value:
	pop	esi edi
	stc
	ret
      ta_fp_last_character:
	cmp	cl,1
	jne	ta_not_fp_value
	or	ah,4
	jmp	ta_fp_character_ok
      ta_fp_character_exp:
	cmp	esi,edx
	je	ta_not_fp_value
	cmp	ah,1
	ja	ta_not_fp_value
	or	ah,2
	cmp	ecx,1
	jne	ta_fp_character_ok
	cmp	byte [esi],'+'
	je	ta_fp_exp_sign
	cmp	byte [esi],'-'
	jne	ta_fp_character_ok
      ta_fp_exp_sign:
	inc	esi
	cmp	byte [esi],1Ah
	jne	ta_not_fp_value
	inc	esi
	lods	byte [esi]
	movzx	ecx,al
	inc	ecx
      ta_fp_character_ok:
	dec	ecx
	jnz	ta_check_fp_value
	or	ah,ah
	jz	ta_not_fp_value
	pop	esi
	mov	[ta_fp_sign],0
      ta_fp_get_sign:
	lods	byte [esi]
	cmp	al,1Ah
	je	ta_fp_get
	cmp	al,'+'
	je	ta_fp_get_sign
	xor	[ta_fp_sign],1
	jmp	ta_fp_get_sign
      ta_fp_get:
	lods	byte [esi]
	movzx	ecx,al
	xor	edx,edx
	mov	edi,ta_fp_value
	mov	[edi],edx
	mov	[edi+4],edx
	mov	[edi+12],edx
	call	ta_fp_optimize
	mov	[ta_fp_format],0
	mov	al,[esi]
      ta_fp_before_dot:
	lods	byte [esi]
	cmp	al,'.'
	je	ta_fp_dot
	cmp	al,'E'
	je	ta_fp_exponent
	cmp	al,'e'
	je	ta_fp_exponent
	cmp	al,'F'
	je	ta_fp_done
	cmp	al,'f'
	je	ta_fp_done
	sub	al,30h
	mov	edi,ta_fp_value+16
	xor	edx,edx
	mov	dword [edi+12],edx
	mov	dword [edi],edx
	mov	dword [edi+4],edx
	mov	[edi+7],al
	mov	dl,7
	mov	dword [edi+8],edx
	call	ta_fp_optimize
	mov	edi,ta_fp_value
	push	ecx
	mov	ecx,10
	call	ta_fp_mul
	pop	ecx
	mov	ebx,ta_fp_value+16
	call	ta_fp_add
	loop	ta_fp_before_dot
      ta_fp_dot:
	mov	edi,ta_fp_value+16
	xor	edx,edx
	mov	[edi],edx
	mov	[edi+4],edx
	mov	byte [edi+7],80h
	mov	[edi+8],edx
	mov	dword [edi+12],edx
	dec	ecx
	jz	ta_fp_done
      ta_fp_after_dot:
	lods	byte [esi]
	cmp	al,'E'
	je	ta_fp_exponent
	cmp	al,'e'
	je	ta_fp_exponent
	cmp	al,'F'
	je	ta_fp_done
	cmp	al,'f'
	je	ta_fp_done
	inc	[ta_fp_format]
	cmp	[ta_fp_format],80h
	jne	ta_fp_counter_ok
	mov	[ta_fp_format],7Fh
      ta_fp_counter_ok:
	dec	esi
	mov	edi,ta_fp_value+16
	push	ecx
	mov	ecx,10
	call	ta_fp_div
	push	dword [edi]
	push	dword [edi+4]
	push	dword [edi+8]
	push	dword [edi+12]
	lods	byte [esi]
	sub	al,30h
	movzx	ecx,al
	call	ta_fp_mul
	mov	ebx,edi
	mov	edi,ta_fp_value
	call	ta_fp_add
	mov	edi,ta_fp_value+16
	pop	dword [edi+12]
	pop	dword [edi+8]
	pop	dword [edi+4]
	pop	dword [edi]
	pop	ecx
	dec	ecx
	jnz	ta_fp_after_dot
	jmp	ta_fp_done
      ta_fp_exponent:
	or	[ta_fp_format],80h
	xor	edx,edx
	xor	ebp,ebp
	dec	ecx
	jnz	ta_get_exponent
	cmp	byte [esi],'+'
	je	ta_fp_exponent_sign
	cmp	byte [esi],'-'
	jne	ta_fp_done
	not	ebp
      ta_fp_exponent_sign:
	add	esi,2
	lods	byte [esi]
	movzx	ecx,al
      ta_get_exponent:
	movzx	eax,byte [esi]
	inc	esi
	sub	al,30h
	cmp	al,10
	jae	ta_exponent_ok
	imul	edx,10
	cmp	edx,8000h
	jae	ta_value_out_of_range
	add	edx,eax
	loop	ta_get_exponent
      ta_exponent_ok:
	mov	edi,ta_fp_value
	or	edx,edx
	jz	ta_fp_done
	mov	ecx,edx
	or	ebp,ebp
	jnz	ta_fp_negative_power
      ta_fp_power:
	push	ecx
	mov	ecx,10
	call	ta_fp_mul
	pop	ecx
	loop	ta_fp_power
	jmp	ta_fp_done
      ta_fp_negative_power:
	push	ecx
	mov	ecx,10
	call	ta_fp_div
	pop	ecx
	loop	ta_fp_negative_power
      ta_fp_done:
	mov	edi,ta_fp_value
	mov	al,[ta_fp_format]
	mov	[edi+10],al
	mov	al,[ta_fp_sign]
	mov	[edi+11],al
	test	byte [edi+15],80h
	jz	ta_fp_ok
	add	dword [edi],1
	adc	dword [edi+4],0
	jnc	ta_fp_ok
	mov	eax,[edi+4]
	shrd	[edi],eax,1
	shr	eax,1
	or	eax,80000000h
	mov	[edi+4],eax
	inc	word [edi+8]
      ta_fp_ok:
	pop	edi
	clc
	ret
      ta_fp_mul:
	or	ecx,ecx
	jz	ta_fp_zero
	mov	eax,[edi+12]
	mul	ecx
	mov	[edi+12],eax
	mov	ebx,edx
	mov	eax,[edi]
	mul	ecx
	add	eax,ebx
	adc	edx,0
	mov	[edi],eax
	mov	ebx,edx
	mov	eax,[edi+4]
	mul	ecx
	add	eax,ebx
	adc	edx,0
	mov	[edi+4],eax
      .loop:
	or	edx,edx
	jz	.done
	mov	eax,[edi]
	shrd	[edi+12],eax,1
	mov	eax,[edi+4]
	shrd	[edi],eax,1
	shrd	eax,edx,1
	mov	[edi+4],eax
	shr	edx,1
	inc	dword [edi+8]
	cmp	dword [edi+8],8000h
	jge	ta_value_out_of_range
	jmp	.loop
      .done:
	ret
      ta_fp_div:
	mov	eax,[edi+4]
	xor	edx,edx
	div	ecx
	mov	[edi+4],eax
	mov	eax,[edi]
	div	ecx
	mov	[edi],eax
	mov	eax,[edi+12]
	div	ecx
	mov	[edi+12],eax
	mov	ebx,eax
	or	ebx,[edi]
	or	ebx,[edi+4]
	jz	ta_fp_zero
      .loop:
	test	byte [edi+7],80h
	jnz	.exp_ok
	mov	eax,[edi]
	shld	[edi+4],eax,1
	mov	eax,[edi+12]
	shld	[edi],eax,1
	add	eax,eax
	mov	[edi+12],eax
	dec	dword [edi+8]
	add	edx,edx
	jmp	.loop
      .exp_ok:
	mov	eax,edx
	xor	edx,edx
	div	ecx
	add	[edi+12],eax
	adc	dword [edi],0
	adc	dword [edi+4],0
	jnc	.done
	mov	eax,[edi+4]
	mov	ebx,[edi]
	shrd	[edi],eax,1
	shrd	[edi+12],ebx,1
	shr	eax,1
	or	eax,80000000h
	mov	[edi+4],eax
	inc	dword [edi+8]
      .done:
	ret
      ta_fp_add:
	cmp	dword [ebx+8],8000h
	je	.done
	cmp	dword [edi+8],8000h
	je	.copy
	mov	eax,[ebx+8]
	cmp	eax,[edi+8]
	jge	.exp_ok
	mov	eax,[edi+8]
      .exp_ok:
	call	.change_exp
	xchg	ebx,edi
	call	.change_exp
	xchg	ebx,edi
	mov	edx,[ebx+12]
	mov	eax,[ebx]
	mov	ebx,[ebx+4]
	add	[edi+12],edx
	adc	[edi],eax
	adc	[edi+4],ebx
	jnc	.done
	mov	eax,[edi]
	shrd	[edi+12],eax,1
	mov	eax,[edi+4]
	shrd	[edi],eax,1
	shr	eax,1
	or	eax,80000000h
	mov	[edi+4],eax
	inc	dword [edi+8]
      .done:
	ret
      .copy:
	mov	eax,[ebx]
	mov	[edi],eax
	mov	eax,[ebx+4]
	mov	[edi+4],eax
	mov	eax,[ebx+8]
	mov	[edi+8],eax
	mov	eax,[ebx+12]
	mov	[edi+12],eax
	ret
      .change_exp:
	push	ecx
	mov	ecx,eax
	sub	ecx,[ebx+8]
	mov	edx,[ebx+4]
	jecxz	.exp_done
      .exp_loop:
	mov	ebp,[ebx]
	shrd	[ebx+12],ebp,1
	shrd	[ebx],edx,1
	shr	edx,1
	inc	dword [ebx+8]
	loop	.exp_loop
      .exp_done:
	mov	[ebx+4],edx
	pop	ecx
	ret
      ta_fp_optimize:
	mov	eax,[edi]
	mov	ebp,[edi+4]
	or	ebp,[edi]
	or	ebp,[edi+12]
	jz	ta_fp_zero
      .loop:
	test	byte [edi+7],80h
	jnz	.done
	shld	[edi+4],eax,1
	mov	ebp,[edi+12]
	shld	eax,ebp,1
	mov	[edi],eax
	shl	dword [edi+12],1
	dec	dword [edi+8]
	jmp	.loop
      .done:
	ret
      ta_fp_zero:
	mov	dword [edi+8],8000h
	ret

ta_preevaluate_logical_expression:
	xor	al,al
  ta_preevaluate_embedded_logical_expression:
	mov	[ta_logical_value_wrapping],al
	push	edi
	call	ta_preevaluate_logical_value
      ta_preevaluation_loop:
	cmp	al,0FFh
	je	ta_invalid_logical_expression
	mov	dl,[esi]
	inc	esi
	cmp	dl,'|'
	je	ta_preevaluate_or
	cmp	dl,'&'
	je	ta_preevaluate_and
	cmp	dl,92h
	je	ta_preevaluation_done
	or	dl,dl
	jnz	ta_invalid_logical_expression
      ta_preevaluation_done:
	pop	edx
	dec	esi
	ret
      ta_preevaluate_or:
	cmp	al,'1'
	je	ta_quick_true
	cmp	al,'0'
	je	ta_leave_only_following
	push	edi
	mov	al,dl
	stos	byte [edi]
	call	ta_preevaluate_logical_value
	pop	ebx
	cmp	al,'0'
	je	ta_leave_only_preceding
	cmp	al,'1'
	jne	ta_preevaluation_loop
	stos	byte [edi]
	xor	al,al
	jmp	ta_preevaluation_loop
      ta_preevaluate_and:
	cmp	al,'0'
	je	ta_quick_false
	cmp	al,'1'
	je	ta_leave_only_following
	push	edi
	mov	al,dl
	stos	byte [edi]
	call	ta_preevaluate_logical_value
	pop	ebx
	cmp	al,'1'
	je	ta_leave_only_preceding
	cmp	al,'0'
	jne	ta_preevaluation_loop
	stos	byte [edi]
	xor	al,al
	jmp	ta_preevaluation_loop
      ta_leave_only_following:
	mov	edi,[esp]
	call	ta_preevaluate_logical_value
	jmp	ta_preevaluation_loop
      ta_leave_only_preceding:
	mov	edi,ebx
	xor	al,al
	jmp	ta_preevaluation_loop
      ta_quick_true:
	call	ta_skip_logical_value
	jc	ta_invalid_logical_expression
	mov	edi,[esp]
	mov	al,'1'
	jmp	ta_preevaluation_loop
      ta_quick_false:
	call	ta_skip_logical_value
	jc	ta_invalid_logical_expression
	mov	edi,[esp]
	mov	al,'0'
	jmp	ta_preevaluation_loop
      ta_invalid_logical_expression:
	pop	edi
	mov	esi,edi
	mov	al,0FFh
	stos	byte [edi]
	ret
  ta_skip_logical_value:
	cmp	byte [esi],'~'
	jne	ta_negation_skipped
	inc	esi
	jmp	ta_skip_logical_value
      ta_negation_skipped:
	mov	al,[esi]
	cmp	al,91h
	jne	ta_skip_simple_logical_value
	inc	esi
	xchg	al,[ta_logical_value_wrapping]
	push	eax
      ta_skip_logical_expression:
	call	ta_skip_logical_value
	lods	byte [esi]
	or	al,al
	jz	ta_wrongly_structured_logical_expression
	cmp	al,0Fh
	je	ta_wrongly_structured_logical_expression
	cmp	al,'|'
	je	ta_skip_logical_expression
	cmp	al,'&'
	je	ta_skip_logical_expression
	cmp	al,92h
	jne	ta_wrongly_structured_logical_expression
	pop	eax
	mov	[ta_logical_value_wrapping],al
      ta_logical_value_skipped:
	clc
	ret
      ta_wrongly_structured_logical_expression:
	pop	eax
	stc
	ret
      ta_skip_simple_logical_value:
	mov	[ta_logical_value_parentheses],0
      ta_find_simple_logical_value_end:
	mov	al,[esi]
	or	al,al
	jz	ta_logical_value_skipped
	cmp	al,0Fh
	je	ta_logical_value_skipped
	cmp	al,'|'
	je	ta_logical_value_skipped
	cmp	al,'&'
	je	ta_logical_value_skipped
	cmp	al,91h
	je	ta_skip_logical_value_internal_parenthesis
	cmp	al,92h
	jne	ta_skip_logical_value_symbol
	sub	[ta_logical_value_parentheses],1
	jnc	ta_skip_logical_value_symbol
	cmp	[ta_logical_value_wrapping],91h
	jne	ta_skip_logical_value_symbol
	jmp	ta_logical_value_skipped
      ta_skip_logical_value_internal_parenthesis:
	inc	[ta_logical_value_parentheses]
      ta_skip_logical_value_symbol:
	call	ta_skip_symbol
	jmp	ta_find_simple_logical_value_end
  ta_preevaluate_logical_value:
	mov	ebp,edi
      ta_preevaluate_negation:
	cmp	byte [esi],'~'
	jne	ta_preevaluate_negation_ok
	movs	byte [edi],[esi]
	jmp	ta_preevaluate_negation
      ta_preevaluate_negation_ok:
	mov	ebx,esi
	cmp	byte [esi],91h
	jne	ta_preevaluate_simple_logical_value
	lods	byte [esi]
	stos	byte [edi]
	push	ebp
	mov	dl,[ta_logical_value_wrapping]
	push	edx
	call	ta_preevaluate_embedded_logical_expression
	pop	edx
	mov	[ta_logical_value_wrapping],dl
	pop	ebp
	cmp	al,0FFh
	je	ta_invalid_logical_value
	cmp	byte [esi],92h
	jne	ta_invalid_logical_value
	or	al,al
	jnz	ta_preevaluated_expression_value
	movs	byte [edi],[esi]
	ret
      ta_preevaluated_expression_value:
	inc	esi
	lea	edx,[edi-1]
	sub	edx,ebp
	test	edx,1
	jz	ta_expression_negation_ok
	xor	al,1
      ta_expression_negation_ok:
	mov	edi,ebp
	ret
      ta_invalid_logical_value:
	mov	edi,ebp
	mov	al,0FFh
	ret
      ta_preevaluate_simple_logical_value:
	xor	edx,edx
	mov	[ta_logical_value_parentheses],edx
      ta_find_logical_value_boundaries:
	mov	al,[esi]
	or	al,al
	jz	ta_logical_value_boundaries_found
	cmp	al,91h
	je	ta_logical_value_internal_parentheses
	cmp	al,92h
	je	ta_logical_value_boundaries_parenthesis_close
	cmp	al,'|'
	je	ta_logical_value_boundaries_found
	cmp	al,'&'
	je	ta_logical_value_boundaries_found
	or	edx,edx
	jnz	ta_next_symbol_in_logical_value
	cmp	al,0F0h
	je	ta_preevaluable_logical_operator
	cmp	al,0F7h
	je	ta_preevaluable_logical_operator
	cmp	al,0F6h
	jne	ta_next_symbol_in_logical_value
      ta_preevaluable_logical_operator:
	mov	edx,esi
      ta_next_symbol_in_logical_value:
	call	ta_skip_symbol
	jmp	ta_find_logical_value_boundaries
      ta_logical_value_internal_parentheses:
	inc	[ta_logical_value_parentheses]
	jmp	ta_next_symbol_in_logical_value
      ta_logical_value_boundaries_parenthesis_close:
	sub	[ta_logical_value_parentheses],1
	jnc	ta_next_symbol_in_logical_value
	cmp	[ta_logical_value_wrapping],91h
	jne	ta_next_symbol_in_logical_value
      ta_logical_value_boundaries_found:
	or	edx,edx
	jz	ta_non_preevaluable_logical_value
	mov	al,[edx]
	cmp	al,0F0h
	je	ta_compare_symbols
	cmp	al,0F7h
	je	ta_compare_symbol_types
	cmp	al,0F6h
	je	ta_scan_symbols_list
      ta_non_preevaluable_logical_value:
	mov	ecx,esi
	mov	esi,ebx
	sub	ecx,esi
	jz	ta_invalid_logical_value
	cmp	esi,edi
	je	ta_leave_logical_value_intact
	rep	movs byte [edi],[esi]
	xor	al,al
	ret
      ta_leave_logical_value_intact:
	add	edi,ecx
	add	esi,ecx
	xor	al,al
	ret
      ta_compare_symbols:
	lea	ecx,[esi-1]
	sub	ecx,edx
	mov	eax,edx
	sub	eax,ebx
	cmp	ecx,eax
	jne	ta_preevaluated_false
	push	esi edi
	mov	esi,ebx
	lea	edi,[edx+1]
	repe	cmps byte [esi],[edi]
	pop	edi esi
	je	ta_preevaluated_true
      ta_preevaluated_false:
	mov	eax,edi
	sub	eax,ebp
	test	eax,1
	jnz	ta_store_true
      ta_store_false:
	mov	edi,ebp
	mov	al,'0'
	ret
      ta_preevaluated_true:
	mov	eax,edi
	sub	eax,ebp
	test	eax,1
	jnz	ta_store_false
      ta_store_true:
	mov	edi,ebp
	mov	al,'1'
	ret
      ta_compare_symbol_types:
	push	esi
	lea	esi,[edx+1]
      ta_type_comparison:
	cmp	esi,[esp]
	je	ta_types_compared
	mov	al,[esi]
	cmp	al,[ebx]
	jne	ta_different_type
	cmp	al,'('
	jne	ta_equal_type
	mov	al,[esi+1]
	mov	ah,[ebx+1]
	cmp	al,ah
	je	ta_equal_type
	or	al,al
	jz	ta_different_type
	or	ah,ah
	jz	ta_different_type
	cmp	al,'.'
	je	ta_different_type
	cmp	ah,'.'
	je	ta_different_type
      ta_equal_type:
	call	ta_skip_symbol
	xchg	esi,ebx
	call	ta_skip_symbol
	xchg	esi,ebx
	jmp	ta_type_comparison
      ta_types_compared:
	pop	esi
	cmp	byte [ebx],0F7h
	jne	ta_preevaluated_false
	jmp	ta_preevaluated_true
      ta_different_type:
	pop	esi
	jmp	ta_preevaluated_false
      ta_scan_symbols_list:
	push	edi esi
	lea	esi,[edx+1]
	sub	edx,ebx
	lods	byte [esi]
	cmp	al,'<'
	jne	ta_invalid_symbols_list
      ta_get_next_from_list:
	mov	edi,esi
      ta_get_from_list:
	cmp	byte [esi],','
	je	ta_compare_in_list
	cmp	byte [esi],'>'
	je	ta_compare_in_list
	cmp	esi,[esp]
	jae	ta_invalid_symbols_list
	call	ta_skip_symbol
	jmp	ta_get_from_list
      ta_compare_in_list:
	mov	ecx,esi
	sub	ecx,edi
	cmp	ecx,edx
	jne	ta_not_equal_length_in_list
	mov	esi,ebx
	repe	cmps byte [esi],[edi]
	mov	esi,edi
	jne	ta_not_equal_in_list
      ta_skip_rest_of_list:
	cmp	byte [esi],'>'
	je	ta_check_list_end
	cmp	esi,[esp]
	jae	ta_invalid_symbols_list
	call	ta_skip_symbol
	jmp	ta_skip_rest_of_list
      ta_check_list_end:
	inc	esi
	cmp	esi,[esp]
	jne	ta_invalid_symbols_list
	pop	esi edi
	jmp	ta_preevaluated_true
      ta_not_equal_in_list:
	add	esi,ecx
      ta_not_equal_length_in_list:
	lods	byte [esi]
	cmp	al,','
	je	ta_get_next_from_list
	cmp	esi,[esp]
	jne	ta_invalid_symbols_list
	pop	esi edi
	jmp	ta_preevaluated_false
      ta_invalid_symbols_list:
	pop	esi edi
	jmp	ta_invalid_logical_value
