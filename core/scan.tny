ta_parser:
	mov	eax,[ta_memory_end]
	mov	[ta_labels_list],eax
	mov	eax,[ta_additional_memory]
	mov	[ta_free_additional_memory],eax
	xor	eax,eax
	mov	[ta_current_locals_prefix],eax
	mov	[ta_anonymous_reverse],eax
	mov	[ta_anonymous_forward],eax
	mov	[ta_hash_tree],eax
	mov	[ta_blocks_stack],eax
	mov	[ta_parsed_lines],eax
	mov	esi,[ta_memory_start]
	promote_esi
	mov	edi,[ta_source_start]
	promote_edi
      ta_parser_loop:
	mov	[ta_current_line],esi
	lea	eax,[edi+100h]
	cmp	eax,[ta_labels_list]
	jae	ta_out_of_memory
	cmp	byte [esi+16],0
	je	ta_empty_line
	cmp	byte [esi+16],3Bh
	je	ta_empty_line
	mov	al,0Fh
	stos	byte [edi]
	mov	eax,esi
	stos	dword [edi]
	inc	[ta_parsed_lines]
	add	esi,16
      ta_parse_line:
	mov	[ta_formatter_symbols_allowed],0
	mov	[ta_decorator_symbols_allowed],0
	cmp	byte [esi],1Ah
	jne	ta_empty_instruction
	push	edi
	add	esi,2
	movzx	ecx,byte [esi-1]
	cmp	byte [esi+ecx],':'
	je	ta_simple_label
	cmp	byte [esi+ecx],'='
	je	ta_constant_label
	call	ta_get_instruction
	jnc	ta_main_instruction_identified
	cmp	byte [esi+ecx],1Ah
	jne	ta_no_data_label
	push	esi ecx
	lea	esi,[esi+ecx+2]
	movzx	ecx,byte [esi-1]
	call	ta_get_data_directive
	jnc	ta_data_label
	pop	ecx esi
      ta_no_data_label:
	call	ta_get_data_directive
	jnc	ta_main_instruction_identified
	pop	edi
	sub	esi,2
	xor	bx,bx
	call	ta_parse_line_contents
	jmp	ta_parse_next_line
      ta_simple_label:
	pop	edi
	call	ta_identify_label
	cmp	byte [esi+1],':'
	je	ta_block_label
	mov	byte [edi],2
	inc	edi
	stos	dword [edi]
	inc	esi
	xor	al,al
	stos	byte [edi]
	jmp	ta_parse_line
      ta_block_label:
	mov	byte [edi],4
	inc	edi
	stos	dword [edi]
	add	esi,2
	jmp	ta_parse_line
      ta_constant_label:
	pop	edi
	call	ta_get_label_id
	mov	byte [edi],3
	inc	edi
	stos	dword [edi]
	xor	al,al
	stos	byte [edi]
	inc	esi
	xor	bx,bx
	call	ta_parse_line_contents
	jmp	ta_parse_next_line
      ta_data_label:
	pop	ecx edx
	pop	edi
	push	eax ebx esi
	mov	esi,edx
	movzx	ecx,byte [esi-1]
	call	ta_identify_label
	mov	byte [edi],2
	inc	edi
	stos	dword [edi]
	pop	esi ebx eax
	stos	byte [edi]
	push	edi
      ta_main_instruction_identified:
	pop	edi
	mov	dl,al
	mov	al,1
	stos	byte [edi]
	mov	ax,bx
	stos	word [edi]
	mov	al,dl
	stos	byte [edi]
	cmp	bx,ta_if_directive-ta_instruction_handler
	je	ta_parse_block
	cmp	bx,ta_repeat_directive-ta_instruction_handler
	je	ta_parse_block
	cmp	bx,ta_while_directive-ta_instruction_handler
	je	ta_parse_block
	cmp	bx,ta_end_directive-ta_instruction_handler
	je	ta_parse_end_directive
	cmp	bx,ta_else_directive-ta_instruction_handler
	je	ta_parse_else
	cmp	bx,ta_assert_directive-ta_instruction_handler
	je	ta_parse_assert
      ta_common_parse:
	call	ta_parse_line_contents
	jmp	ta_parse_next_line
      ta_empty_instruction:
	lods	byte [esi]
	or	al,al
	jz	ta_parse_next_line
	cmp	al,':'
	je	ta_invalid_name
	dec	esi
	mov	[ta_parenthesis_stack],0
	call	ta_parse_argument
	jmp	ta_parse_next_line
      ta_empty_line:
	add	esi,16
      ta_skip_rest_of_line:
	call	ta_skip_foreign_line
      ta_parse_next_line:
	cmp	esi,[ta_source_start]
	jb	ta_parser_loop
      ta_source_parsed:
	cmp	[ta_blocks_stack],0
	je	ta_blocks_stack_ok
	pop	eax
	pop	[ta_current_line]
	jmp	ta_missing_end_directive
      ta_blocks_stack_ok:
	xor	al,al
	stos	byte [edi]
	add	edi,0Fh
	and	edi,not 0Fh
	mov	[ta_code_start],edi
	ret
      ta_parse_block:
	mov	eax,esp
	sub	eax,[ta_stack_limit]
	cmp	eax,100h
	jb	ta_stack_overflow
	push	[ta_current_line]
	mov	ax,bx
	shl	eax,16
	push	eax
	inc	[ta_blocks_stack]
	cmp	bx,ta_if_directive-ta_instruction_handler
	je	ta_parse_if
	cmp	bx,ta_while_directive-ta_instruction_handler
	je	ta_parse_while
	call	ta_parse_line_contents
	jmp	ta_parse_next_line
      ta_parse_end_directive:
	cmp	byte [esi],1Ah
	jne	ta_common_parse
	push	edi
	inc	esi
	movzx	ecx,byte [esi]
	inc	esi
	call	ta_get_instruction
	pop	edi
	jnc	ta_parse_end_block
	sub	esi,2
	jmp	ta_common_parse
      ta_parse_end_block:
	mov	dl,al
	mov	al,1
	stos	byte [edi]
	mov	ax,bx
	stos	word [edi]
	mov	al,dl
	stos	byte [edi]
	lods	byte [esi]
	or	al,al
	jnz	ta_extra_characters_on_line
	cmp	bx,ta_if_directive-ta_instruction_handler
	je	ta_close_parsing_block
	cmp	bx,ta_repeat_directive-ta_instruction_handler
	je	ta_close_parsing_block
	cmp	bx,ta_while_directive-ta_instruction_handler
	je	ta_close_parsing_block
	jmp	ta_parse_next_line
      ta_close_parsing_block:
	cmp	[ta_blocks_stack],0
	je	ta_unexpected_instruction
	cmp	bx,[esp+2]
	jne	ta_unexpected_instruction
	dec	[ta_blocks_stack]
	pop	eax edx
	cmp	bx,ta_if_directive-ta_instruction_handler
	jne	ta_parse_next_line
	test	al,1100b
	jz	ta_parse_next_line
	test	al,10000b
	jnz	ta_parse_next_line
	sub	edi,8
	jmp	ta_parse_next_line
      ta_parse_if:
	push	edi
	call	ta_parse_line_contents
	xor	al,al
	stos	byte [edi]
	xchg	esi,[esp]
	mov	edi,esi
	call	ta_preevaluate_logical_expression
	pop	esi
	cmp	al,'0'
	je	ta_parse_false_condition_block
	cmp	al,'1'
	je	ta_parse_true_condition_block
	or	byte [esp],10000b
	jmp	ta_parse_next_line
      ta_parse_while:
	push	edi
	call	ta_parse_line_contents
	xor	al,al
	stos	byte [edi]
	xchg	esi,[esp]
	mov	edi,esi
	call	ta_preevaluate_logical_expression
	pop	esi
	cmp	al,'0'
	je	ta_parse_false_condition_block
	cmp	al,'1'
	jne	ta_parse_next_line
	stos	byte [edi]
	jmp	ta_parse_next_line
      ta_parse_false_condition_block:
	or	byte [esp],1
	sub	edi,4
	jmp	ta_skip_parsing
      ta_parse_true_condition_block:
	or	byte [esp],100b
	sub	edi,4
	jmp	ta_parse_next_line
      ta_parse_else:
	cmp	[ta_blocks_stack],0
	je	ta_unexpected_instruction
	cmp	word [esp+2],ta_if_directive-ta_instruction_handler
	jne	ta_unexpected_instruction
	lods	byte [esi]
	or	al,al
	jz	ta_parse_pure_else
	cmp	al,1Ah
	jne	ta_extra_characters_on_line
	push	edi
	movzx	ecx,byte [esi]
	inc	esi
	call	ta_get_instruction
	jc	ta_extra_characters_on_line
	pop	edi
	cmp	bx,ta_if_directive-ta_instruction_handler
	jne	ta_extra_characters_on_line
	test	byte [esp],100b
	jnz	ta_skip_true_condition_else
	mov	dl,al
	mov	al,1
	stos	byte [edi]
	mov	ax,bx
	stos	word [edi]
	mov	al,dl
	stos	byte [edi]
	jmp	ta_parse_if
      ta_parse_assert:
	push	edi
	call	ta_parse_line_contents
	xor	al,al
	stos	byte [edi]
	xchg	esi,[esp]
	mov	edi,esi
	call	ta_preevaluate_logical_expression
	pop	esi
	or	al,al
	jz	ta_parse_next_line
	stos	byte [edi]
	jmp	ta_parse_next_line
      ta_skip_true_condition_else:
	sub	edi,4
	or	byte [esp],1
	jmp	ta_skip_parsing_contents
      ta_parse_pure_else:
	bts	dword [esp],1
	jc	ta_unexpected_instruction
	test	byte [esp],100b
	jz	ta_parse_next_line
	sub	edi,4
	or	byte [esp],1
	jmp	ta_skip_parsing
      ta_skip_parsing:
	cmp	esi,[ta_source_start]
	jae	ta_source_parsed
	mov	[ta_current_line],esi
	add	esi,16
      ta_skip_parsing_line:
	cmp	byte [esi],1Ah
	jne	ta_skip_parsing_contents
	inc	esi
	movzx	ecx,byte [esi]
	inc	esi
	cmp	byte [esi+ecx],':'
	je	ta_skip_parsing_label
	push	edi
	call	ta_get_instruction
	pop	edi
	jnc	ta_skip_parsing_instruction
	add	esi,ecx
	jmp	ta_skip_parsing_contents
      ta_skip_parsing_label:
	lea	esi,[esi+ecx+1]
	jmp	ta_skip_parsing_line
      ta_skip_parsing_instruction:
	cmp	bx,ta_if_directive-ta_instruction_handler
	je	ta_skip_parsing_block
	cmp	bx,ta_repeat_directive-ta_instruction_handler
	je	ta_skip_parsing_block
	cmp	bx,ta_while_directive-ta_instruction_handler
	je	ta_skip_parsing_block
	cmp	bx,ta_end_directive-ta_instruction_handler
	je	ta_skip_parsing_end_directive
	cmp	bx,ta_else_directive-ta_instruction_handler
	je	ta_skip_parsing_else
      ta_skip_parsing_contents:
	lods	byte [esi]
	or	al,al
	jz	ta_skip_parsing
	cmp	al,1Ah
	je	ta_skip_parsing_symbol
	cmp	al,3Bh
	je	ta_skip_parsing_symbol
	cmp	al,22h
	je	ta_skip_parsing_string
	jmp	ta_skip_parsing_contents
      ta_skip_parsing_symbol:
	lods	byte [esi]
	movzx	eax,al
	add	esi,eax
	jmp	ta_skip_parsing_contents
      ta_skip_parsing_string:
	lods	dword [esi]
	add	esi,eax
	jmp	ta_skip_parsing_contents
      ta_skip_parsing_block:
	mov	eax,esp
	sub	eax,[ta_stack_limit]
	cmp	eax,100h
	jb	ta_stack_overflow
	push	[ta_current_line]
	mov	ax,bx
	shl	eax,16
	push	eax
	inc	[ta_blocks_stack]
	jmp	ta_skip_parsing_contents
      ta_skip_parsing_end_directive:
	cmp	byte [esi],1Ah
	jne	ta_skip_parsing_contents
	push	edi
	inc	esi
	movzx	ecx,byte [esi]
	inc	esi
	call	ta_get_instruction
	pop	edi
	jnc	ta_skip_parsing_end_block
	add	esi,ecx
	jmp	ta_skip_parsing_contents
      ta_skip_parsing_end_block:
	lods	byte [esi]
	or	al,al
	jnz	ta_extra_characters_on_line
	cmp	bx,ta_if_directive-ta_instruction_handler
	je	ta_close_skip_parsing_block
	cmp	bx,ta_repeat_directive-ta_instruction_handler
	je	ta_close_skip_parsing_block
	cmp	bx,ta_while_directive-ta_instruction_handler
	je	ta_close_skip_parsing_block
	jmp	ta_skip_parsing
      ta_close_skip_parsing_block:
	cmp	[ta_blocks_stack],0
	je	ta_unexpected_instruction
	cmp	bx,[esp+2]
	jne	ta_unexpected_instruction
	dec	[ta_blocks_stack]
	pop	eax edx
	test	al,1
	jz	ta_skip_parsing
	cmp	bx,ta_if_directive-ta_instruction_handler
	jne	ta_parse_next_line
	test	al,10000b
	jz	ta_parse_next_line
	mov	al,0Fh
	stos	byte [edi]
	mov	eax,[ta_current_line]
	stos	dword [edi]
	inc	[ta_parsed_lines]
	mov	eax,1 + (ta_end_directive-ta_instruction_handler) shl 8
	stos	dword [edi]
	mov	eax,1 + (ta_if_directive-ta_instruction_handler) shl 8
	stos	dword [edi]
	jmp	ta_parse_next_line
      ta_skip_parsing_else:
	cmp	[ta_blocks_stack],0
	je	ta_unexpected_instruction
	cmp	word [esp+2],ta_if_directive-ta_instruction_handler
	jne	ta_unexpected_instruction
	lods	byte [esi]
	or	al,al
	jz	ta_skip_parsing_pure_else
	cmp	al,1Ah
	jne	ta_extra_characters_on_line
	push	edi
	movzx	ecx,byte [esi]
	inc	esi
	call	ta_get_instruction
	jc	ta_extra_characters_on_line
	pop	edi
	cmp	bx,ta_if_directive-ta_instruction_handler
	jne	ta_extra_characters_on_line
	mov	al,[esp]
	test	al,1
	jz	ta_skip_parsing_contents
	test	al,100b
	jnz	ta_skip_parsing_contents
	test	al,10000b
	jnz	ta_parse_else_if
	xor	al,al
	mov	[esp],al
	mov	al,0Fh
	stos	byte [edi]
	mov	eax,[ta_current_line]
	stos	dword [edi]
	inc	[ta_parsed_lines]
      ta_parse_else_if:
	mov	eax,1 + (ta_if_directive-ta_instruction_handler) shl 8
	stos	dword [edi]
	jmp	ta_parse_if
      ta_skip_parsing_pure_else:
	bts	dword [esp],1
	jc	ta_unexpected_instruction
	mov	al,[esp]
	test	al,1
	jz	ta_skip_parsing
	test	al,100b
	jnz	ta_skip_parsing
	and	al,not 1
	or	al,1000b
	mov	[esp],al
	jmp	ta_parse_next_line

ta_parse_line_contents:
	mov	[ta_parenthesis_stack],0
      ta_parse_instruction_arguments:
	cmp	bx,ta_prefix_instruction-ta_instruction_handler
	je	ta_allow_embedded_instruction
	cmp	bx,ta_times_directive-ta_instruction_handler
	je	ta_parse_times_directive
	cmp	bx,ta_end_directive-ta_instruction_handler
	je	ta_allow_embedded_instruction
	cmp	bx,ta_label_directive-ta_instruction_handler
	je	ta_parse_label_directive
	cmp	bx,ta_segment_directive-ta_instruction_handler
	je	ta_parse_segment_directive
	cmp	bx,ta_load_directive-ta_instruction_handler
	je	ta_parse_load_directive
	cmp	bx,ta_extrn_directive-ta_instruction_handler
	je	ta_parse_extrn_directive
	cmp	bx,ta_public_directive-ta_instruction_handler
	je	ta_parse_public_directive
	cmp	bx,ta_section_directive-ta_instruction_handler
	je	ta_parse_formatter_argument
	cmp	bx,ta_format_directive-ta_instruction_handler
	je	ta_parse_formatter_argument
	cmp	bx,ta_data_directive-ta_instruction_handler
	je	ta_parse_formatter_argument
	jmp	ta_parse_argument
      ta_parse_formatter_argument:
	or	[ta_formatter_symbols_allowed],-1
      ta_parse_argument:
	lea	eax,[edi+100h]
	cmp	eax,[ta_labels_list]
	jae	ta_out_of_memory
	lods	byte [esi]
	cmp	al,':'
	je	ta_instruction_separator
	cmp	al,','
	je	ta_separator
	cmp	al,'='
	je	ta_expression_comparator
	cmp	al,'|'
	je	ta_separator
	cmp	al,'&'
	je	ta_separator
	cmp	al,'~'
	je	ta_separator
	cmp	al,'>'
	je	ta_greater
	cmp	al,'<'
	je	ta_less
	cmp	al,')'
	je	ta_close_parenthesis
	or	al,al
	jz	ta_contents_parsed
	cmp	al,'['
	je	ta_address_argument
	cmp	al,']'
	je	ta_separator
	cmp	al,'{'
	je	ta_open_decorator
	cmp	al,'}'
	je	ta_close_decorator
	cmp	al,'#'
	je	ta_unallowed_character
	cmp	al,'`'
	je	ta_unallowed_character
	cmp	al,3Bh
	je	ta_foreign_argument
	cmp	[ta_decorator_symbols_allowed],0
	je	ta_not_a_separator
	cmp	al,'-'
	je	ta_separator
      ta_not_a_separator:
	dec	esi
	cmp	al,1Ah
	jne	ta_expression_argument
	push	edi
	mov	edi,ta_directive_operators
	call	ta_get_operator
	or	al,al
	jnz	ta_operator_argument
	inc	esi
	movzx	ecx,byte [esi]
	inc	esi
	call	ta_get_symbol
	jnc	ta_symbol_argument
	cmp	ecx,1
	jne	ta_check_argument
	cmp	byte [esi],'?'
	jne	ta_check_argument
	pop	edi
	movs	byte [edi],[esi]
	jmp	ta_argument_parsed
      ta_foreign_argument:
	dec	esi
	call	ta_skip_foreign_line
	jmp	ta_contents_parsed
      ta_symbol_argument:
	pop	edi
	stos	word [edi]
	cmp	byte [esi],'+'
	jne	ta_argument_parsed
	and	ax,0F0FFh
	cmp	ax,6010h
	jne	ta_argument_parsed
	movs	byte [edi],[esi]
	jmp	ta_argument_parsed
      ta_operator_argument:
	pop	edi
	cmp	al,85h
	je	ta_ptr_argument
	stos	byte [edi]
	cmp	al,8Ch
	je	ta_forced_expression
	cmp	al,81h
	je	ta_forced_parenthesis
	cmp	al,80h
	je	ta_parse_at_operator
	cmp	al,82h
	je	ta_parse_from_operator
	cmp	al,89h
	je	ta_parse_label_operator
	cmp	al,0F8h
	je	ta_forced_expression
	jmp	ta_argument_parsed
      ta_instruction_separator:
	stos	byte [edi]
      ta_allow_embedded_instruction:
	cmp	byte [esi],1Ah
	jne	ta_parse_argument
	push	edi
	inc	esi
	movzx	ecx,byte [esi]
	inc	esi
	call	ta_get_instruction
	jnc	ta_embedded_instruction
	call	ta_get_data_directive
	jnc	ta_embedded_instruction
	pop	edi
	sub	esi,2
	jmp	ta_parse_argument
      ta_embedded_instruction:
	pop	edi
	mov	dl,al
	mov	al,1
	stos	byte [edi]
	mov	ax,bx
	stos	word [edi]
	mov	al,dl
	stos	byte [edi]
	jmp	ta_parse_instruction_arguments
      ta_parse_times_directive:
	mov	al,'('
	stos	byte [edi]
	call	ta_convert_expression
	mov	al,')'
	stos	byte [edi]
	cmp	byte [esi],':'
	jne	ta_allow_embedded_instruction
	movs	byte [edi],[esi]
	jmp	ta_allow_embedded_instruction
      ta_parse_segment_directive:
	or	[ta_formatter_symbols_allowed],-1
      ta_parse_label_directive:
	cmp	byte [esi],1Ah
	jne	ta_argument_parsed
	push	esi
	inc	esi
	movzx	ecx,byte [esi]
	inc	esi
	call	ta_identify_label
	pop	ebx
	cmp	eax,0Fh
	je	ta_non_label_identified
	mov	byte [edi],2
	inc	edi
	stos	dword [edi]
	xor	al,al
	stos	byte [edi]
	jmp	ta_argument_parsed
      ta_non_label_identified:
	mov	esi,ebx
	jmp	ta_argument_parsed
      ta_parse_load_directive:
	cmp	byte [esi],1Ah
	jne	ta_argument_parsed
	push	esi
	inc	esi
	movzx	ecx,byte [esi]
	inc	esi
	call	ta_get_label_id
	pop	ebx
	cmp	eax,0Fh
	je	ta_non_label_identified
	mov	byte [edi],2
	inc	edi
	stos	dword [edi]
	xor	al,al
	stos	byte [edi]
	jmp	ta_argument_parsed
      ta_parse_public_directive:
	cmp	byte [esi],1Ah
	jne	ta_parse_argument
	inc	esi
	push	esi
	movzx	ecx,byte [esi]
	inc	esi
	push	esi ecx
	push	edi
	or	[ta_formatter_symbols_allowed],-1
	call	ta_get_symbol
	mov	[ta_formatter_symbols_allowed],0
	pop	edi
	jc	ta_parse_public_label
	cmp	al,1Dh
	jne	ta_parse_public_label
	add	esp,12
	stos	word [edi]
	jmp	ta_parse_public_directive
      ta_parse_public_label:
	pop	ecx esi
	mov	al,2
	stos	byte [edi]
	call	ta_get_label_id
	stos	dword [edi]
	mov	ax,8600h
	stos	word [edi]
	pop	ebx
	push	ebx esi edi
	mov	edi,ta_directive_operators
	call	ta_get_operator
	pop	edi edx ebx
	cmp	al,86h
	je	ta_argument_parsed
	mov	esi,edx
	xchg	esi,ebx
	movzx	ecx,byte [esi]
	inc	esi
	mov	ax,'('
	stos	word [edi]
	mov	eax,ecx
	stos	dword [edi]
	rep	movs byte [edi],[esi]
	xor	al,al
	stos	byte [edi]
	xchg	esi,ebx
	jmp	ta_argument_parsed
      ta_parse_extrn_directive:
	cmp	byte [esi],22h
	je	ta_parse_quoted_extrn
	cmp	byte [esi],1Ah
	jne	ta_parse_argument
	push	esi
	movzx	ecx,byte [esi+1]
	add	esi,2
	mov	ax,'('
	stos	word [edi]
	mov	eax,ecx
	stos	dword [edi]
	rep	movs byte [edi],[esi]
	mov	ax,8600h
	stos	word [edi]
	pop	esi
      ta_parse_label_operator:
	cmp	byte [esi],1Ah
	jne	ta_argument_parsed
	inc	esi
	movzx	ecx,byte [esi]
	inc	esi
	mov	al,2
	stos	byte [edi]
	call	ta_get_label_id
	stos	dword [edi]
	xor	al,al
	stos	byte [edi]
	jmp	ta_argument_parsed
      ta_parse_from_operator:
	cmp	byte [esi],22h
	je	ta_argument_parsed
      ta_parse_at_operator:
	cmp	byte [esi],':'
	je	ta_argument_parsed
	jmp	ta_forced_multipart_expression
      ta_parse_quoted_extrn:
	inc	esi
	mov	ax,'('
	stos	word [edi]
	lods	dword [esi]
	mov	ecx,eax
	stos	dword [edi]
	rep	movs byte [edi],[esi]
	xor	al,al
	stos	byte [edi]
	push	esi edi
	mov	edi,ta_directive_operators
	call	ta_get_operator
	mov	edx,esi
	pop	edi esi
	cmp	al,86h
	jne	ta_argument_parsed
	stos	byte [edi]
	mov	esi,edx
	jmp	ta_parse_label_operator
      ta_ptr_argument:
	call	ta_parse_address
	jmp	ta_address_parsed
      ta_check_argument:
	push	esi ecx
	sub	esi,2
	mov	edi,ta_single_operand_operators
	call	ta_get_operator
	pop	ecx esi
	or	al,al
	jnz	ta_not_instruction
	call	ta_get_instruction
	jnc	ta_embedded_instruction
	call	ta_get_data_directive
	jnc	ta_embedded_instruction
      ta_not_instruction:
	pop	edi
	sub	esi,2
      ta_expression_argument:
	cmp	byte [esi],22h
	jne	ta_not_string
	mov	eax,[esi+1]
	lea	ebx,[esi+5+eax]
	push	ebx ecx esi edi
	call	ta_parse_expression
	pop	eax edx ecx ebx
	cmp	esi,ebx
	jne	ta_expression_argument_parsed
	mov	edi,eax
	mov	esi,edx
      ta_string_argument:
	inc	esi
	mov	ax,'('
	stos	word [edi]
	lods	dword [esi]
	mov	ecx,eax
	stos	dword [edi]
	shr	ecx,1
	jnc	ta_string_movsb_ok
	movs	byte [edi],[esi]
      ta_string_movsb_ok:
	shr	ecx,1
	jnc	ta_string_movsw_ok
	movs	word [edi],[esi]
      ta_string_movsw_ok:
	rep	movs dword [edi],[esi]
	xor	al,al
	stos	byte [edi]
	jmp	ta_expression_argument_parsed
      ta_parse_expression:
	mov	al,'('
	stos	byte [edi]
	call	ta_convert_expression
	mov	al,')'
	stos	byte [edi]
	ret
      ta_not_string:
	cmp	byte [esi],'('
	jne	ta_expression
	mov	eax,esp
	sub	eax,[ta_stack_limit]
	cmp	eax,100h
	jb	ta_stack_overflow
	push	esi edi
	inc	esi
	mov	al,91h
	stos	byte [edi]
	inc	[ta_parenthesis_stack]
	jmp	ta_parse_argument
      ta_expression_comparator:
	stos	byte [edi]
	jmp	ta_forced_expression
      ta_greater:
	cmp	byte [esi],'='
	jne	ta_separator
	inc	esi
	mov	al,0F2h
	jmp	ta_separator
      ta_less:
	cmp	byte [edi-1],0F6h
	je	ta_separator
	cmp	byte [esi],'>'
	je	ta_not_equal
	cmp	byte [esi],'='
	jne	ta_separator
	inc	esi
	mov	al,0F3h
	jmp	ta_separator
      ta_not_equal:
	inc	esi
	mov	al,0F1h
	jmp	ta_expression_comparator
      ta_expression:
	call	ta_parse_expression
	jmp	ta_expression_argument_parsed
      ta_forced_expression:
	xor	al,al
	xchg	al,[ta_formatter_symbols_allowed]
	push	eax
	call	ta_parse_expression
      ta_forced_expression_parsed:
	pop	eax
	mov	[ta_formatter_symbols_allowed],al
	jmp	ta_argument_parsed
      ta_forced_multipart_expression:
	xor	al,al
	xchg	al,[ta_formatter_symbols_allowed]
	push	eax
	call	ta_parse_expression
	cmp	byte [esi],':'
	jne	ta_forced_expression_parsed
	movs	byte [edi],[esi]
	call	ta_parse_expression
	jmp	ta_forced_expression_parsed
      ta_address_argument:
	call	ta_parse_address
	lods	byte [esi]
	cmp	al,']'
	je	ta_address_parsed
	cmp	al,','
	je	ta_divided_address
	dec	esi
	mov	al,')'
	stos	byte [edi]
	jmp	ta_argument_parsed
      ta_divided_address:
	mov	ax,'),'
	stos	word [edi]
	jmp	ta_expression
      ta_address_parsed:
	mov	al,']'
	stos	byte [edi]
	jmp	ta_argument_parsed
      ta_parse_address:
	mov	al,'['
	stos	byte [edi]
	cmp	word [esi],021Ah
	jne	ta_convert_address
	push	esi
	add	esi,4
	lea	ebx,[esi+1]
	cmp	byte [esi],':'
	pop	esi
	jne	ta_convert_address
	add	esi,2
	mov	ecx,2
	push	ebx edi
	call	ta_get_symbol
	pop	edi esi
	jc	ta_unknown_segment_prefix
	cmp	al,10h
	jne	ta_unknown_segment_prefix
	mov	al,ah
	and	ah,11110000b
	cmp	ah,30h
	jne	ta_unknown_segment_prefix
	add	al,30h
	stos	byte [edi]
	jmp	ta_convert_address
      ta_unknown_segment_prefix:
	sub	esi,5
      ta_convert_address:
	push	edi
	mov	edi,ta_address_sizes
	call	ta_get_operator
	pop	edi
	or	al,al
	jz	ta_convert_expression
	add	al,70h
	stos	byte [edi]
	jmp	ta_convert_expression
      ta_forced_parenthesis:
	cmp	byte [esi],'('
	jne	ta_argument_parsed
	inc	esi
	mov	al,91h
	jmp	ta_separator
      ta_unallowed_character:
	mov	al,0FFh
	jmp	ta_separator
      ta_open_decorator:
	inc	[ta_decorator_symbols_allowed]
	jmp	ta_separator
      ta_close_decorator:
	dec	[ta_decorator_symbols_allowed]
	jmp	ta_separator
      ta_close_parenthesis:
	mov	al,92h
      ta_separator:
	stos	byte [edi]
      ta_argument_parsed:
	cmp	[ta_parenthesis_stack],0
	je	ta_parse_argument
	dec	[ta_parenthesis_stack]
	add	esp,8
	jmp	ta_argument_parsed
      ta_expression_argument_parsed:
	cmp	[ta_parenthesis_stack],0
	je	ta_parse_argument
	cmp	byte [esi],')'
	jne	ta_argument_parsed
	dec	[ta_parenthesis_stack]
	pop	edi esi
	jmp	ta_expression
      ta_contents_parsed:
	cmp	[ta_parenthesis_stack],0
	je	ta_contents_ok
	dec	[ta_parenthesis_stack]
	add	esp,8
	jmp	ta_contents_parsed
      ta_contents_ok:
	ret

ta_identify_label:
	cmp	byte [esi],'.'
	je	ta_local_label_name
	call	ta_get_label_id
	cmp	eax,10h
	jb	ta_label_identified
	or	ebx,ebx
	jz	ta_anonymous_label_name
	dec	ebx
	mov	[ta_current_locals_prefix],ebx
      ta_label_identified:
	ret
      ta_anonymous_label_name:
	cmp	byte [esi-1],'@'
	je	ta_anonymous_label_name_ok
	mov	eax,0Fh
      ta_anonymous_label_name_ok:
	ret
      ta_local_label_name:
	call	ta_get_label_id
	ret

ta_get_operator:
	cmp	byte [esi],1Ah
	jne	ta_get_simple_operator
	mov	edx,esi
	push	ebp
	inc	esi
	lods	byte [esi]
	movzx	ebp,al
	push	edi
	mov	ecx,ebp
	call	ta_lower_case
	pop	edi
      ta_check_operator:
	mov	esi,ta_converted
	movzx	ecx,byte [edi]
	jecxz	ta_no_operator
	inc	edi
	mov	ebx,edi
	add	ebx,ecx
	cmp	ecx,ebp
	jne	ta_next_operator
	repe	cmps byte [esi],[edi]
	je	ta_operator_found
	jb	ta_no_operator
      ta_next_operator:
	mov	edi,ebx
	inc	edi
	jmp	ta_check_operator
      ta_no_operator:
	mov	esi,edx
	mov	ecx,ebp
	pop	ebp
      ta_no_simple_operator:
	xor	al,al
	ret
      ta_operator_found:
	lea	esi,[edx+2+ebp]
	mov	ecx,ebp
	pop	ebp
	mov	al,[edi]
	ret
      ta_get_simple_operator:
	mov	al,[esi]
	cmp	al,22h
	je	ta_no_simple_operator
      ta_simple_operator:
	cmp	byte [edi],1
	jb	ta_no_simple_operator
	ja	ta_simple_next_operator
	cmp	al,[edi+1]
	je	ta_simple_operator_found
      ta_simple_next_operator:
	movzx	ecx,byte [edi]
	lea	edi,[edi+1+ecx+1]
	jmp	ta_simple_operator
      ta_simple_operator_found:
	inc	esi
	mov	al,[edi+2]
	ret

ta_get_symbol:
	push	esi
	mov	ebp,ecx
	call	ta_lower_case
	mov	ecx,ebp
	cmp	cl,11
	ja	ta_no_symbol
	sub	cl,1
	jc	ta_no_symbol
	movzx	ebx,word [ta_symbols+ecx*4]
	add	ebx,ta_symbols
	movzx	edx,word [ta_symbols+ecx*4+2]
      ta_scan_symbols:
	or	edx,edx
	jz	ta_no_symbol
	mov	eax,edx
	shr	eax,1
	lea	edi,[ebp+2]
	imul	eax,edi
	lea	edi,[ebx+eax]
	mov	esi,ta_converted
	mov	ecx,ebp
	repe	cmps byte [esi],[edi]
	ja	ta_symbols_up
	jb	ta_symbols_down
	mov	ax,[edi]
	cmp	al,18h
	jb	ta_symbol_ok
	cmp	al,1Fh
	je	ta_decorator_symbol
	cmp	[ta_formatter_symbols_allowed],0
	je	ta_no_symbol
      ta_symbol_ok:
	pop	esi
	add	esi,ebp
	clc
	ret
      ta_decorator_symbol:
	cmp	[ta_decorator_symbols_allowed],0
	jne	ta_symbol_ok
      ta_no_symbol:
	pop	esi
	mov	ecx,ebp
	stc
	ret
      ta_symbols_down:
	shr	edx,1
	jmp	ta_scan_symbols
      ta_symbols_up:
	lea	ebx,[edi+ecx+2]
	shr	edx,1
	adc	edx,-1
	jmp	ta_scan_symbols

ta_get_data_directive:
	push	esi
	mov	ebp,ecx
	call	ta_lower_case
	mov	ecx,ebp
	cmp	cl,4
	ja	ta_no_instruction
	sub	cl,2
	jc	ta_no_instruction
	movzx	ebx,word [ta_data_directives+ecx*4]
	add	ebx,ta_data_directives
	movzx	edx,word [ta_data_directives+ecx*4+2]
	jmp	ta_scan_instructions

ta_get_instruction:
	push	esi
	mov	ebp,ecx
	call	ta_lower_case
	mov	ecx,ebp
	cmp	cl,17
	ja	ta_no_instruction
	sub	cl,2
	jc	ta_no_instruction
	movzx	ebx,word [ta_instructions+ecx*4]
	add	ebx,ta_instructions
	movzx	edx,word [ta_instructions+ecx*4+2]
      ta_scan_instructions:
	or	edx,edx
	jz	ta_no_instruction
	mov	eax,edx
	shr	eax,1
	lea	edi,[ebp+3]
	imul	eax,edi
	lea	edi,[ebx+eax]
	mov	esi,ta_converted
	mov	ecx,ebp
	repe	cmps byte [esi],[edi]
	ja	ta_instructions_up
	jb	ta_instructions_down
	pop	esi
	add	esi,ebp
	mov	al,[edi]
	mov	bx,[edi+1]
	clc
	ret
      ta_no_instruction:
	pop	esi
	mov	ecx,ebp
	stc
	ret
      ta_instructions_down:
	shr	edx,1
	jmp	ta_scan_instructions
      ta_instructions_up:
	lea	ebx,[edi+ecx+3]
	shr	edx,1
	adc	edx,-1
	jmp	ta_scan_instructions

ta_get_label_id:
	cmp	ecx,100h
	jae	ta_name_too_long
	cmp	byte [esi],'@'
	je	ta_anonymous_label
	cmp	byte [esi],'.'
	jne	ta_standard_label
	cmp	byte [esi+1],'.'
	je	ta_standard_label
	cmp	[ta_current_locals_prefix],0
	je	ta_standard_label
	push	edi
	mov	edi,[ta_additional_memory_end]
	promote_edi
	sub	edi,2
	sub	edi,ecx
	push	ecx esi
	mov	esi,[ta_current_locals_prefix]
	promote_esi
	lods	byte [esi]
	movzx	ecx,al
	sub	edi,ecx
	cmp	edi,[ta_free_additional_memory]
	jb	ta_out_of_memory
	mov	word [edi],0
	add	edi,2
	mov	ebx,edi
	rep	movs byte [edi],[esi]
	pop	esi ecx
	add	al,cl
	jc	ta_name_too_long
	rep	movs byte [edi],[esi]
	pop	edi
	push	ebx esi
	movzx	ecx,al
	mov	byte [ebx-1],al
	mov	esi,ebx
	call	ta_get_label_id
	pop	esi ebx
	cmp	ebx,[eax+24]
	jne	ta_composed_label_id_ok
	lea	edx,[ebx-2]
	mov	[ta_additional_memory_end],edx
      ta_composed_label_id_ok:
	ret
      ta_anonymous_label:
	cmp	ecx,2
	jne	ta_standard_label
	mov	al,[esi+1]
	mov	ebx,ta_characters
	xlat	byte [ebx]
	cmp	al,'@'
	je	ta_new_anonymous
	cmp	al,'b'
	je	ta_anonymous_back
	cmp	al,'r'
	je	ta_anonymous_back
	cmp	al,'f'
	jne	ta_standard_label
	add	esi,2
	mov	eax,[ta_anonymous_forward]
	or	eax,eax
	jnz	ta_anonymous_ok
	mov	eax,[ta_current_line]
	mov	[ta_error_line],eax
	call	ta_allocate_label
	mov	[ta_anonymous_forward],eax
      ta_anonymous_ok:
	xor	ebx,ebx
	ret
      ta_anonymous_back:
	mov	eax,[ta_anonymous_reverse]
	add	esi,2
	or	eax,eax
	jz	ta_bogus_anonymous
	jmp	ta_anonymous_ok
      ta_bogus_anonymous:
	call	ta_allocate_label
	mov	[ta_anonymous_reverse],eax
	jmp	ta_anonymous_ok
      ta_new_anonymous:
	add	esi,2
	mov	eax,[ta_anonymous_forward]
	or	eax,eax
	jnz	ta_new_anonymous_ok
	call	ta_allocate_label
      ta_new_anonymous_ok:
	mov	[ta_anonymous_reverse],eax
	mov	[ta_anonymous_forward],0
	jmp	ta_anonymous_ok
      ta_standard_label:
	cmp	byte [esi],'%'
	je	ta_get_predefined_id
	cmp	byte [esi],'$'
	je	ta_current_address_label
	cmp	byte [esi],'?'
	jne	ta_find_label
	cmp	ecx,1
	jne	ta_find_label
	inc	esi
	mov	eax,0Fh
	ret
      ta_current_address_label:
	cmp	ecx,3
	je	ta_current_address_label_3_characters
	ja	ta_find_label
	inc	esi
	cmp	ecx,1
	jbe	ta_get_current_offset_id
	inc	esi
	cmp	byte [esi-1],'$'
	je	ta_get_org_origin_id
	cmp	byte [esi-1],'%'
	je	ta_get_file_offset_id
	sub	esi,2
	jmp	ta_find_label
      ta_get_current_offset_id:
	xor	eax,eax
	ret
      ta_get_counter_id:
	mov	eax,1
	ret
      ta_get_timestamp_id:
	mov	eax,2
	ret
      ta_get_org_origin_id:
	mov	eax,3
	ret
      ta_get_file_offset_id:
	mov	eax,4
	ret
      ta_current_address_label_3_characters:
	cmp	word [esi+1],'%%'
	jne	ta_find_label
	add	esi,3
      ta_get_actual_file_offset_id:
	mov	eax,5
	ret
      ta_get_predefined_id:
	cmp	ecx,2
	ja	ta_find_label
	inc	esi
	cmp	cl,1
	je	ta_get_counter_id
	lods	byte [esi]
	mov	ebx,ta_characters
	xlat	[ebx]
	cmp	al,'t'
	je	ta_get_timestamp_id
	sub	esi,2
      ta_find_label:
	xor	ebx,ebx
	mov	eax,2166136261
	mov	ebp,16777619
      ta_hash_label:
	xor	al,[esi+ebx]
	mul	ebp
	inc	bl
	cmp	bl,cl
	jb	ta_hash_label
	mov	ebp,eax
	shl	eax,8
	and	ebp,0FFh shl 24
	xor	ebp,eax
	or	ebp,ebx
	mov	[ta_label_hash],ebp
	push	edi esi
	push	ecx
	mov	ecx,32
	mov	ebx,ta_hash_tree
      ta_follow_tree:
	mov	edx,[ebx]
	or	edx,edx
	jz	ta_extend_tree
	xor	eax,eax
	shl	ebp,1
	adc	eax,0
	lea	ebx,[edx+eax*4]
	dec	ecx
	jnz	ta_follow_tree
	mov	[ta_label_leaf],ebx
	pop	edx
	mov	eax,[ebx]
	or	eax,eax
	jz	ta_add_label
	mov	ebx,esi
	mov	ebp,[ta_label_hash]
      ta_compare_labels:
	mov	esi,ebx
	mov	ecx,edx
	mov	edi,[eax+4]
	mov	edi,[edi+24]
	repe	cmps byte [esi],[edi]
	je	ta_label_found
	mov	eax,[eax]
	or	eax,eax
	jnz	ta_compare_labels
	jmp	ta_add_label
      ta_label_found:
	add	esp,4
	pop	edi
	mov	eax,[eax+4]
	ret
      ta_extend_tree:
	mov	edx,[ta_free_additional_memory]
	promote_edx
	lea	eax,[edx+8]
	cmp	eax,[ta_additional_memory_end]
	ja	ta_out_of_memory
	mov	[ta_free_additional_memory],eax
	xor	eax,eax
	mov	[edx],eax
	mov	[edx+4],eax
	shl	ebp,1
	adc	eax,0
	mov	[ebx],edx
	lea	ebx,[edx+eax*4]
	dec	ecx
	jnz	ta_extend_tree
	mov	[ta_label_leaf],ebx
	pop	edx
      ta_add_label:
	mov	ecx,edx
	pop	esi
	cmp	byte [esi-2],0
	je	ta_label_name_ok
	mov	al,[esi]
	cmp	al,30h
	jb	ta_name_first_char_ok
	cmp	al,39h
	jbe	ta_numeric_name
      ta_name_first_char_ok:
	cmp	al,'$'
	jne	ta_check_for_reserved_word
      ta_numeric_name:
	add	esi,ecx
      ta_reserved_word:
	mov	eax,0Fh
	pop	edi
	ret
      ta_check_for_reserved_word:
	call	ta_get_instruction
	jnc	ta_reserved_word
	call	ta_get_data_directive
	jnc	ta_reserved_word
	call	ta_get_symbol
	jnc	ta_reserved_word
	sub	esi,2
	mov	edi,ta_operators
	call	ta_get_operator
	or	al,al
	jnz	ta_reserved_word
	mov	edi,ta_single_operand_operators
	call	ta_get_operator
	or	al,al
	jnz	ta_reserved_word
	mov	edi,ta_directive_operators
	call	ta_get_operator
	or	al,al
	jnz	ta_reserved_word
	inc	esi
	movzx	ecx,byte [esi]
	inc	esi
      ta_label_name_ok:
	mov	edx,[ta_free_additional_memory]
	promote_edx
	lea	eax,[edx+8]
	cmp	eax,[ta_additional_memory_end]
	ja	ta_out_of_memory
	mov	[ta_free_additional_memory],eax
	mov	ebx,esi
	add	esi,ecx
	mov	eax,[ta_label_leaf]
	mov	edi,[eax]
	mov	[edx],edi
	mov	[eax],edx
	call	ta_allocate_label
	mov	[edx+4],eax
	mov	[eax+24],ebx
	pop	edi
	ret
      ta_allocate_label:
	mov	eax,[ta_labels_list]
	mov	ecx,LABEL_STRUCTURE_SIZE shr 2
      ta_initialize_label:
	sub	eax,4
	mov	dword [eax],0
	loop	ta_initialize_label
	mov	[ta_labels_list],eax
	ret

LABEL_STRUCTURE_SIZE = 32
