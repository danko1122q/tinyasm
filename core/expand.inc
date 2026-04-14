ta_preprocessor:
	mov	edi,ta_characters
	xor	al,al
      ta_make_characters_table:
	stosb
	inc	al
	jnz	ta_make_characters_table
	mov	esi,ta_characters+'a'
	mov	edi,ta_characters+'A'
	mov	ecx,26
	rep	movsb
	mov	edi,ta_characters
	mov	esi,symbol_characters+1
	movzx	ecx,byte [esi-1]
	xor	eax,eax
      ta_mark_symbol_characters:
	lodsb
	mov	byte [edi+eax],0
	loop	ta_mark_symbol_characters
	mov	edi,ta_locals_counter
	mov	ax,1 + '0' shl 8
	stos	word [edi]
	mov	edi,[ta_memory_start]
	mov	[ta_include_paths],edi
	mov	esi,include_variable
	call	ta_get_environment_variable
	; if env var was found, append ';' separator before -i paths
	cmp	edi,[ta_include_paths]
	je	ta_no_env_include
	mov	byte [edi],';'
	inc	edi
      ta_no_env_include:
	; append paths collected from -i flags (already ';'-separated, ends with null)
	mov	esi,ta_include_extra
      ta_append_include_extra:
	lodsb
	stos	byte [edi]
	or	al,al
	jnz	ta_append_include_extra
	mov	[ta_memory_start],edi
	mov	eax,[ta_additional_memory]
	mov	[ta_free_additional_memory],eax
	mov	eax,[ta_additional_memory_end]
	mov	[ta_labels_list],eax
	xor	eax,eax
	mov	[ta_source_start],eax
	mov	[ta_tagged_blocks],eax
	mov	[ta_hash_tree],eax
	mov	[ta_error],eax
	mov	[ta_macro_status],al
	mov	[ta_current_line],eax
	mov	esi,[ta_initial_definitions]
	test	esi,esi
	jz	ta_predefinitions_ok
      ta_process_predefinitions:
	movzx	ecx,byte [esi]
	test	ecx,ecx
	jz	ta_predefinitions_ok
	inc	esi
	lea	eax,[esi+ecx]
	push	eax
	mov	ch,10b
	call	ta_add_preprocessor_symbol
	pop	esi
	mov	edi,[ta_memory_start]
	mov	[edx+8],edi
      ta_convert_predefinition:
	cmp	edi,[ta_memory_end]
	jae	ta_out_of_memory
	lods	byte [esi]
	or	al,al
	jz	ta_predefinition_converted
	cmp	al,20h
	je	ta_convert_predefinition
	mov	ah,al
	mov	ebx,ta_characters
	xlat	byte [ebx]
	or	al,al
	jz	ta_predefinition_separator
	cmp	ah,27h
	je	ta_predefinition_string
	cmp	ah,22h
	je	ta_predefinition_string
	mov	byte [edi],1Ah
	scas	word [edi]
	xchg	al,ah
	stos	byte [edi]
	mov	ebx,ta_characters
	xor	ecx,ecx
      ta_predefinition_symbol:
	lods	byte [esi]
	stos	byte [edi]
	xlat	byte [ebx]
	or	al,al
	loopnzd ta_predefinition_symbol
	neg	ecx
	cmp	ecx,255
	ja	ta_invalid_definition
	mov	ebx,edi
	sub	ebx,ecx
	mov	byte [ebx-2],cl
      ta_found_predefinition_separator:
	dec	edi
	mov	ah,[esi-1]
      ta_predefinition_separator:
	xchg	al,ah
	or	al,al
	jz	ta_predefinition_converted
	cmp	al,20h
	je	ta_convert_predefinition
	cmp	al,3Bh
	je	ta_invalid_definition
	cmp	al,5Ch
	je	ta_predefinition_backslash
	stos	byte [edi]
	jmp	ta_convert_predefinition
      ta_predefinition_string:
	mov	al,22h
	stos	byte [edi]
	scas	dword [edi]
	mov	ebx,edi
      ta_copy_predefinition_string:
	lods	byte [esi]
	stos	byte [edi]
	or	al,al
	jz	ta_invalid_definition
	cmp	al,ah
	jne	ta_copy_predefinition_string
	lods	byte [esi]
	cmp	al,ah
	je	ta_copy_predefinition_string
	dec	esi
	dec	edi
	mov	eax,edi
	sub	eax,ebx
	mov	[ebx-4],eax
	jmp	ta_convert_predefinition
      ta_predefinition_backslash:
	mov	byte [edi],0
	lods	byte [esi]
	or	al,al
	jz	ta_invalid_definition
	cmp	al,20h
	je	ta_invalid_definition
	cmp	al,3Bh
	je	ta_invalid_definition
	mov	al,1Ah
	stos	byte [edi]
	mov	ecx,edi
	mov	ax,5C01h
	stos	word [edi]
	dec	esi
      ta_group_predefinition_backslashes:
	lods	byte [esi]
	cmp	al,5Ch
	jne	ta_predefinition_backslashed_symbol
	stos	byte [edi]
	inc	byte [ecx]
	jmp	ta_group_predefinition_backslashes
      ta_predefinition_backslashed_symbol:
	cmp	al,20h
	je	ta_invalid_definition
	cmp	al,22h
	je	ta_invalid_definition
	cmp	al,27h
	je	ta_invalid_definition
	cmp	al,3Bh
	je	ta_invalid_definition
	mov	ah,al
	mov	ebx,ta_characters
	xlat	byte [ebx]
	or	al,al
	jz	ta_predefinition_backslashed_symbol_character
	mov	al,ah
      ta_convert_predefinition_backslashed_symbol:
	stos	byte [edi]
	xlat	byte [ebx]
	or	al,al
	jz	ta_found_predefinition_separator
	inc	byte [ecx]
	jz	ta_invalid_definition
	lods	byte [esi]
	jmp	ta_convert_predefinition_backslashed_symbol
      ta_predefinition_backslashed_symbol_character:
	mov	al,ah
	stos	byte [edi]
	inc	byte [ecx]
	jmp	ta_convert_predefinition
      ta_predefinition_converted:
	mov	[ta_memory_start],edi
	sub	edi,[edx+8]
	mov	[edx+12],edi
	jmp	ta_process_predefinitions
      ta_predefinitions_ok:
	mov	esi,[ta_input_file]
	mov	edx,esi
	call	ta_open
	jc	ta_main_file_not_found
	mov	edi,[ta_memory_start]
	call	ta_preprocess_file
	cmp	[ta_macro_status],0
	je	ta_process_postponed
	mov	eax,[ta_error_line]
	mov	[ta_current_line],eax
	jmp	ta_incomplete_macro
      ta_process_postponed:
	mov	edx,ta_hash_tree
	mov	ecx,32
      ta_find_postponed_list:
	mov	edx,[edx]
	or	edx,edx
	loopnz	ta_find_postponed_list
	jz	ta_preprocessing_finished
      ta_process_postponed_list:
	mov	eax,[edx]
	or	eax,eax
	jz	ta_preprocessing_finished
	push	edx
	mov	ebx,edx
      ta_find_earliest_postponed:
	mov	eax,[edx]
	or	eax,eax
	jz	ta_earliest_postponed_found
	mov	ebx,edx
	mov	edx,eax
	jmp	ta_find_earliest_postponed
      ta_earliest_postponed_found:
	mov	[ebx],eax
	call	ta_use_postponed_macro
	pop	edx
	cmp	[ta_macro_status],0
	je	ta_process_postponed_list
	mov	eax,[ta_error_line]
	mov	[ta_current_line],eax
	jmp	ta_incomplete_macro
      ta_preprocessing_finished:
	mov	[ta_source_start],edi
	ret
      ta_use_postponed_macro:
	lea	esi,[edi-1]
	push	ecx esi
	mov	[ta_struc_name],0
	jmp	ta_use_macro

ta_preprocess_file:
	push	[ta_memory_end]
	push	esi
	mov	al,2
	xor	edx,edx
	call	ta_lseek
	push	eax
	xor	al,al
	xor	edx,edx
	call	ta_lseek
	pop	ecx
	mov	edx,[ta_memory_end]
	dec	edx
	mov	byte [edx],1Ah
	sub	edx,ecx
	jc	ta_out_of_memory
	mov	esi,edx
	cmp	edx,edi
	jbe	ta_out_of_memory
	mov	[ta_memory_end],edx
	call	ta_read
	call	ta_close
	pop	edx
	xor	ecx,ecx
	mov	ebx,esi
      ta_preprocess_source:
	inc	ecx
	mov	[ta_current_line],edi
	mov	eax,edx
	stos	dword [edi]
	mov	eax,ecx
	stos	dword [edi]
	mov	eax,esi
	sub	eax,ebx
	stos	dword [edi]
	xor	eax,eax
	stos	dword [edi]
	push	ebx edx
	call	ta_convert_line
	call	ta_preprocess_line
	pop	edx ebx
      ta_next_line:
	cmp	byte [esi-1],0
	je	ta_file_end
	cmp	byte [esi-1],1Ah
	jne	ta_preprocess_source
      ta_file_end:
	pop	[ta_memory_end]
	clc
	ret

ta_convert_line:
	push	ecx
	test	[ta_macro_status],0Fh
	jz	ta_convert_line_data
	mov	ax,3Bh
	stos	word [edi]
      ta_convert_line_data:
	cmp	edi,[ta_memory_end]
	jae	ta_out_of_memory
	lods	byte [esi]
	cmp	al,20h
	je	ta_convert_line_data
	cmp	al,9
	je	ta_convert_line_data
	mov	ah,al
	mov	ebx,ta_characters
	xlat	byte [ebx]
	or	al,al
	jz	ta_convert_separator
	cmp	ah,27h
	je	ta_convert_string
	cmp	ah,22h
	je	ta_convert_string
	mov	byte [edi],1Ah
	scas	word [edi]
	xchg	al,ah
	stos	byte [edi]
	mov	ebx,ta_characters
	xor	ecx,ecx
      ta_convert_symbol:
	lods	byte [esi]
	stos	byte [edi]
	xlat	byte [ebx]
	or	al,al
	loopnzd ta_convert_symbol
	neg	ecx
	cmp	ecx,255
	ja	ta_name_too_long
	mov	ebx,edi
	sub	ebx,ecx
	mov	byte [ebx-2],cl
      ta_found_separator:
	dec	edi
	mov	ah,[esi-1]
      ta_convert_separator:
	xchg	al,ah
	cmp	al,20h
	jb	ta_control_character
	je	ta_convert_line_data
      ta_symbol_character:
	cmp	al,3Bh
	je	ta_ignore_comment
	cmp	al,5Ch
	je	ta_backslash_character
	stos	byte [edi]
	jmp	ta_convert_line_data
      ta_control_character:
	cmp	al,1Ah
	je	ta_line_end
	cmp	al,0Dh
	je	ta_cr_character
	cmp	al,0Ah
	je	ta_lf_character
	cmp	al,9
	je	ta_convert_line_data
	or	al,al
	jnz	ta_symbol_character
	jmp	ta_line_end
      ta_lf_character:
	lods	byte [esi]
	cmp	al,0Dh
	je	ta_line_end
	dec	esi
	jmp	ta_line_end
      ta_cr_character:
	lods	byte [esi]
	cmp	al,0Ah
	je	ta_line_end
	dec	esi
	jmp	ta_line_end
      ta_convert_string:
	mov	al,22h
	stos	byte [edi]
	scas	dword [edi]
	mov	ebx,edi
      ta_copy_string:
	lods	byte [esi]
	stos	byte [edi]
	cmp	al,0Ah
	je	ta_no_end_quote
	cmp	al,0Dh
	je	ta_no_end_quote
	or	al,al
	jz	ta_no_end_quote
	cmp	al,1Ah
	je	ta_no_end_quote
	cmp	al,ah
	jne	ta_copy_string
	lods	byte [esi]
	cmp	al,ah
	je	ta_copy_string
	dec	esi
	dec	edi
	mov	eax,edi
	sub	eax,ebx
	mov	[ebx-4],eax
	jmp	ta_convert_line_data
      ta_backslash_character:
	mov	byte [edi],0
	lods	byte [esi]
	cmp	al,20h
	je	ta_concatenate_lines
	cmp	al,9
	je	ta_concatenate_lines
	cmp	al,1Ah
	je	ta_line_end
	or	al,al
	jz	ta_line_end
	cmp	al,0Ah
	je	ta_concatenate_lf
	cmp	al,0Dh
	je	ta_concatenate_cr
	cmp	al,3Bh
	je	ta_find_concatenated_line
	mov	al,1Ah
	stos	byte [edi]
	mov	ecx,edi
	mov	ax,5C01h
	stos	word [edi]
	dec	esi
      ta_group_backslashes:
	lods	byte [esi]
	cmp	al,5Ch
	jne	ta_backslashed_symbol
	stos	byte [edi]
	inc	byte [ecx]
	jz	ta_name_too_long
	jmp	ta_group_backslashes
      ta_no_end_quote:
	mov	byte [ebx-5],0
	jmp	ta_missing_end_quote
      ta_backslashed_symbol:
	cmp	al,1Ah
	je	ta_extra_characters_on_line
	or	al,al
	jz	ta_extra_characters_on_line
	cmp	al,0Ah
	je	ta_extra_characters_on_line
	cmp	al,0Dh
	je	ta_extra_characters_on_line
	cmp	al,20h
	je	ta_extra_characters_on_line
	cmp	al,9
	je	ta_extra_characters_on_line
	cmp	al,22h
	je	ta_extra_characters_on_line
	cmp	al,27h
	je	ta_extra_characters_on_line
	cmp	al,3Bh
	je	ta_extra_characters_on_line
	mov	ah,al
	mov	ebx,ta_characters
	xlat	byte [ebx]
	or	al,al
	jz	ta_backslashed_symbol_character
	mov	al,ah
      ta_convert_backslashed_symbol:
	stos	byte [edi]
	xlat	byte [ebx]
	or	al,al
	jz	ta_found_separator
	inc	byte [ecx]
	jz	ta_name_too_long
	lods	byte [esi]
	jmp	ta_convert_backslashed_symbol
      ta_backslashed_symbol_character:
	mov	al,ah
	stos	byte [edi]
	inc	byte [ecx]
	jmp	ta_convert_line_data
      ta_concatenate_lines:
	lods	byte [esi]
	cmp	al,20h
	je	ta_concatenate_lines
	cmp	al,9
	je	ta_concatenate_lines
	cmp	al,1Ah
	je	ta_line_end
	or	al,al
	jz	ta_line_end
	cmp	al,0Ah
	je	ta_concatenate_lf
	cmp	al,0Dh
	je	ta_concatenate_cr
	cmp	al,3Bh
	jne	ta_extra_characters_on_line
      ta_find_concatenated_line:
	lods	byte [esi]
	cmp	al,0Ah
	je	ta_concatenate_lf
	cmp	al,0Dh
	je	ta_concatenate_cr
	or	al,al
	jz	ta_concatenate_ok
	cmp	al,1Ah
	jne	ta_find_concatenated_line
	jmp	ta_line_end
      ta_concatenate_lf:
	lods	byte [esi]
	cmp	al,0Dh
	je	ta_concatenate_ok
	dec	esi
	jmp	ta_concatenate_ok
      ta_concatenate_cr:
	lods	byte [esi]
	cmp	al,0Ah
	je	ta_concatenate_ok
	dec	esi
      ta_concatenate_ok:
	inc	dword [esp]
	jmp	ta_convert_line_data
      ta_ignore_comment:
	lods	byte [esi]
	cmp	al,0Ah
	je	ta_lf_character
	cmp	al,0Dh
	je	ta_cr_character
	or	al,al
	jz	ta_line_end
	cmp	al,1Ah
	jne	ta_ignore_comment
      ta_line_end:
	xor	al,al
	stos	byte [edi]
	pop	ecx
	ret

ta_lower_case:
	mov	edi,ta_converted
	mov	ebx,ta_characters
      ta_convert_case:
	lods	byte [esi]
	xlat	byte [ebx]
	stos	byte [edi]
	loop	ta_convert_case
      ta_case_ok:
	ret

ta_get_directive:
	push	edi
	mov	edx,esi
	mov	ebp,ecx
	call	ta_lower_case
	pop	edi
      ta_scan_directives:
	mov	esi,ta_converted
	movzx	eax,byte [edi]
	or	al,al
	jz	ta_no_directive
	mov	ecx,ebp
	inc	edi
	mov	ebx,edi
	add	ebx,eax
	mov	ah,[esi]
	cmp	ah,[edi]
	jb	ta_no_directive
	ja	ta_next_directive
	cmp	cl,al
	jne	ta_next_directive
	repe	cmps byte [esi],[edi]
	jb	ta_no_directive
	je	ta_directive_found
      ta_next_directive:
	mov	edi,ebx
	add	edi,2
	jmp	ta_scan_directives
      ta_no_directive:
	mov	esi,edx
	mov	ecx,ebp
	stc
	ret
      ta_directive_found:
	call	ta_get_directive_handler_base
      ta_directive_handler:
	lea	esi,[edx+ebp]
	movzx	ecx,word [ebx]
	add	eax,ecx
	clc
	ret
      ta_get_directive_handler_base:
	mov	eax,[esp]
	ret

ta_preprocess_line:
	mov	eax,esp
	sub	eax,[ta_stack_limit]
	cmp	eax,100h
	jb	ta_stack_overflow
	push	ecx esi
      ta_preprocess_current_line:
	mov	esi,[ta_current_line]
	add	esi,16
	cmp	word [esi],3Bh
	jne	ta_line_start_ok
	add	esi,2
      ta_line_start_ok:
	test	[ta_macro_status],0F0h
	jnz	ta_macro_preprocessing
	cmp	byte [esi],1Ah
	jne	ta_not_fix_constant
	movzx	edx,byte [esi+1]
	lea	edx,[esi+2+edx]
	cmp	word [edx],031Ah
	jne	ta_not_fix_constant
	mov	ebx,ta_characters
	movzx	eax,byte [edx+2]
	xlat	byte [ebx]
	ror	eax,8
	mov	al,[edx+3]
	xlat	byte [ebx]
	ror	eax,8
	mov	al,[edx+4]
	xlat	byte [ebx]
	ror	eax,16
	cmp	eax,'fix'
	je	ta_define_fix_constant
      ta_not_fix_constant:
	call	ta_process_fix_constants
	jmp	ta_initial_preprocessing_ok
      ta_macro_preprocessing:
	call	ta_process_macro_operators
      ta_initial_preprocessing_ok:
	mov	esi,[ta_current_line]
	add	esi,16
	mov	al,[ta_macro_status]
	test	al,2
	jnz	ta_skip_macro_block
	test	al,1
	jnz	ta_find_macro_block
      ta_preprocess_instruction:
	mov	[ta_current_offset],esi
	lods	byte [esi]
	movzx	ecx,byte [esi]
	inc	esi
	cmp	al,1Ah
	jne	ta_not_preprocessor_symbol
	cmp	cl,3
	jb	ta_not_preprocessor_directive
	push	edi
	mov	edi,ta_preprocessor_directives
	call	ta_get_directive
	pop	edi
	jc	ta_not_preprocessor_directive
	mov	byte [edx-2],3Bh
	jmp	near eax
      ta_not_preprocessor_directive:
	xor	ch,ch
	call	ta_get_preprocessor_symbol
	jc	ta_not_macro
	mov	byte [ebx-2],3Bh
	mov	[ta_struc_name],0
	jmp	ta_use_macro
      ta_not_macro:
	mov	[ta_struc_name],esi
	add	esi,ecx
	lods	byte [esi]
	cmp	al,':'
	je	ta_preprocess_label
	cmp	al,1Ah
	jne	ta_not_preprocessor_symbol
	lods	byte [esi]
	cmp	al,3
	jne	ta_not_symbolic_constant
	mov	ebx,ta_characters
	movzx	eax,byte [esi]
	xlat	byte [ebx]
	ror	eax,8
	mov	al,[esi+1]
	xlat	byte [ebx]
	ror	eax,8
	mov	al,[esi+2]
	xlat	byte [ebx]
	ror	eax,16
	cmp	eax,'equ'
	je	ta_define_equ_constant
	mov	al,3
      ta_not_symbolic_constant:
	mov	ch,1
	mov	cl,al
	call	ta_get_preprocessor_symbol
	jc	ta_not_preprocessor_symbol
	push	edx esi
	mov	esi,[ta_struc_name]
	mov	[ta_struc_label],esi
	sub	[ta_struc_label],2
	mov	cl,[esi-1]
	mov	ch,10b
	call	ta_get_preprocessor_symbol
	jc	ta_struc_name_ok
	test	edx,edx
	jz	ta_reserved_word_used_as_symbol
	mov	ecx,[edx+12]
	add	ecx,3
	lea	ebx,[edi+ecx]
	mov	ecx,edi
	sub	ecx,[ta_struc_label]
	lea	esi,[edi-1]
	lea	edi,[ebx-1]
	std
	rep	movs byte [edi],[esi]
	cld
	mov	edi,[ta_struc_label]
	mov	esi,[edx+8]
	mov	ecx,[edx+12]
	add	[ta_struc_name],ecx
	add	[ta_struc_name],3
	call	ta_move_data
	mov	al,3Ah
	stos	byte [edi]
	mov	ax,3Bh
	stos	word [edi]
	mov	edi,ebx
	pop	esi
	add	esi,[edx+12]
	add	esi,3
	pop	edx
	jmp	ta_use_macro
      ta_struc_name_ok:
	mov	edx,[ta_struc_name]
	movzx	eax,byte [edx-1]
	add	edx,eax
	push	edi
	lea	esi,[edi-1]
	mov	ecx,edi
	sub	ecx,edx
	std
	rep	movs byte [edi],[esi]
	cld
	pop	edi
	inc	edi
	mov	al,3Ah
	mov	[edx],al
	inc	al
	mov	[edx+1],al
	pop	esi edx
	inc	esi
	jmp	ta_use_macro
      ta_preprocess_label:
	dec	esi
	sub	esi,ecx
	lea	ebp,[esi-2]
	mov	ch,10b
	call	ta_get_preprocessor_symbol
	jnc	ta_symbolic_constant_in_label
	lea	esi,[esi+ecx+1]
	cmp	byte [esi],':'
	jne	ta_preprocess_instruction
	inc	esi
	jmp	ta_preprocess_instruction
      ta_symbolic_constant_in_label:
	test	edx,edx
	jz	ta_reserved_word_used_as_symbol
	mov	ebx,[edx+8]
	mov	ecx,[edx+12]
	add	ecx,ebx
      ta_check_for_broken_label:
	cmp	ebx,ecx
	je	ta_label_broken
	cmp	byte [ebx],1Ah
	jne	ta_label_broken
	movzx	eax,byte [ebx+1]
	lea	ebx,[ebx+2+eax]
	cmp	ebx,ecx
	je	ta_label_constant_ok
	cmp	byte [ebx],':'
	jne	ta_label_broken
	inc	ebx
	cmp	byte [ebx],':'
	jne	ta_check_for_broken_label
	inc	ebx
	jmp	ta_check_for_broken_label
      ta_label_broken:
	call	ta_replace_symbolic_constant
	jmp	ta_line_preprocessed
      ta_label_constant_ok:
	mov	ecx,edi
	sub	ecx,esi
	mov	edi,[edx+12]
	add	edi,ebp
	push	edi
	lea	eax,[edi+ecx]
	push	eax
	cmp	esi,edi
	je	ta_replace_label
	jb	ta_move_rest_of_line_up
	rep	movs byte [edi],[esi]
	jmp	ta_replace_label
      ta_move_rest_of_line_up:
	lea	esi,[esi+ecx-1]
	lea	edi,[edi+ecx-1]
	std
	rep	movs byte [edi],[esi]
	cld
      ta_replace_label:
	mov	ecx,[edx+12]
	mov	edi,[esp+4]
	sub	edi,ecx
	mov	esi,[edx+8]
	rep	movs byte [edi],[esi]
	pop	edi esi
	inc	esi
	jmp	ta_preprocess_instruction
      ta_not_preprocessor_symbol:
	mov	esi,[ta_current_offset]
	call	ta_process_equ_constants
      ta_line_preprocessed:
	pop	esi ecx
	ret

ta_get_preprocessor_symbol:
	push	ebp edi esi
	mov	ebp,ecx
	shl	ebp,22
	mov	al,ch
	and	al,11b
	movzx	ecx,cl
	cmp	al,10b
	jne	ta_no_preprocessor_special_symbol
	cmp	cl,4
	jbe	ta_no_preprocessor_special_symbol
	mov	ax,'__'
	cmp	ax,[esi]
	jne	ta_no_preprocessor_special_symbol
	cmp	ax,[esi+ecx-2]
	jne	ta_no_preprocessor_special_symbol
	add	esi,2
	sub	ecx,4
	push	ebp
	mov	edi,ta_preprocessor_special_symbols
	call	ta_get_directive
	pop	ebp
	jc	ta_preprocessor_special_symbol_not_recognized
	add	esi,2
	xor	edx,edx
	jmp	ta_preprocessor_symbol_found
      ta_preprocessor_special_symbol_not_recognized:
	add	ecx,4
	sub	esi,2
      ta_no_preprocessor_special_symbol:
	mov	ebx,ta_hash_tree
	mov	edi,10
      ta_follow_hashes_roots:
	mov	edx,[ebx]
	or	edx,edx
	jz	ta_preprocessor_symbol_not_found
	xor	eax,eax
	shl	ebp,1
	adc	eax,0
	lea	ebx,[edx+eax*4]
	dec	edi
	jnz	ta_follow_hashes_roots
	mov	edi,ebx
	call	ta_calculate_hash
	mov	ebp,eax
	and	ebp,3FFh
	shl	ebp,10
	xor	ebp,eax
	mov	ebx,edi
	mov	edi,22
      ta_follow_hashes_tree:
	mov	edx,[ebx]
	or	edx,edx
	jz	ta_preprocessor_symbol_not_found
	xor	eax,eax
	shl	ebp,1
	adc	eax,0
	lea	ebx,[edx+eax*4]
	dec	edi
	jnz	ta_follow_hashes_tree
	mov	al,cl
	mov	edx,[ebx]
	or	edx,edx
	jz	ta_preprocessor_symbol_not_found
      ta_compare_with_preprocessor_symbol:
	mov	edi,[edx+4]
	cmp	edi,1
	jbe	ta_next_equal_hash
	repe	cmps byte [esi],[edi]
	je	ta_preprocessor_symbol_found
	mov	cl,al
	mov	esi,[esp]
      ta_next_equal_hash:
	mov	edx,[edx]
	or	edx,edx
	jnz	ta_compare_with_preprocessor_symbol
      ta_preprocessor_symbol_not_found:
	pop	esi edi ebp
	stc
	ret
      ta_preprocessor_symbol_found:
	pop	ebx edi ebp
	clc
	ret
      ta_calculate_hash:
	xor	ebx,ebx
	mov	eax,2166136261
	mov	ebp,16777619
      ta_fnv1a_hash:
	xor	al,[esi+ebx]
	mul	ebp
	inc	bl
	cmp	bl,cl
	jb	ta_fnv1a_hash
	ret
ta_add_preprocessor_symbol:
	push	edi esi
	xor	eax,eax
	or	cl,cl
	jz	ta_reshape_hash
	cmp	ch,11b
	je	ta_preprocessor_symbol_name_ok
	push	ecx
	movzx	ecx,cl
	mov	edi,ta_preprocessor_directives
	call	ta_get_directive
	jnc	ta_reserved_word_used_as_symbol
	pop	ecx
      ta_preprocessor_symbol_name_ok:
	call	ta_calculate_hash
      ta_reshape_hash:
	mov	ebp,eax
	and	ebp,3FFh
	shr	eax,10
	xor	ebp,eax
	shl	ecx,22
	or	ebp,ecx
	mov	ebx,ta_hash_tree
	mov	ecx,32
      ta_find_leave_for_symbol:
	mov	edx,[ebx]
	or	edx,edx
	jz	ta_extend_hashes_tree
	xor	eax,eax
	rol	ebp,1
	adc	eax,0
	lea	ebx,[edx+eax*4]
	dec	ecx
	jnz	ta_find_leave_for_symbol
	mov	edx,[ebx]
	or	edx,edx
	jz	ta_add_symbol_entry
	shr	ebp,30
	cmp	ebp,11b
	je	ta_reuse_symbol_entry
	cmp	dword [edx+4],0
	jne	ta_add_symbol_entry
      ta_find_entry_to_reuse:
	mov	edi,[edx]
	or	edi,edi
	jz	ta_reuse_symbol_entry
	cmp	dword [edi+4],0
	jne	ta_reuse_symbol_entry
	mov	edx,edi
	jmp	ta_find_entry_to_reuse
      ta_add_symbol_entry:
	mov	eax,edx
	mov	edx,[ta_labels_list]
	sub	edx,16
	cmp	edx,[ta_free_additional_memory]
	jb	ta_out_of_memory
	mov	[ta_labels_list],edx
	mov	[edx],eax
	mov	[ebx],edx
      ta_reuse_symbol_entry:
	pop	esi edi
	mov	[edx+4],esi
	ret
      ta_extend_hashes_tree:
	mov	edx,[ta_labels_list]
	sub	edx,8
	cmp	edx,[ta_free_additional_memory]
	jb	ta_out_of_memory
	mov	[ta_labels_list],edx
	xor	eax,eax
	mov	[edx],eax
	mov	[edx+4],eax
	shl	ebp,1
	adc	eax,0
	mov	[ebx],edx
	lea	ebx,[edx+eax*4]
	dec	ecx
	jnz	ta_extend_hashes_tree
	mov	edx,[ta_labels_list]
	sub	edx,16
	cmp	edx,[ta_free_additional_memory]
	jb	ta_out_of_memory
	mov	[ta_labels_list],edx
	mov	dword [edx],0
	mov	[ebx],edx
	pop	esi edi
	mov	[edx+4],esi
	ret

ta_define_fix_constant:
	add	edx,5
	add	esi,2
	push	edx
	mov	ch,11b
	jmp	ta_define_preprocessor_constant
ta_define_equ_constant:
	add	esi,3
	push	esi
	call	ta_process_equ_constants
	mov	esi,[ta_struc_name]
	mov	ch,10b
      ta_define_preprocessor_constant:
	mov	byte [esi-2],3Bh
	mov	cl,[esi-1]
	call	ta_add_preprocessor_symbol
	pop	ebx
	mov	ecx,edi
	dec	ecx
	sub	ecx,ebx
	mov	[edx+8],ebx
	mov	[edx+12],ecx
	jmp	ta_line_preprocessed
ta_define_symbolic_constant:
	lods	byte [esi]
	cmp	al,1Ah
	jne	ta_invalid_name
	lods	byte [esi]
	mov	cl,al
	mov	ch,10b
	call	ta_add_preprocessor_symbol
	movzx	eax,byte [esi-1]
	add	esi,eax
	lea	ecx,[edi-1]
	sub	ecx,esi
	mov	[edx+8],esi
	mov	[edx+12],ecx
	jmp	ta_line_preprocessed

ta_define_struc:
	mov	ch,1
	jmp	ta_make_macro
ta_define_macro:
	xor	ch,ch
      ta_make_macro:
	lods	byte [esi]
	cmp	al,1Ah
	jne	ta_invalid_name
	lods	byte [esi]
	mov	cl,al
	call	ta_add_preprocessor_symbol
	mov	eax,[ta_current_line]
	mov	[edx+12],eax
	movzx	eax,byte [esi-1]
	add	esi,eax
	mov	[edx+8],esi
	mov	al,[ta_macro_status]
	and	al,0F0h
	or	al,1
	mov	[ta_macro_status],al
	mov	eax,[ta_current_line]
	mov	[ta_error_line],eax
	xor	ebp,ebp
	lods	byte [esi]
	or	al,al
	jz	ta_line_preprocessed
	cmp	al,'{'
	je	ta_found_macro_block
	dec	esi
      ta_skip_macro_arguments:
	lods	byte [esi]
	cmp	al,1Ah
	je	ta_skip_macro_argument
	cmp	al,'['
	jne	ta_invalid_macro_arguments
	or	ebp,-1
	jz	ta_invalid_macro_arguments
	lods	byte [esi]
	cmp	al,1Ah
	jne	ta_invalid_macro_arguments
      ta_skip_macro_argument:
	movzx	eax,byte [esi]
	inc	esi
	add	esi,eax
	lods	byte [esi]
	cmp	al,':'
	je	ta_macro_argument_with_default_value
	cmp	al,'='
	je	ta_macro_argument_with_default_value
	cmp	al,'*'
	jne	ta_macro_argument_end
	lods	byte [esi]
      ta_macro_argument_end:
	cmp	al,','
	je	ta_skip_macro_arguments
	cmp	al,'&'
	je	ta_macro_arguments_finisher
	cmp	al,']'
	jne	ta_end_macro_arguments
	not	ebp
      ta_macro_arguments_finisher:
	lods	byte [esi]
      ta_end_macro_arguments:
	or	ebp,ebp
	jnz	ta_invalid_macro_arguments
	or	al,al
	jz	ta_line_preprocessed
	cmp	al,'{'
	je	ta_found_macro_block
	jmp	ta_invalid_macro_arguments
      ta_macro_argument_with_default_value:
	or	[ta_skip_default_argument_value],-1
	call	ta_skip_macro_argument_value
	inc	esi
	jmp	ta_macro_argument_end
      ta_skip_macro_argument_value:
	cmp	byte [esi],'<'
	jne	ta_simple_argument
	mov	ecx,1
	inc	esi
      ta_enclosed_argument:
	lods	byte [esi]
	or	al,al
	jz	ta_invalid_macro_arguments
	cmp	al,1Ah
	je	ta_enclosed_symbol
	cmp	al,22h
	je	ta_enclosed_string
	cmp	al,'>'
	je	ta_enclosed_argument_end
	cmp	al,'<'
	jne	ta_enclosed_argument
	inc	ecx
	jmp	ta_enclosed_argument
      ta_enclosed_symbol:
	movzx	eax,byte [esi]
	inc	esi
	add	esi,eax
	jmp	ta_enclosed_argument
      ta_enclosed_string:
	lods	dword [esi]
	add	esi,eax
	jmp	ta_enclosed_argument
      ta_enclosed_argument_end:
	loop	ta_enclosed_argument
	lods	byte [esi]
	or	al,al
	jz	ta_argument_value_end
	cmp	al,','
	je	ta_argument_value_end
	cmp	[ta_skip_default_argument_value],0
	je	ta_invalid_macro_arguments
	cmp	al,'{'
	je	ta_argument_value_end
	cmp	al,'&'
	je	ta_argument_value_end
	or	ebp,ebp
	jz	ta_invalid_macro_arguments
	cmp	al,']'
	je	ta_argument_value_end
	jmp	ta_invalid_macro_arguments
      ta_simple_argument:
	lods	byte [esi]
	or	al,al
	jz	ta_argument_value_end
	cmp	al,','
	je	ta_argument_value_end
	cmp	al,22h
	je	ta_argument_string
	cmp	al,1Ah
	je	ta_argument_symbol
	cmp	[ta_skip_default_argument_value],0
	je	ta_simple_argument
	cmp	al,'{'
	je	ta_argument_value_end
	cmp	al,'&'
	je	ta_argument_value_end
	or	ebp,ebp
	jz	ta_simple_argument
	cmp	al,']'
	je	ta_argument_value_end
      ta_argument_symbol:
	movzx	eax,byte [esi]
	inc	esi
	add	esi,eax
	jmp	ta_simple_argument
      ta_argument_string:
	lods	dword [esi]
	add	esi,eax
	jmp	ta_simple_argument
      ta_argument_value_end:
	dec	esi
	ret
      ta_find_macro_block:
	add	esi,2
	lods	byte [esi]
	or	al,al
	jz	ta_line_preprocessed
	cmp	al,'{'
	jne	ta_unexpected_characters
      ta_found_macro_block:
	or	[ta_macro_status],2
      ta_skip_macro_block:
	lods	byte [esi]
	cmp	al,1Ah
	je	ta_skip_macro_symbol
	cmp	al,3Bh
	je	ta_skip_macro_symbol
	cmp	al,22h
	je	ta_skip_macro_string
	or	al,al
	jz	ta_line_preprocessed
	cmp	al,'}'
	jne	ta_skip_macro_block
	mov	al,[ta_macro_status]
	and	[ta_macro_status],0F0h
	test	al,8
	jnz	ta_use_instant_macro
	cmp	byte [esi],0
	je	ta_line_preprocessed
	mov	ecx,edi
	sub	ecx,esi
	mov	edx,esi
	lea	esi,[esi+ecx-1]
	lea	edi,[edi+1+16]
	mov	ebx,edi
	dec	edi
	std
	rep	movs byte [edi],[esi]
	cld
	mov	edi,edx
	xor	al,al
	stos	byte [edi]
	mov	esi,[ta_current_line]
	mov	[ta_current_line],edi
	mov	ecx,4
	rep	movs dword [edi],[esi]
	mov	edi,ebx
	jmp	ta_initial_preprocessing_ok
      ta_skip_macro_symbol:
	movzx	eax,byte [esi]
	inc	esi
	add	esi,eax
	jmp	ta_skip_macro_block
      ta_skip_macro_string:
	lods	dword [esi]
	add	esi,eax
	jmp	ta_skip_macro_block
ta_postpone_directive:
	push	esi
	mov	esi,edx
	xor	ecx,ecx
	call	ta_add_preprocessor_symbol
	mov	eax,[ta_current_line]
	mov	[ta_error_line],eax
	mov	[edx+12],eax
	pop	esi
	mov	[edx+8],esi
	mov	al,[ta_macro_status]
	and	al,0F0h
	or	al,1
	mov	[ta_macro_status],al
	lods	byte [esi]
	or	al,al
	jz	ta_line_preprocessed
	cmp	al,'{'
	jne	ta_unexpected_characters
	jmp	ta_found_macro_block
ta_rept_directive:
	mov	[ta_base_code],0
	jmp	ta_define_instant_macro
ta_irp_directive:
	mov	[ta_base_code],1
	jmp	ta_define_instant_macro
ta_irps_directive:
	mov	[ta_base_code],2
	jmp	ta_define_instant_macro
ta_irpv_directive:
	mov	[ta_base_code],3
	jmp	ta_define_instant_macro
ta_match_directive:
	mov	[ta_base_code],10h
ta_define_instant_macro:
	mov	al,[ta_macro_status]
	and	al,0F0h
	or	al,8+1
	mov	[ta_macro_status],al
	mov	eax,[ta_current_line]
	mov	[ta_error_line],eax
	mov	[ta_instant_macro_start],esi
	cmp	[ta_base_code],10h
	je	ta_prepare_match
      ta_skip_parameters:
	lods	byte [esi]
	or	al,al
	jz	ta_parameters_skipped
	cmp	al,'{'
	je	ta_parameters_skipped
	cmp	al,22h
	je	ta_skip_quoted_parameter
	cmp	al,1Ah
	jne	ta_skip_parameters
	lods	byte [esi]
	movzx	eax,al
	add	esi,eax
	jmp	ta_skip_parameters
      ta_skip_quoted_parameter:
	lods	dword [esi]
	add	esi,eax
	jmp	ta_skip_parameters
      ta_parameters_skipped:
	dec	esi
	mov	[ta_parameters_end],esi
	lods	byte [esi]
	cmp	al,'{'
	je	ta_found_macro_block
	or	al,al
	jnz	ta_invalid_macro_arguments
	jmp	ta_line_preprocessed
ta_prepare_match:
	call	ta_skip_pattern
	mov	[ta_value_type],80h+10b
	call	ta_process_symbolic_constants
	jmp	ta_parameters_skipped
      ta_skip_pattern:
	lods	byte [esi]
	or	al,al
	jz	ta_invalid_macro_arguments
	cmp	al,','
	je	ta_pattern_skipped
	cmp	al,22h
	je	ta_skip_quoted_string_in_pattern
	cmp	al,1Ah
	je	ta_skip_symbol_in_pattern
	cmp	al,'='
	jne	ta_skip_pattern
	mov	al,[esi]
	cmp	al,1Ah
	je	ta_skip_pattern
	cmp	al,22h
	je	ta_skip_pattern
	inc	esi
	jmp	ta_skip_pattern
      ta_skip_symbol_in_pattern:
	lods	byte [esi]
	movzx	eax,al
	add	esi,eax
	jmp	ta_skip_pattern
      ta_skip_quoted_string_in_pattern:
	lods	dword [esi]
	add	esi,eax
	jmp	ta_skip_pattern
      ta_pattern_skipped:
	ret

ta_purge_macro:
	xor	ch,ch
	jmp	ta_restore_preprocessor_symbol
ta_purge_struc:
	mov	ch,1
	jmp	ta_restore_preprocessor_symbol
ta_restore_equ_constant:
	mov	ch,10b
      ta_restore_preprocessor_symbol:
	push	ecx
	lods	byte [esi]
	cmp	al,1Ah
	jne	ta_invalid_name
	lods	byte [esi]
	mov	cl,al
	call	ta_get_preprocessor_symbol
	jc	ta_no_symbol_to_restore
	test	edx,edx
	jz	ta_symbol_restored
	mov	dword [edx+4],0
	jmp	ta_symbol_restored
      ta_no_symbol_to_restore:
	add	esi,ecx
      ta_symbol_restored:
	pop	ecx
	lods	byte [esi]
	cmp	al,','
	je	ta_restore_preprocessor_symbol
	or	al,al
	jnz	ta_extra_characters_on_line
	jmp	ta_line_preprocessed

ta_process_fix_constants:
	mov	[ta_value_type],11b
	jmp	ta_process_symbolic_constants
ta_process_equ_constants:
	mov	[ta_value_type],10b
      ta_process_symbolic_constants:
	mov	ebp,esi
	lods	byte [esi]
	cmp	al,1Ah
	je	ta_check_symbol
	cmp	al,22h
	je	ta_ignore_string
	cmp	al,'{'
	je	ta_check_brace
	or	al,al
	jnz	ta_process_symbolic_constants
	ret
      ta_ignore_string:
	lods	dword [esi]
	add	esi,eax
	jmp	ta_process_symbolic_constants
      ta_check_brace:
	test	[ta_value_type],80h
	jz	ta_process_symbolic_constants
	ret
      ta_no_replacing:
	movzx	ecx,byte [esi-1]
	add	esi,ecx
	jmp	ta_process_symbolic_constants
      ta_check_symbol:
	mov	cl,[esi]
	inc	esi
	mov	ch,[ta_value_type]
	call	ta_get_preprocessor_symbol
	jc	ta_no_replacing
	mov	[ta_current_section],edi
      ta_replace_symbolic_constant:
	test	edx,edx
	jz	ta_replace_special_symbolic_constant
	mov	ecx,[edx+12]
	mov	edx,[edx+8]
	xchg	esi,edx
	call	ta_move_data
	mov	esi,edx
      ta_process_after_replaced:
	lods	byte [esi]
	cmp	al,1Ah
	je	ta_symbol_after_replaced
	stos	byte [edi]
	cmp	al,22h
	je	ta_string_after_replaced
	cmp	al,'{'
	je	ta_brace_after_replaced
	or	al,al
	jnz	ta_process_after_replaced
	mov	ecx,edi
	sub	ecx,esi
	mov	edi,ebp
	call	ta_move_data
	mov	esi,edi
	ret
      ta_move_data:
	lea	eax,[edi+ecx]
	cmp	eax,[ta_memory_end]
	jae	ta_out_of_memory
	shr	ecx,1
	jnc	ta_movsb_ok
	movs	byte [edi],[esi]
      ta_movsb_ok:
	shr	ecx,1
	jnc	ta_movsw_ok
	movs	word [edi],[esi]
      ta_movsw_ok:
	rep	movs dword [edi],[esi]
	ret
      ta_string_after_replaced:
	lods	dword [esi]
	stos	dword [edi]
	mov	ecx,eax
	call	ta_move_data
	jmp	ta_process_after_replaced
      ta_brace_after_replaced:
	test	[ta_value_type],80h
	jz	ta_process_after_replaced
	mov	edx,edi
	mov	ecx,[ta_current_section]
	sub	edx,ecx
	sub	ecx,esi
	rep	movs byte [edi],[esi]
	mov	ecx,edi
	sub	ecx,esi
	mov	edi,ebp
	call	ta_move_data
	lea	esi,[ebp+edx]
	ret
      ta_symbol_after_replaced:
	mov	cl,[esi]
	inc	esi
	mov	ch,[ta_value_type]
	call	ta_get_preprocessor_symbol
	jnc	ta_replace_symbolic_constant
	movzx	ecx,byte [esi-1]
	mov	al,1Ah
	mov	ah,cl
	stos	word [edi]
	call	ta_move_data
	jmp	ta_process_after_replaced
      ta_replace_special_symbolic_constant:
	jmp	near eax
    ta_preprocessed_file_value:
	call	ta_get_current_line_from_file
	test	ebx,ebx
	jz	ta_process_after_replaced
	push	esi edi
	mov	esi,[ebx]
	mov	edi,esi
	xor	al,al
	or	ecx,-1
	repne	scas byte [edi]
	add	ecx,2
	neg	ecx
	pop	edi
	lea	eax,[edi+1+4+ecx]
	cmp	eax,[ta_memory_end]
	ja	ta_out_of_memory
	mov	al,22h
	stos	byte [edi]
	mov	eax,ecx
	stos	dword [edi]
	rep	movs byte [edi],[esi]
	pop	esi
	jmp	ta_process_after_replaced
    ta_preprocessed_line_value:
	call	ta_get_current_line_from_file
	test	ebx,ebx
	jz	ta_process_after_replaced
	lea	eax,[edi+1+4+20]
	cmp	eax,[ta_memory_end]
	ja	ta_out_of_memory
	mov	ecx,[ebx+4]
	call	ta_store_number_symbol
	jmp	ta_process_after_replaced
    ta_get_current_line_from_file:
	mov	ebx,[ta_current_line]
      ta_find_line_from_file:
	test	ebx,ebx
	jz	ta_line_from_file_found
	test	byte [ebx+7],80h
	jz	ta_line_from_file_found
	mov	ebx,[ebx+8]
	jmp	ta_find_line_from_file
      ta_line_from_file_found:
	ret

ta_process_macro_operators:
	xor	dl,dl
	mov	ebp,edi
      ta_before_macro_operators:
	mov	edi,esi
	lods	byte [esi]
	cmp	al,'`'
	je	ta_symbol_conversion
	cmp	al,'#'
	je	ta_concatenation
	cmp	al,1Ah
	je	ta_symbol_before_macro_operators
	cmp	al,3Bh
	je	ta_no_more_macro_operators
	cmp	al,22h
	je	ta_string_before_macro_operators
	xor	dl,dl
	or	al,al
	jnz	ta_before_macro_operators
	mov	edi,esi
	ret
      ta_no_more_macro_operators:
	mov	edi,ebp
	ret
      ta_symbol_before_macro_operators:
	mov	dl,1Ah
	mov	ebx,esi
	lods	byte [esi]
	movzx	ecx,al
	jecxz	ta_symbol_before_macro_operators_ok
	mov	edi,esi
	cmp	byte [esi],'\'
	je	ta_escaped_symbol
      ta_symbol_before_macro_operators_ok:
	add	esi,ecx
	jmp	ta_before_macro_operators
      ta_string_before_macro_operators:
	mov	dl,22h
	mov	ebx,esi
	lods	dword [esi]
	add	esi,eax
	jmp	ta_before_macro_operators
      ta_escaped_symbol:
	dec	byte [edi-1]
	dec	ecx
	inc	esi
	cmp	ecx,1
	rep	movs byte [edi],[esi]
	jne	ta_after_macro_operators
	mov	al,[esi-1]
	mov	ecx,ebx
	mov	ebx,ta_characters
	xlat	byte [ebx]
	mov	ebx,ecx
	or	al,al
	jnz	ta_after_macro_operators
	sub	edi,3
	mov	al,[esi-1]
	stos	byte [edi]
	xor	dl,dl
	jmp	ta_after_macro_operators
      ta_reduce_symbol_conversion:
	inc	esi
      ta_symbol_conversion:
	mov	edx,esi
	mov	al,[esi]
	cmp	al,1Ah
	jne	ta_symbol_character_conversion
	lods	word [esi]
	movzx	ecx,ah
	lea	ebx,[edi+3]
	jecxz	ta_convert_to_quoted_string
	cmp	byte [esi],'\'
	jne	ta_convert_to_quoted_string
	inc	esi
	dec	ecx
	dec	ebx
	jmp	ta_convert_to_quoted_string
      ta_symbol_character_conversion:
	cmp	al,22h
	je	ta_after_macro_operators
	cmp	al,'`'
	je	ta_reduce_symbol_conversion
	lea	ebx,[edi+5]
	xor	ecx,ecx
	or	al,al
	jz	ta_convert_to_quoted_string
	cmp	al,'#'
	je	ta_convert_to_quoted_string
	inc	ecx
      ta_convert_to_quoted_string:
	sub	ebx,edx
	ja	ta_shift_line_data
	mov	al,22h
	mov	dl,al
	stos	byte [edi]
	mov	ebx,edi
	mov	eax,ecx
	stos	dword [edi]
	rep	movs byte [edi],[esi]
	cmp	edi,esi
	je	ta_before_macro_operators
	jmp	ta_after_macro_operators
      ta_shift_line_data:
	push	ecx
	mov	edx,esi
	lea	esi,[ebp-1]
	add	ebp,ebx
	lea	edi,[ebp-1]
	lea	ecx,[esi+1]
	sub	ecx,edx
	std
	rep	movs byte [edi],[esi]
	cld
	pop	eax
	sub	edi,3
	mov	dl,22h
	mov	[edi-1],dl
	mov	ebx,edi
	mov	[edi],eax
	lea	esi,[edi+4+eax]
	jmp	ta_before_macro_operators
      ta_concatenation:
	cmp	dl,1Ah
	je	ta_symbol_concatenation
	cmp	dl,22h
	je	ta_string_concatenation
      ta_no_concatenation:
	cmp	esi,edi
	je	ta_before_macro_operators
	jmp	ta_after_macro_operators
      ta_symbol_concatenation:
	cmp	byte [esi],1Ah
	jne	ta_no_concatenation
	inc	esi
	lods	byte [esi]
	movzx	ecx,al
	jecxz	ta_do_symbol_concatenation
	cmp	byte [esi],'\'
	je	ta_concatenate_escaped_symbol
      ta_do_symbol_concatenation:
	add	[ebx],cl
	jc	ta_name_too_long
	rep	movs byte [edi],[esi]
	jmp	ta_after_macro_operators
      ta_concatenate_escaped_symbol:
	inc	esi
	dec	ecx
	jz	ta_do_symbol_concatenation
	movzx	eax,byte [esi]
	cmp	byte [ta_characters+eax],0
	jne	ta_do_symbol_concatenation
	sub	esi,3
	jmp	ta_no_concatenation
      ta_string_concatenation:
	cmp	byte [esi],22h
	je	ta_do_string_concatenation
	cmp	byte [esi],'`'
	jne	ta_no_concatenation
      ta_concatenate_converted_symbol:
	inc	esi
	mov	al,[esi]
	cmp	al,'`'
	je	ta_concatenate_converted_symbol
	cmp	al,22h
	je	ta_do_string_concatenation
	cmp	al,1Ah
	jne	ta_concatenate_converted_symbol_character
	inc	esi
	lods	byte [esi]
	movzx	ecx,al
	jecxz	ta_finish_concatenating_converted_symbol
	cmp	byte [esi],'\'
	jne	ta_finish_concatenating_converted_symbol
	inc	esi
	dec	ecx
      ta_finish_concatenating_converted_symbol:
	add	[ebx],ecx
	rep	movs byte [edi],[esi]
	jmp	ta_after_macro_operators
      ta_concatenate_converted_symbol_character:
	or	al,al
	jz	ta_after_macro_operators
	cmp	al,'#'
	je	ta_after_macro_operators
	inc	dword [ebx]
	movs	byte [edi],[esi]
	jmp	ta_after_macro_operators
      ta_do_string_concatenation:
	inc	esi
	lods	dword [esi]
	mov	ecx,eax
	add	[ebx],eax
	rep	movs byte [edi],[esi]
      ta_after_macro_operators:
	lods	byte [esi]
	cmp	al,'`'
	je	ta_symbol_conversion
	cmp	al,'#'
	je	ta_concatenation
	stos	byte [edi]
	cmp	al,1Ah
	je	ta_symbol_after_macro_operators
	cmp	al,3Bh
	je	ta_no_more_macro_operators
	cmp	al,22h
	je	ta_string_after_macro_operators
	xor	dl,dl
	or	al,al
	jnz	ta_after_macro_operators
	ret
      ta_symbol_after_macro_operators:
	mov	dl,1Ah
	mov	ebx,edi
	lods	byte [esi]
	stos	byte [edi]
	movzx	ecx,al
	jecxz	ta_symbol_after_macro_operatorss_ok
	cmp	byte [esi],'\'
	je	ta_escaped_symbol
      ta_symbol_after_macro_operatorss_ok:
	rep	movs byte [edi],[esi]
	jmp	ta_after_macro_operators
      ta_string_after_macro_operators:
	mov	dl,22h
	mov	ebx,edi
	lods	dword [esi]
	stos	dword [edi]
	mov	ecx,eax
	rep	movs byte [edi],[esi]
	jmp	ta_after_macro_operators

ta_use_macro:
	push	[ta_free_additional_memory]
	push	[ta_macro_symbols]
	mov	[ta_macro_symbols],0
	push	[ta_counter_limit]
	push	dword [edx+4]
	mov	dword [edx+4],1
	push	edx
	mov	ebx,esi
	mov	esi,[edx+8]
	mov	eax,[edx+12]
	mov	[ta_macro_line],eax
	mov	[ta_counter_limit],0
	xor	ebp,ebp
      ta_process_macro_arguments:
	mov	al,[esi]
	or	al,al
	jz	ta_arguments_end
	cmp	al,'{'
	je	ta_arguments_end
	inc	esi
	cmp	al,'['
	jne	ta_get_macro_arguments
	mov	ebp,esi
	inc	esi
	inc	[ta_counter_limit]
      ta_get_macro_arguments:
	call	ta_get_macro_argument
	lods	byte [esi]
	cmp	al,','
	je	ta_next_argument
	cmp	al,']'
	je	ta_next_arguments_group
	cmp	al,'&'
	je	ta_arguments_end
	dec	esi
	jmp	ta_arguments_end
      ta_next_argument:
	cmp	byte [ebx],','
	jne	ta_process_macro_arguments
	inc	ebx
	jmp	ta_process_macro_arguments
      ta_next_arguments_group:
	cmp	byte [ebx],','
	jne	ta_arguments_end
	inc	ebx
	inc	[ta_counter_limit]
	mov	esi,ebp
	jmp	ta_process_macro_arguments
      ta_get_macro_argument:
	lods	byte [esi]
	movzx	ecx,al
	mov	eax,[ta_counter_limit]
	call	ta_add_macro_symbol
	add	esi,ecx
	xor	eax,eax
	mov	[ta_default_argument_value],eax
	cmp	byte [esi],'*'
	je	ta_required_value
	cmp	byte [esi],':'
	je	ta_get_default_value
	cmp	byte [esi],'='
	jne	ta_default_value_ok
      ta_get_default_value:
	inc	esi
	mov	[ta_default_argument_value],esi
	or	[ta_skip_default_argument_value],-1
	call	ta_skip_macro_argument_value
	jmp	ta_default_value_ok
      ta_required_value:
	inc	esi
	or	[ta_default_argument_value],-1
      ta_default_value_ok:
	xchg	esi,ebx
	mov	[edx+12],esi
	mov	[ta_skip_default_argument_value],0
	cmp	byte [ebx],'&'
	je	ta_greedy_macro_argument
	call	ta_skip_macro_argument_value
	call	ta_finish_macro_argument
	jmp	ta_got_macro_argument
      ta_greedy_macro_argument:
	call	ta_skip_foreign_line
	dec	esi
	mov	eax,[edx+12]
	mov	ecx,esi
	sub	ecx,eax
	mov	[edx+8],ecx
      ta_got_macro_argument:
	xchg	esi,ebx
	cmp	dword [edx+8],0
	jne	ta_macro_argument_ok
	mov	eax,[ta_default_argument_value]
	or	eax,eax
	jz	ta_macro_argument_ok
	cmp	eax,-1
	je	ta_invalid_macro_arguments
	mov	[edx+12],eax
	call	ta_finish_macro_argument
      ta_macro_argument_ok:
	ret
      ta_finish_macro_argument:
	mov	eax,[edx+12]
	mov	ecx,esi
	sub	ecx,eax
	cmp	byte [eax],'<'
	jne	ta_argument_value_length_ok
	inc	dword [edx+12]
	sub	ecx,2
	or	ecx,80000000h
      ta_argument_value_length_ok:
	mov	[edx+8],ecx
	ret
      ta_arguments_end:
	cmp	byte [ebx],0
	jne	ta_invalid_macro_arguments
	mov	eax,[esp+4]
	dec	eax
	call	ta_process_macro
	pop	edx
	pop	dword [edx+4]
	pop	[ta_counter_limit]
	pop	[ta_macro_symbols]
	pop	[ta_free_additional_memory]
	jmp	ta_line_preprocessed
ta_use_instant_macro:
	push	edi
	push	[ta_current_line]
	push	esi
	mov	eax,[ta_error_line]
	mov	[ta_current_line],eax
	mov	[ta_macro_line],eax
	mov	esi,[ta_instant_macro_start]
	cmp	[ta_base_code],10h
	jae	ta_do_match
	cmp	[ta_base_code],0
	jne	ta_do_irp
	call	ta_precalculate_value
	cmp	eax,0
	jl	ta_value_out_of_range
	push	[ta_free_additional_memory]
	push	[ta_macro_symbols]
	mov	[ta_macro_symbols],0
	push	[ta_counter_limit]
	mov	[ta_struc_name],0
	mov	[ta_counter_limit],eax
	lods	byte [esi]
	or	al,al
	jz	ta_rept_counters_ok
	cmp	al,'{'
	je	ta_rept_counters_ok
	cmp	al,1Ah
	jne	ta_invalid_macro_arguments
      ta_add_rept_counter:
	lods	byte [esi]
	movzx	ecx,al
	xor	eax,eax
	call	ta_add_macro_symbol
	add	esi,ecx
	xor	eax,eax
	mov	dword [edx+12],eax
	inc	eax
	mov	dword [edx+8],eax
	lods	byte [esi]
	cmp	al,':'
	jne	ta_rept_counter_added
	push	edx
	call	ta_precalculate_value
	mov	edx,eax
	add	edx,[ta_counter_limit]
	jo	ta_value_out_of_range
	pop	edx
	mov	dword [edx+8],eax
	lods	byte [esi]
      ta_rept_counter_added:
	cmp	al,','
	jne	ta_rept_counters_ok
	lods	byte [esi]
	cmp	al,1Ah
	jne	ta_invalid_macro_arguments
	jmp	ta_add_rept_counter
      ta_rept_counters_ok:
	dec	esi
	cmp	[ta_counter_limit],0
	je	ta_instant_macro_finish
      ta_instant_macro_parameters_ok:
	xor	eax,eax
	call	ta_process_macro
      ta_instant_macro_finish:
	pop	[ta_counter_limit]
	pop	[ta_macro_symbols]
	pop	[ta_free_additional_memory]
      ta_instant_macro_done:
	pop	ebx esi edx
	cmp	byte [ebx],0
	je	ta_line_preprocessed
	mov	[ta_current_line],edi
	mov	ecx,4
	rep	movs dword [edi],[esi]
	test	[ta_macro_status],0Fh
	jz	ta_instant_macro_attached_line
	mov	ax,3Bh
	stos	word [edi]
      ta_instant_macro_attached_line:
	mov	esi,ebx
	sub	edx,ebx
	mov	ecx,edx
	call	ta_move_data
	jmp	ta_initial_preprocessing_ok
      ta_precalculate_value:
	push	edi
	call	ta_convert_expression
	mov	al,')'
	stosb
	push	esi
	mov	esi,[esp+4]
	mov	[ta_error_line],0
	mov	[ta_value_size],0
	call	ta_calculate_expression
	cmp	[ta_error_line],0
	je	ta_value_precalculated
	jmp	[ta_error]
      ta_value_precalculated:
	mov	eax,[edi]
	mov	ecx,[edi+4]
	cdq
	cmp	edx,ecx
	jne	ta_value_out_of_range
	cmp	dl,[edi+13]
	jne	ta_value_out_of_range
	pop	esi edi
	ret
ta_do_irp:
	cmp	byte [esi],1Ah
	jne	ta_invalid_macro_arguments
	movzx	eax,byte [esi+1]
	lea	esi,[esi+2+eax]
	lods	byte [esi]
	cmp	[ta_base_code],1
	ja	ta_irps_name_ok
	cmp	al,':'
	je	ta_irp_with_default_value
	cmp	al,'='
	je	ta_irp_with_default_value
	cmp	al,'*'
	jne	ta_irp_name_ok
	lods	byte [esi]
      ta_irp_name_ok:
	cmp	al,','
	jne	ta_invalid_macro_arguments
	jmp	ta_irp_parameters_start
      ta_irp_with_default_value:
	xor	ebp,ebp
	or	[ta_skip_default_argument_value],-1
	call	ta_skip_macro_argument_value
	cmp	byte [esi],','
	jne	ta_invalid_macro_arguments
	inc	esi
	jmp	ta_irp_parameters_start
      ta_irps_name_ok:
	cmp	al,','
	jne	ta_invalid_macro_arguments
	cmp	[ta_base_code],3
	je	ta_irp_parameters_start
	mov	al,[esi]
	or	al,al
	jz	ta_instant_macro_done
	cmp	al,'{'
	je	ta_instant_macro_done
      ta_irp_parameters_start:
	xor	eax,eax
	push	[ta_free_additional_memory]
	push	[ta_macro_symbols]
	mov	[ta_macro_symbols],eax
	push	[ta_counter_limit]
	mov	[ta_counter_limit],eax
	mov	[ta_struc_name],eax
	cmp	[ta_base_code],3
	je	ta_get_irpv_parameter
	mov	ebx,esi
	cmp	[ta_base_code],2
	je	ta_get_irps_parameter
	mov	edx,[ta_parameters_end]
	mov	al,[edx]
	push	eax
	mov	byte [edx],0
      ta_get_irp_parameter:
	inc	[ta_counter_limit]
	mov	esi,[ta_instant_macro_start]
	inc	esi
	call	ta_get_macro_argument
	cmp	byte [ebx],','
	jne	ta_irp_parameters_end
	inc	ebx
	jmp	ta_get_irp_parameter
      ta_irp_parameters_end:
	mov	esi,ebx
	pop	eax
	mov	[esi],al
	jmp	ta_instant_macro_parameters_ok
      ta_get_irps_parameter:
	mov	esi,[ta_instant_macro_start]
	inc	esi
	lods	byte [esi]
	movzx	ecx,al
	inc	[ta_counter_limit]
	mov	eax,[ta_counter_limit]
	call	ta_add_macro_symbol
	mov	[edx+12],ebx
	cmp	byte [ebx],1Ah
	je	ta_irps_symbol
	cmp	byte [ebx],22h
	je	ta_irps_quoted_string
	mov	eax,1
	jmp	ta_irps_parameter_ok
      ta_irps_quoted_string:
	mov	eax,[ebx+1]
	add	eax,1+4
	jmp	ta_irps_parameter_ok
      ta_irps_symbol:
	movzx	eax,byte [ebx+1]
	add	eax,1+1
      ta_irps_parameter_ok:
	mov	[edx+8],eax
	add	ebx,eax
	cmp	byte [ebx],0
	je	ta_irps_parameters_end
	cmp	byte [ebx],'{'
	jne	ta_get_irps_parameter
      ta_irps_parameters_end:
	mov	esi,ebx
	jmp	ta_instant_macro_parameters_ok
      ta_get_irpv_parameter:
	lods	byte [esi]
	cmp	al,1Ah
	jne	ta_invalid_macro_arguments
	lods	byte [esi]
	mov	ebp,esi
	mov	cl,al
	mov	ch,10b
	call	ta_get_preprocessor_symbol
	jc	ta_instant_macro_finish
	test	edx,edx
	jz	ta_invalid_use_of_symbol
	push	edx
      ta_mark_variable_value:
	inc	[ta_counter_limit]
	mov	[edx+4],ebp
      ta_next_variable_value:
	mov	edx,[edx]
	or	edx,edx
	jz	ta_variable_values_marked
	mov	eax,[edx+4]
	cmp	eax,1
	jbe	ta_next_variable_value
	mov	esi,ebp
	movzx	ecx,byte [esi-1]
	xchg	edi,eax
	repe	cmps byte [esi],[edi]
	xchg	edi,eax
	je	ta_mark_variable_value
	jmp	ta_next_variable_value
      ta_variable_values_marked:
	pop	edx
	push	[ta_counter_limit]
      ta_add_irpv_value:
	push	edx
	mov	esi,[ta_instant_macro_start]
	inc	esi
	lods	byte [esi]
	movzx	ecx,al
	mov	eax,[esp+4]
	call	ta_add_macro_symbol
	mov	ebx,edx
	pop	edx
	mov	ecx,[edx+12]
	mov	eax,[edx+8]
	mov	[ebx+12],eax
	mov	[ebx+8],ecx
      ta_collect_next_variable_value:
	mov	edx,[edx]
	or	edx,edx
	jz	ta_variable_values_collected
	cmp	ebp,[edx+4]
	jne	ta_collect_next_variable_value
	dec	dword [esp]
	jnz	ta_add_irpv_value
      ta_variable_values_collected:
	pop	eax
	mov	esi,ebp
	movzx	ecx,byte [esi-1]
	add	esi,ecx
	cmp	byte [esi],0
	je	ta_instant_macro_parameters_ok
	cmp	byte [esi],'{'
	jne	ta_invalid_macro_arguments
	jmp	ta_instant_macro_parameters_ok

ta_do_match:
	mov	ebx,esi
	call	ta_skip_pattern
	call	ta_exact_match
	mov	edx,edi
	mov	al,[ebx]
	cmp	al,1Ah
	je	ta_free_match
	cmp	al,','
	jne	ta_instant_macro_done
	cmp	esi,[ta_parameters_end]
	je	ta_matched_pattern
	jmp	ta_instant_macro_done
      ta_free_match:
	add	edx,12
	cmp	edx,[ta_memory_end]
	ja	ta_out_of_memory
	mov	[edx-12],ebx
	mov	[edx-8],esi
	call	ta_skip_match_element
	jc	ta_try_different_matching
	mov	[edx-4],esi
	movzx	eax,byte [ebx+1]
	lea	ebx,[ebx+2+eax]
	cmp	byte [ebx],1Ah
	je	ta_free_match
      ta_find_exact_match:
	call	ta_exact_match
	cmp	esi,[ta_parameters_end]
	je	ta_end_matching
	cmp	byte [ebx],1Ah
	je	ta_free_match
	mov	ebx,[edx-12]
	movzx	eax,byte [ebx+1]
	lea	ebx,[ebx+2+eax]
	mov	esi,[edx-4]
	jmp	ta_match_more_elements
      ta_try_different_matching:
	sub	edx,12
	cmp	edx,edi
	je	ta_instant_macro_done
	mov	ebx,[edx-12]
	movzx	eax,byte [ebx+1]
	lea	ebx,[ebx+2+eax]
	cmp	byte [ebx],1Ah
	je	ta_try_different_matching
	mov	esi,[edx-4]
      ta_match_more_elements:
	call	ta_skip_match_element
	jc	ta_try_different_matching
	mov	[edx-4],esi
	jmp	ta_find_exact_match
      ta_skip_match_element:
	cmp	esi,[ta_parameters_end]
	je	ta_cannot_match
	mov	al,[esi]
	cmp	al,1Ah
	je	ta_skip_match_symbol
	cmp	al,22h
	je	ta_skip_match_quoted_string
	add	esi,1
	ret
      ta_skip_match_quoted_string:
	mov	eax,[esi+1]
	add	esi,5
	jmp	ta_skip_match_ok
      ta_skip_match_symbol:
	movzx	eax,byte [esi+1]
	add	esi,2
      ta_skip_match_ok:
	add	esi,eax
	ret
      ta_cannot_match:
	stc
	ret
      ta_exact_match:
	cmp	esi,[ta_parameters_end]
	je	ta_exact_match_complete
	mov	ah,[esi]
	mov	al,[ebx]
	cmp	al,','
	je	ta_exact_match_complete
	cmp	al,1Ah
	je	ta_exact_match_complete
	cmp	al,'='
	je	ta_match_verbatim
	call	ta_match_elements
	je	ta_exact_match
      ta_exact_match_complete:
	ret
      ta_match_verbatim:
	inc	ebx
	call	ta_match_elements
	je	ta_exact_match
	dec	ebx
	ret
      ta_match_elements:
	mov	al,[ebx]
	cmp	al,1Ah
	je	ta_match_symbols
	cmp	al,22h
	je	ta_match_quoted_strings
	cmp	al,ah
	je	ta_symbol_characters_matched
	ret
      ta_symbol_characters_matched:
	lea	ebx,[ebx+1]
	lea	esi,[esi+1]
	ret
      ta_match_quoted_strings:
	mov	ecx,[ebx+1]
	add	ecx,5
	jmp	ta_compare_elements
      ta_match_symbols:
	movzx	ecx,byte [ebx+1]
	add	ecx,2
      ta_compare_elements:
	mov	eax,esi
	mov	ebp,edi
	mov	edi,ebx
	repe	cmps byte [esi],[edi]
	jne	ta_elements_mismatch
	mov	ebx,edi
	mov	edi,ebp
	ret
      ta_elements_mismatch:
	mov	esi,eax
	mov	edi,ebp
	ret
      ta_end_matching:
	cmp	byte [ebx],','
	jne	ta_instant_macro_done
      ta_matched_pattern:
	xor	eax,eax
	push	[ta_free_additional_memory]
	push	[ta_macro_symbols]
	mov	[ta_macro_symbols],eax
	push	[ta_counter_limit]
	mov	[ta_counter_limit],eax
	mov	[ta_struc_name],eax
	push	esi edi edx
      ta_add_matched_symbol:
	cmp	edi,[esp]
	je	ta_matched_symbols_ok
	mov	esi,[edi]
	inc	esi
	lods	byte [esi]
	movzx	ecx,al
	xor	eax,eax
	call	ta_add_macro_symbol
	mov	eax,[edi+4]
	mov	dword [edx+12],eax
	mov	ecx,[edi+8]
	sub	ecx,eax
	mov	dword [edx+8],ecx
	add	edi,12
	jmp	ta_add_matched_symbol
      ta_matched_symbols_ok:
	pop	edx edi esi
	jmp	ta_instant_macro_parameters_ok

ta_process_macro:
	push	dword [ta_macro_status]
	or	[ta_macro_status],10h
	push	[ta_counter]
	push	[ta_macro_block]
	push	[ta_macro_block_line]
	push	[ta_macro_block_line_number]
	push	[ta_struc_label]
	push	[ta_struc_name]
	push	eax
	push	[ta_current_line]
	lods	byte [esi]
	cmp	al,'{'
	je	ta_macro_instructions_start
	or	al,al
	jnz	ta_unexpected_characters
      ta_find_macro_instructions:
	mov	[ta_macro_line],esi
	add	esi,16+2
	lods	byte [esi]
	or	al,al
	jz	ta_find_macro_instructions
	cmp	al,'{'
	je	ta_macro_instructions_start
	cmp	al,3Bh
	jne	ta_unexpected_characters
	call	ta_skip_foreign_symbol
	jmp	ta_find_macro_instructions
      ta_macro_instructions_start:
	mov	ecx,80000000h
	mov	[ta_macro_block],esi
	mov	eax,[ta_macro_line]
	mov	[ta_macro_block_line],eax
	mov	[ta_macro_block_line_number],ecx
	xor	eax,eax
	mov	[ta_counter],eax
	cmp	[ta_counter_limit],eax
	je	ta_process_macro_line
	inc	[ta_counter]
      ta_process_macro_line:
	lods	byte [esi]
	or	al,al
	jz	ta_process_next_line
	cmp	al,'}'
	je	ta_macro_block_processed
	dec	esi
	mov	[ta_current_line],edi
	lea	eax,[edi+10h]
	cmp	eax,[ta_memory_end]
	jae	ta_out_of_memory
	mov	eax,[esp+4]
	or	eax,eax
	jz	ta_instant_macro_line_header
	stos	dword [edi]
	mov	eax,ecx
	stos	dword [edi]
	mov	eax,[esp]
	stos	dword [edi]
	mov	eax,[ta_macro_line]
	stos	dword [edi]
	jmp	ta_macro_line_header_ok
      ta_instant_macro_line_header:
	mov	eax,[esp]
	add	eax,16
      ta_find_defining_directive:
	inc	eax
	cmp	byte [eax-1],3Bh
	je	ta_defining_directive_ok
	cmp	byte [eax-1],1Ah
	jne	ta_find_defining_directive
	push	eax
	movzx	eax,byte [eax]
	inc	eax
	add	[esp],eax
	pop	eax
	jmp	ta_find_defining_directive
      ta_defining_directive_ok:
	stos	dword [edi]
	mov	eax,ecx
	stos	dword [edi]
	mov	eax,[ta_macro_line]
	stos	dword [edi]
	stos	dword [edi]
      ta_macro_line_header_ok:
	or	[ta_macro_status],20h
	push	ebx ecx
	test	[ta_macro_status],0Fh
	jz	ta_process_macro_line_element
	mov	ax,3Bh
	stos	word [edi]
      ta_process_macro_line_element:
	lea	eax,[edi+100h]
	cmp	eax,[ta_memory_end]
	jae	ta_out_of_memory
	lods	byte [esi]
	cmp	al,'}'
	je	ta_macro_line_processed
	or	al,al
	jz	ta_macro_line_processed
	cmp	al,1Ah
	je	ta_process_macro_symbol
	cmp	al,3Bh
	je	ta_macro_foreign_line
	and	[ta_macro_status],not 20h
	stos	byte [edi]
	cmp	al,22h
	jne	ta_process_macro_line_element
      ta_copy_macro_string:
	mov	ecx,[esi]
	add	ecx,4
	call	ta_move_data
	jmp	ta_process_macro_line_element
      ta_process_macro_symbol:
	push	esi edi
	test	[ta_macro_status],20h
	jz	ta_not_macro_directive
	movzx	ecx,byte [esi]
	inc	esi
	mov	edi,ta_macro_directives
	call	ta_get_directive
	jnc	ta_process_macro_directive
	dec	esi
	jmp	ta_not_macro_directive
      ta_process_macro_directive:
	mov	edx,eax
	pop	edi eax
	mov	byte [edi],0
	inc	edi
	pop	ecx ebx
	jmp	near edx
      ta_not_macro_directive:
	and	[ta_macro_status],not 20h
	movzx	ecx,byte [esi]
	inc	esi
	mov	eax,[ta_counter]
	call	ta_get_macro_symbol
	jnc	ta_group_macro_symbol
	xor	eax,eax
	cmp	[ta_counter],eax
	je	ta_multiple_macro_symbol_values
	call	ta_get_macro_symbol
	jc	ta_not_macro_symbol
      ta_replace_macro_symbol:
	pop	edi eax
	mov	ecx,[edx+8]
	mov	edx,[edx+12]
	or	edx,edx
	jz	ta_replace_macro_counter
	and	ecx,not 80000000h
	xchg	esi,edx
	call	ta_move_data
	mov	esi,edx
	jmp	ta_process_macro_line_element
      ta_group_macro_symbol:
	xor	eax,eax
	cmp	[ta_counter],eax
	je	ta_replace_macro_symbol
	push	esi edx
	sub	esi,ecx
	call	ta_get_macro_symbol
	mov	ebx,edx
	pop	edx esi
	jc	ta_replace_macro_symbol
	cmp	edx,ebx
	ja	ta_replace_macro_symbol
	mov	edx,ebx
	jmp	ta_replace_macro_symbol
      ta_multiple_macro_symbol_values:
	inc	eax
	push	eax
	call	ta_get_macro_symbol
	pop	eax
	jc	ta_not_macro_symbol
	pop	edi
	push	ecx
	mov	ecx,[edx+8]
	mov	edx,[edx+12]
	xchg	esi,edx
	btr	ecx,31
	jc	ta_enclose_macro_symbol_value
	rep	movs byte [edi],[esi]
	jmp	ta_macro_symbol_value_ok
      ta_enclose_macro_symbol_value:
	mov	byte [edi],'<'
	inc	edi
	rep	movs byte [edi],[esi]
	mov	byte [edi],'>'
	inc	edi
      ta_macro_symbol_value_ok:
	cmp	eax,[ta_counter_limit]
	je	ta_multiple_macro_symbol_values_ok
	mov	byte [edi],','
	inc	edi
	mov	esi,edx
	pop	ecx
	push	edi
	sub	esi,ecx
	jmp	ta_multiple_macro_symbol_values
      ta_multiple_macro_symbol_values_ok:
	pop	ecx eax
	mov	esi,edx
	jmp	ta_process_macro_line_element
      ta_replace_macro_counter:
	mov	eax,[ta_counter]
	and	eax,not 80000000h
	jz	ta_group_macro_counter
	add	ecx,eax
	dec	ecx
	call	ta_store_number_symbol
	jmp	ta_process_macro_line_element
      ta_group_macro_counter:
	mov	edx,ecx
	xor	ecx,ecx
      ta_multiple_macro_counter_values:
	push	ecx edx
	add	ecx,edx
	call	ta_store_number_symbol
	pop	edx ecx
	inc	ecx
	cmp	ecx,[ta_counter_limit]
	je	ta_process_macro_line_element
	mov	byte [edi],','
	inc	edi
	jmp	ta_multiple_macro_counter_values
      ta_store_number_symbol:
	cmp	ecx,0
	jge	ta_numer_symbol_sign_ok
	neg	ecx
	mov	al,'-'
	stos	byte [edi]
      ta_numer_symbol_sign_ok:
	mov	ax,1Ah
	stos	word [edi]
	push	edi
	mov	eax,ecx
	mov	ecx,1000000000
	xor	edx,edx
	xor	bl,bl
      ta_store_number_digits:
	div	ecx
	push	edx
	or	bl,bl
	jnz	ta_store_number_digit
	cmp	ecx,1
	je	ta_store_number_digit
	or	al,al
	jz	ta_number_digit_ok
	not	bl
      ta_store_number_digit:
	add	al,30h
	stos	byte [edi]
      ta_number_digit_ok:
	mov	eax,ecx
	xor	edx,edx
	mov	ecx,10
	div	ecx
	mov	ecx,eax
	pop	eax
	or	ecx,ecx
	jnz	ta_store_number_digits
	pop	ebx
	mov	eax,edi
	sub	eax,ebx
	mov	[ebx-1],al
	ret
      ta_not_macro_symbol:
	pop	edi esi
	mov	al,1Ah
	stos	byte [edi]
	mov	al,[esi]
	inc	esi
	stos	byte [edi]
	cmp	byte [esi],'.'
	jne	ta_copy_raw_symbol
	mov	ebx,[esp+8+8]
	or	ebx,ebx
	jz	ta_copy_raw_symbol
	cmp	al,1
	je	ta_copy_struc_name
	xchg	esi,ebx
	movzx	ecx,byte [esi-1]
	add	[edi-1],cl
	jc	ta_name_too_long
	rep	movs byte [edi],[esi]
	xchg	esi,ebx
      ta_copy_raw_symbol:
	movzx	ecx,al
	rep	movs byte [edi],[esi]
	jmp	ta_process_macro_line_element
      ta_copy_struc_name:
	inc	esi
	xchg	esi,ebx
	movzx	ecx,byte [esi-1]
	mov	[edi-1],cl
	rep	movs byte [edi],[esi]
	xchg	esi,ebx
	mov	eax,[esp+8+12]
	cmp	byte [eax],3Bh
	je	ta_process_macro_line_element
	cmp	byte [eax],1Ah
	jne	ta_disable_replaced_struc_name
	mov	byte [eax],3Bh
	jmp	ta_process_macro_line_element
      ta_disable_replaced_struc_name:
	mov	ebx,[esp+8+8]
	push	esi edi
	lea	edi,[ebx-3]
	lea	esi,[edi-2]
	lea	ecx,[esi+1]
	sub	ecx,eax
	std
	rep	movs byte [edi],[esi]
	cld
	mov	word [eax],3Bh
	pop	edi esi
	jmp	ta_process_macro_line_element
      ta_skip_foreign_symbol:
	lods	byte [esi]
	movzx	eax,al
	add	esi,eax
      ta_skip_foreign_line:
	lods	byte [esi]
	cmp	al,1Ah
	je	ta_skip_foreign_symbol
	cmp	al,3Bh
	je	ta_skip_foreign_symbol
	cmp	al,22h
	je	ta_skip_foreign_string
	or	al,al
	jnz	ta_skip_foreign_line
	ret
      ta_skip_foreign_string:
	lods	dword [esi]
	add	esi,eax
	jmp	ta_skip_foreign_line
      ta_macro_foreign_line:
	call	ta_skip_foreign_symbol
      ta_macro_line_processed:
	mov	byte [edi],0
	inc	edi
	push	eax
	call	ta_preprocess_line
	pop	eax
	pop	ecx ebx
	cmp	al,'}'
	je	ta_macro_block_processed
      ta_process_next_line:
	inc	ecx
	mov	[ta_macro_line],esi
	add	esi,16+2
	jmp	ta_process_macro_line
      ta_macro_block_processed:
	call	ta_close_macro_block
	jc	ta_process_macro_line
	pop	[ta_current_line]
	add	esp,12
	pop	[ta_macro_block_line_number]
	pop	[ta_macro_block_line]
	pop	[ta_macro_block]
	pop	[ta_counter]
	pop	eax
	and	al,0F0h
	and	[ta_macro_status],0Fh
	or	[ta_macro_status],al
	ret

ta_local_symbols:
	lods	byte [esi]
	cmp	al,1Ah
	jne	ta_invalid_argument
	mov	byte [edi-1],3Bh
	xor	al,al
	stos	byte [edi]
      ta_make_local_symbol:
	push	ecx
	lods	byte [esi]
	movzx	ecx,al
	mov	eax,[ta_counter]
	call	ta_add_macro_symbol
	mov	[edx+12],edi
	movzx	eax,[ta_locals_counter]
	add	eax,ecx
	inc	eax
	cmp	eax,100h
	jae	ta_name_too_long
	lea	ebp,[edi+2+eax]
	cmp	ebp,[ta_memory_end]
	jae	ta_out_of_memory
	mov	ah,al
	mov	al,1Ah
	stos	word [edi]
	rep	movs byte [edi],[esi]
	mov	al,'?'
	stos	byte [edi]
	push	esi
	mov	esi,ta_locals_counter+1
	movzx	ecx,[ta_locals_counter]
	rep	movs byte [edi],[esi]
	pop	esi
	mov	eax,edi
	sub	eax,[edx+12]
	mov	[edx+8],eax
	xor	al,al
	stos	byte [edi]
	mov	eax,ta_locals_counter
	movzx	ecx,byte [eax]
      ta_counter_loop:
	inc	byte [eax+ecx]
	cmp	byte [eax+ecx],'9'+1
	jb	ta_counter_ok
	jne	ta_letter_digit
	mov	byte [eax+ecx],'A'
	jmp	ta_counter_ok
      ta_letter_digit:
	cmp	byte [eax+ecx],'Z'+1
	jb	ta_counter_ok
	jne	ta_small_letter_digit
	mov	byte [eax+ecx],'a'
	jmp	ta_counter_ok
      ta_small_letter_digit:
	cmp	byte [eax+ecx],'z'+1
	jb	ta_counter_ok
	mov	byte [eax+ecx],'0'
	loop	ta_counter_loop
	inc	byte [eax]
	movzx	ecx,byte [eax]
	mov	byte [eax+ecx],'0'
      ta_counter_ok:
	pop	ecx
	lods	byte [esi]
	cmp	al,'}'
	je	ta_macro_block_processed
	or	al,al
	jz	ta_process_next_line
	cmp	al,','
	jne	ta_extra_characters_on_line
	dec	edi
	lods	byte [esi]
	cmp	al,1Ah
	je	ta_make_local_symbol
	jmp	ta_invalid_argument
ta_common_block:
	call	ta_close_macro_block
	jc	ta_process_macro_line
	mov	[ta_counter],0
	jmp	ta_new_macro_block
ta_forward_block:
	cmp	[ta_counter_limit],0
	je	ta_common_block
	call	ta_close_macro_block
	jc	ta_process_macro_line
	mov	[ta_counter],1
	jmp	ta_new_macro_block
ta_reverse_block:
	cmp	[ta_counter_limit],0
	je	ta_common_block
	call	ta_close_macro_block
	jc	ta_process_macro_line
	mov	eax,[ta_counter_limit]
	or	eax,80000000h
	mov	[ta_counter],eax
      ta_new_macro_block:
	mov	[ta_macro_block],esi
	mov	eax,[ta_macro_line]
	mov	[ta_macro_block_line],eax
	mov	[ta_macro_block_line_number],ecx
	jmp	ta_process_macro_line
ta_close_macro_block:
	cmp	esi,[ta_macro_block]
	je	ta_block_closed
	cmp	[ta_counter],0
	je	ta_block_closed
	jl	ta_reverse_counter
	mov	eax,[ta_counter]
	cmp	eax,[ta_counter_limit]
	je	ta_block_closed
	inc	[ta_counter]
	jmp	ta_continue_block
      ta_reverse_counter:
	mov	eax,[ta_counter]
	dec	eax
	cmp	eax,80000000h
	je	ta_block_closed
	mov	[ta_counter],eax
      ta_continue_block:
	mov	esi,[ta_macro_block]
	mov	eax,[ta_macro_block_line]
	mov	[ta_macro_line],eax
	mov	ecx,[ta_macro_block_line_number]
	stc
	ret
      ta_block_closed:
	clc
	ret
ta_get_macro_symbol:
	push	ecx
	call	ta_find_macro_symbol_leaf
	jc	ta_macro_symbol_not_found
	mov	edx,[ebx]
	mov	ebx,esi
      ta_try_macro_symbol:
	or	edx,edx
	jz	ta_macro_symbol_not_found
	mov	ecx,[esp]
	mov	edi,[edx+4]
	repe	cmps byte [esi],[edi]
	je	ta_macro_symbol_found
	mov	esi,ebx
	mov	edx,[edx]
	jmp	ta_try_macro_symbol
      ta_macro_symbol_found:
	pop	ecx
	clc
	ret
      ta_macro_symbol_not_found:
	pop	ecx
	stc
	ret
      ta_find_macro_symbol_leaf:
	shl	eax,8
	mov	al,cl
	mov	ebp,eax
	mov	ebx,ta_macro_symbols
      ta_follow_macro_symbols_tree:
	mov	edx,[ebx]
	or	edx,edx
	jz	ta_no_such_macro_symbol
	xor	eax,eax
	shr	ebp,1
	adc	eax,0
	lea	ebx,[edx+eax*4]
	or	ebp,ebp
	jnz	ta_follow_macro_symbols_tree
	add	ebx,8
	clc
	ret
      ta_no_such_macro_symbol:
	stc
	ret
ta_add_macro_symbol:
	push	ebx ebp
	call	ta_find_macro_symbol_leaf
	jc	ta_extend_macro_symbol_tree
	mov	eax,[ebx]
      ta_make_macro_symbol:
	mov	edx,[ta_free_additional_memory]
	add	edx,16
	cmp	edx,[ta_labels_list]
	ja	ta_out_of_memory
	xchg	edx,[ta_free_additional_memory]
	mov	[ebx],edx
	mov	[edx],eax
	mov	[edx+4],esi
	pop	ebp ebx
	ret
      ta_extend_macro_symbol_tree:
	mov	edx,[ta_free_additional_memory]
	add	edx,16
	cmp	edx,[ta_labels_list]
	ja	ta_out_of_memory
	xchg	edx,[ta_free_additional_memory]
	xor	eax,eax
	mov	[edx],eax
	mov	[edx+4],eax
	mov	[edx+8],eax
	mov	[edx+12],eax
	shr	ebp,1
	adc	eax,0
	mov	[ebx],edx
	lea	ebx,[edx+eax*4]
	or	ebp,ebp
	jnz	ta_extend_macro_symbol_tree
	add	ebx,8
	xor	eax,eax
	jmp	ta_make_macro_symbol

ta_include_file:
	lods	byte [esi]
	cmp	al,22h
	jne	ta_invalid_argument
	lods	dword [esi]
	cmp	byte [esi+eax],0
	jne	ta_extra_characters_on_line
	push	esi
	push	edi
	mov	ebx,[ta_current_line]
      ta_find_current_file_path:
	mov	esi,[ebx]
	test	byte [ebx+7],80h
	jz	ta_copy_current_file_path
	mov	ebx,[ebx+8]
	jmp	ta_find_current_file_path
      ta_copy_current_file_path:
	lods	byte [esi]
	stos	byte [edi]
	or	al,al
	jnz	ta_copy_current_file_path
      ta_cut_current_file_name:
	cmp	edi,[esp]
	je	ta_current_file_path_ok
	cmp	byte [edi-1],'\'
	je	ta_current_file_path_ok
	cmp	byte [edi-1],'/'
	je	ta_current_file_path_ok
	dec	edi
	jmp	ta_cut_current_file_name
      ta_current_file_path_ok:
	mov	esi,[esp+4]
	call	ta_expand_path
	pop	edx
	mov	esi,edx
	call	ta_open
	jnc	ta_include_path_ok
	mov	ebp,[ta_include_paths]
      ta_try_include_directories:
	mov	edi,esi
	mov	esi,ebp
	cmp	byte [esi],0
	je	ta_try_in_current_directory
	push	ebp
	push	edi
	call	ta_get_include_directory
	mov	[esp+4],esi
	mov	esi,[esp+8]
	call	ta_expand_path
	pop	edx
	mov	esi,edx
	call	ta_open
	pop	ebp
	jnc	ta_include_path_ok
	jmp	ta_try_include_directories
	mov	edi,esi
      ta_try_in_current_directory:
	mov	esi,[esp]
	push	edi
	call	ta_expand_path
	pop	edx
	mov	esi,edx
	call	ta_open
	jc	ta_file_not_found
      ta_include_path_ok:
	mov	edi,[esp]
      ta_copy_preprocessed_path:
	lods	byte [esi]
	stos	byte [edi]
	or	al,al
	jnz	ta_copy_preprocessed_path
	pop	esi
	lea	ecx,[edi-1]
	sub	ecx,esi
	mov	[esi-4],ecx
	push	dword [ta_macro_status]
	and	[ta_macro_status],0Fh
	call	ta_preprocess_file
	pop	eax
	and	al,0F0h
	and	[ta_macro_status],0Fh
	or	[ta_macro_status],al
	jmp	ta_line_preprocessed
