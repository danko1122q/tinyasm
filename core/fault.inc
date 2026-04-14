ta_out_of_memory:
	push	_ta_out_of_memory
	jmp	ta_fatal_error
ta_stack_overflow:
	push	_ta_stack_overflow
	jmp	ta_fatal_error
ta_main_file_not_found:
	push	_ta_main_file_not_found
	jmp	ta_fatal_error
ta_write_failed:
	push	_ta_write_failed
	jmp	ta_fatal_error

ta_code_cannot_be_generated:
	push	_ta_code_cannot_be_generated
	jmp	ta_general_error
ta_format_limitations_exceeded:
	push	_ta_format_limitations_exceeded
	jmp	ta_general_error
ta_invalid_definition:
	push	_ta_invalid_definition
    ta_general_error:
	cmp	[ta_symbols_file],0
	je	ta_fatal_error
	call	ta_dump_preprocessed_source
	jmp	ta_fatal_error

ta_file_not_found:
	push	_ta_file_not_found
	jmp	ta_error_with_source
ta_error_reading_file:
	push	_ta_error_reading_file
	jmp	ta_error_with_source
ta_invalid_file_format:
	push	_ta_invalid_file_format
	jmp	ta_error_with_source
ta_invalid_macro_arguments:
	push	_ta_invalid_macro_arguments
	jmp	ta_error_with_source
ta_incomplete_macro:
	push	_ta_incomplete_macro
	jmp	ta_error_with_source
ta_unexpected_characters:
	push	_ta_unexpected_characters
	jmp	ta_error_with_source
ta_invalid_argument:
	push	_ta_invalid_argument
	jmp	ta_error_with_source
ta_illegal_instruction:
	push	_ta_illegal_instruction
	jmp	ta_error_with_source
ta_invalid_operand:
	push	_ta_invalid_operand
	jmp	ta_error_with_source
ta_invalid_operand_size:
	push	_ta_invalid_operand_size
	jmp	ta_error_with_source
ta_operand_size_not_specified:
	push	_ta_operand_size_not_specified
	jmp	ta_error_with_source
ta_operand_sizes_do_not_match:
	push	_ta_operand_sizes_do_not_match
	jmp	ta_error_with_source
ta_invalid_address_size:
	push	_ta_invalid_address_size
	jmp	ta_error_with_source
ta_address_sizes_do_not_agree:
	push	_ta_address_sizes_do_not_agree
	jmp	ta_error_with_source
ta_disallowed_combination_of_registers:
	push	_ta_disallowed_combination_of_registers
	jmp	ta_error_with_source
ta_long_immediate_not_encodable:
	push	_ta_long_immediate_not_encodable
	jmp	ta_error_with_source
ta_relative_jump_out_of_range:
	push	_ta_relative_jump_out_of_range
	jmp	ta_error_with_source
ta_invalid_expression:
	push	_ta_invalid_expression
	jmp	ta_error_with_source
ta_invalid_address:
	push	_ta_invalid_address
	jmp	ta_error_with_source
ta_invalid_value:
	push	_ta_invalid_value
	jmp	ta_error_with_source
ta_value_out_of_range:
	push	_ta_value_out_of_range
	jmp	ta_error_with_source
ta_undefined_symbol:
	mov	edi,ta_message
	mov	esi,_ta_undefined_symbol
	call	ta_copy_asciiz
	push	ta_message
	cmp	[ta_error_info],0
	je	ta_error_with_source
	mov	esi,[ta_error_info]
	mov	esi,[esi+24]
	or	esi,esi
	jz	ta_error_with_source
	mov	byte [edi-1],20h
	call	ta_write_quoted_symbol_name
	jmp	ta_error_with_source
    ta_copy_asciiz:
	lods	byte [esi]
	stos	byte [edi]
	test	al,al
	jnz	ta_copy_asciiz
	ret
    ta_write_quoted_symbol_name:
	mov	al,27h
	stosb
	movzx	ecx,byte [esi-1]
	rep	movs byte [edi],[esi]
	mov	ax,27h
	stosw
	ret
ta_symbol_out_of_scope:
	mov	edi,ta_message
	mov	esi,_ta_symbol_out_of_scope_1
	call	ta_copy_asciiz
	cmp	[ta_error_info],0
	je	ta_finish_symbol_out_of_scope_message
	mov	esi,[ta_error_info]
	mov	esi,[esi+24]
	or	esi,esi
	jz	ta_finish_symbol_out_of_scope_message
	mov	byte [edi-1],20h
	call	ta_write_quoted_symbol_name
    ta_finish_symbol_out_of_scope_message:
	mov	byte [edi-1],20h
	mov	esi,_ta_symbol_out_of_scope_2
	call	ta_copy_asciiz
	push	ta_message
	jmp	ta_error_with_source
ta_invalid_use_of_symbol:
	push	_ta_invalid_use_of_symbol
	jmp	ta_error_with_source
ta_name_too_long:
	push	_ta_name_too_long
	jmp	ta_error_with_source
ta_invalid_name:
	push	_ta_invalid_name
	jmp	ta_error_with_source
ta_reserved_word_used_as_symbol:
	push	_ta_reserved_word_used_as_symbol
	jmp	ta_error_with_source
ta_symbol_already_defined:
	push	_ta_symbol_already_defined
	jmp	ta_error_with_source
ta_missing_end_quote:
	push	_ta_missing_end_quote
	jmp	ta_error_with_source
ta_missing_end_directive:
	push	_ta_missing_end_directive
	jmp	ta_error_with_source
ta_unexpected_instruction:
	push	_ta_unexpected_instruction
	jmp	ta_error_with_source
ta_extra_characters_on_line:
	push	_ta_extra_characters_on_line
	jmp	ta_error_with_source
ta_section_not_aligned_enough:
	push	_ta_section_not_aligned_enough
	jmp	ta_error_with_source
ta_setting_already_specified:
	push	_ta_setting_already_specified
	jmp	ta_error_with_source
ta_data_already_defined:
	push	_ta_data_already_defined
	jmp	ta_error_with_source
ta_too_many_repeats:
	push	_ta_too_many_repeats
	jmp	ta_error_with_source
ta_assertion_failed:
	push	_ta_assertion_failed
	jmp	ta_error_with_source
ta_invoked_error:
	push	_ta_invoked_error
    ta_error_with_source:
	cmp	[ta_symbols_file],0
	je	ta_assembler_error
	call	ta_dump_preprocessed_source
	call	ta_restore_preprocessed_source
	jmp	ta_assembler_error
