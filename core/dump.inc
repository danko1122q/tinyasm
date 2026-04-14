ta_dump_symbols:
	mov	edi,[ta_code_start]
	call	ta_setup_dump_header
	mov	esi,[ta_input_file]
	call	ta_copy_asciiz
	cmp	edi,[ta_tagged_blocks]
	jae	ta_out_of_memory
	mov	eax,edi
	sub	eax,ebx
	mov	[ebx-40h+0Ch],eax
	mov	esi,[ta_output_file]
	call	ta_copy_asciiz
	cmp	edi,[ta_tagged_blocks]
	jae	ta_out_of_memory
	mov	edx,[ta_symbols_stream]
	mov	ebp,[ta_free_additional_memory]
	and	[ta_number_of_sections],0
	cmp	[ta_output_format],4
	je	ta_prepare_strings_table
	cmp	[ta_output_format],5
	jne	ta_strings_table_ready
	bt	[ta_format_flags],0
	jc	ta_strings_table_ready
      ta_prepare_strings_table:
	cmp	edx,ebp
	je	ta_strings_table_ready
	mov	al,[edx]
	test	al,al
	jz	ta_prepare_string
	cmp	al,80h
	je	ta_prepare_string
	add	edx,0Ch
	cmp	al,0C0h
	jb	ta_prepare_strings_table
	add	edx,4
	jmp	ta_prepare_strings_table
      ta_prepare_string:
	mov	esi,edi
	sub	esi,ebx
	xchg	esi,[edx+4]
	test	al,al
	jz	ta_prepare_section_string
	or	dword [edx+4],1 shl 31
	add	edx,0Ch
      ta_prepare_external_string:
	mov	ecx,[esi]
	add	esi,4
	rep	movs byte [edi],[esi]
	mov	byte [edi],0
	inc	edi
	cmp	edi,[ta_tagged_blocks]
	jae	ta_out_of_memory
	jmp	ta_prepare_strings_table
      ta_prepare_section_string:
	mov	ecx,[ta_number_of_sections]
	mov	eax,ecx
	inc	eax
	mov	[ta_number_of_sections],eax
	xchg	eax,[edx+4]
	shl	ecx,2
	add	ecx,[ta_free_additional_memory]
	mov	[ecx],eax
	add	edx,20h
	test	esi,esi
	jz	ta_prepare_default_section_string
	cmp	[ta_output_format],5
	jne	ta_prepare_external_string
	bt	[ta_format_flags],0
	jc	ta_prepare_external_string
	mov	esi,[esi]
	add	esi,[ta_resource_data]
      ta_copy_elf_section_name:
	lods	byte [esi]
	cmp	edi,[ta_tagged_blocks]
	jae	ta_out_of_memory
	stos	byte [edi]
	test	al,al
	jnz	ta_copy_elf_section_name
	jmp	ta_prepare_strings_table
      ta_prepare_default_section_string:
	mov	eax,'.fla'
	stos	dword [edi]
	mov	ax,'t'
	stos	word [edi]
	cmp	edi,[ta_tagged_blocks]
	jae	ta_out_of_memory
	jmp	ta_prepare_strings_table
      ta_strings_table_ready:
	mov	edx,[ta_tagged_blocks]
	mov	ebp,[ta_memory_end]
	sub	ebp,[ta_labels_list]
	add	ebp,edx
      ta_prepare_labels_dump:
	cmp	edx,ebp
	je	ta_labels_dump_ok
	mov	eax,[edx+24]
	test	eax,eax
	jz	ta_label_dump_name_ok
	cmp	eax,[ta_memory_start]
	jb	ta_label_name_outside_source
	cmp	eax,[ta_source_start]
	ja	ta_label_name_outside_source
	sub	eax,[ta_memory_start]
	dec	eax
	mov	[edx+24],eax
	jmp	ta_label_dump_name_ok
      ta_label_name_outside_source:
	mov	esi,eax
	mov	eax,edi
	sub	eax,ebx
	or	eax,1 shl 31
	mov	[edx+24],eax
	movzx	ecx,byte [esi-1]
	lea	eax,[edi+ecx+1]
	cmp	edi,[ta_tagged_blocks]
	jae	ta_out_of_memory
	rep	movsb
	xor	al,al
	stosb
      ta_label_dump_name_ok:
	mov	eax,[edx+28]
	test	eax,eax
	jz	ta_label_dump_line_ok
	sub	eax,[ta_memory_start]
	mov	[edx+28],eax
      ta_label_dump_line_ok:
	test	byte [edx+9],4
	jz	ta_convert_base_symbol_for_label
	xor	eax,eax
	mov	[edx],eax
	mov	[edx+4],eax
	jmp	ta_base_symbol_for_label_ok
      ta_convert_base_symbol_for_label:
	mov	eax,[edx+20]
	test	eax,eax
	jz	ta_base_symbol_for_label_ok
	cmp	eax,[ta_symbols_stream]
	mov	eax,[eax+4]
	jae	ta_base_symbol_for_label_ok
	xor	eax,eax
      ta_base_symbol_for_label_ok:
	mov	[edx+20],eax
	mov	ax,[ta_current_pass]
	cmp	ax,[edx+16]
	je	ta_label_defined_flag_ok
	and	byte [edx+8],not 1
      ta_label_defined_flag_ok:
	cmp	ax,[edx+18]
	je	ta_label_used_flag_ok
	and	byte [edx+8],not 8
      ta_label_used_flag_ok:
	add	edx,LABEL_STRUCTURE_SIZE
	jmp	ta_prepare_labels_dump
      ta_labels_dump_ok:
	mov	eax,edi
	sub	eax,ebx
	mov	[ebx-40h+14h],eax
	add	eax,40h
	mov	[ebx-40h+18h],eax
	mov	ecx,[ta_memory_end]
	sub	ecx,[ta_labels_list]
	mov	[ebx-40h+1Ch],ecx
	add	eax,ecx
	mov	[ebx-40h+20h],eax
	mov	ecx,[ta_source_start]
	sub	ecx,[ta_memory_start]
	mov	[ebx-40h+24h],ecx
	add	eax,ecx
	mov	[ebx-40h+28h],eax
	mov	eax,[ta_number_of_sections]
	shl	eax,2
	mov	[ebx-40h+34h],eax
	call	ta_prepare_preprocessed_source
	mov	esi,[ta_labels_list]
	mov	ebp,edi
      ta_make_lines_dump:
	cmp	esi,[ta_tagged_blocks]
	je	ta_lines_dump_ok
	mov	eax,[esi-4]
	mov	ecx,[esi-8]
	sub	esi,8
	sub	esi,ecx
	cmp	eax,1
	je	ta_process_line_dump
	cmp	eax,2
	jne	ta_make_lines_dump
	add	dword [ebx-40h+3Ch],8
	jmp	ta_make_lines_dump
      ta_process_line_dump:
	push	ebx
	mov	ebx,[esi+8]
	mov	eax,[esi+4]
	sub	eax,[ta_code_start]
	add	eax,[ta_headers_size]
	test	byte [ebx+0Ah],1
	jz	ta_store_offset
	xor	eax,eax
      ta_store_offset:
	stos	dword [edi]
	mov	eax,[esi]
	sub	eax,[ta_memory_start]
	stos	dword [edi]
	mov	eax,[esi+4]
	xor	edx,edx
	xor	cl,cl
	sub	eax,[ebx]
	sbb	edx,[ebx+4]
	sbb	cl,[ebx+8]
	stos	dword [edi]
	mov	eax,edx
	stos	dword [edi]
	mov	eax,[ebx+10h]
	stos	dword [edi]
	mov	eax,[ebx+14h]
	test	eax,eax
	jz	ta_base_symbol_for_line_ok
	cmp	eax,[ta_symbols_stream]
	mov	eax,[eax+4]
	jae	ta_base_symbol_for_line_ok
	xor	eax,eax
      ta_base_symbol_for_line_ok:
	stos	dword [edi]
	mov	al,[ebx+9]
	stos	byte [edi]
	mov	al,[esi+10h]
	stos	byte [edi]
	mov	al,[ebx+0Ah]
	and	al,1
	stos	byte [edi]
	mov	al,cl
	stos	byte [edi]
	pop	ebx
	cmp	edi,[ta_tagged_blocks]
	jae	ta_out_of_memory
	mov	eax,edi
	sub	eax,1Ch
	sub	eax,ebp
	mov	[esi],eax
	jmp	ta_make_lines_dump
      ta_lines_dump_ok:
	mov	edx,edi
	mov	eax,[ta_current_offset]
	sub	eax,[ta_code_start]
	add	eax,[ta_headers_size]
	stos	dword [edi]
	mov	ecx,edi
	sub	ecx,ebx
	sub	ecx,[ebx-40h+14h]
	mov	[ebx-40h+2Ch],ecx
	add	ecx,[ebx-40h+28h]
	mov	[ebx-40h+30h],ecx
	add	ecx,[ebx-40h+34h]
	mov	[ebx-40h+38h],ecx
      ta_find_inexisting_offsets:
	sub	edx,1Ch
	cmp	edx,ebp
	jb	ta_write_symbols
	test	byte [edx+1Ah],1
	jnz	ta_find_inexisting_offsets
	cmp	eax,[edx]
	jb	ta_correct_inexisting_offset
	mov	eax,[edx]
	jmp	ta_find_inexisting_offsets
      ta_correct_inexisting_offset:
	and	dword [edx],0
	or	byte [edx+1Ah],2
	jmp	ta_find_inexisting_offsets
      ta_write_symbols:
	mov	edx,[ta_symbols_file]
	call	ta_create
	jc	ta_write_failed
	mov	edx,[ta_code_start]
	mov	ecx,[edx+14h]
	add	ecx,40h
	call	ta_write
	jc	ta_write_failed
	mov	edx,[ta_tagged_blocks]
	mov	ecx,[ta_memory_end]
	sub	ecx,[ta_labels_list]
	call	ta_write
	jc	ta_write_failed
	mov	edx,[ta_memory_start]
	mov	ecx,[ta_source_start]
	sub	ecx,edx
	call	ta_write
	jc	ta_write_failed
	mov	edx,ebp
	mov	ecx,edi
	sub	ecx,edx
	call	ta_write
	jc	ta_write_failed
	mov	edx,[ta_free_additional_memory]
	mov	ecx,[ta_number_of_sections]
	shl	ecx,2
	call	ta_write
	jc	ta_write_failed
	mov	esi,[ta_labels_list]
	mov	edi,[ta_memory_start]
      ta_make_references_dump:
	cmp	esi,[ta_tagged_blocks]
	je	ta_references_dump_ok
	mov	eax,[esi-4]
	mov	ecx,[esi-8]
	sub	esi,8
	sub	esi,ecx
	cmp	eax,2
	je	ta_dump_reference
	cmp	eax,1
	jne	ta_make_references_dump
	mov	edx,[esi]
	jmp	ta_make_references_dump
      ta_dump_reference:
	mov	eax,[ta_memory_end]
	sub	eax,[esi]
	sub	eax,LABEL_STRUCTURE_SIZE
	stosd
	mov	eax,edx
	stosd
	cmp	edi,[ta_tagged_blocks]
	jb	ta_make_references_dump
	jmp	ta_out_of_memory
      ta_references_dump_ok:
	mov	edx,[ta_memory_start]
	mov	ecx,edi
	sub	ecx,edx
	call	ta_write
	jc	ta_write_failed
	call	ta_close
	ret
      ta_setup_dump_header:
	xor	eax,eax
	mov	ecx,40h shr 2
	rep	stos dword [edi]
	mov	ebx,edi
	mov	dword [ebx-40h],'fas'+1Ah shl 24
	mov	dword [ebx-40h+4],VERSION_MAJOR + VERSION_MINOR shl 8 + 40h shl 16
	mov	dword [ebx-40h+10h],40h
	ret
ta_prepare_preprocessed_source:
	mov	esi,[ta_memory_start]
	mov	ebp,[ta_source_start]
	test	ebp,ebp
	jnz	ta_prepare_preprocessed_line
	mov	ebp,[ta_current_line]
	inc	ebp
      ta_prepare_preprocessed_line:
	cmp	esi,ebp
	jae	ta_preprocessed_source_ok
	mov	eax,[ta_memory_start]
	mov	edx,[ta_input_file]
	cmp	[esi],edx
	jne	ta_line_not_from_main_input
	mov	[esi],eax
      ta_line_not_from_main_input:
	sub	[esi],eax
	test	byte [esi+7],1 shl 7
	jz	ta_prepare_next_preprocessed_line
	sub	[esi+8],eax
	sub	[esi+12],eax
      ta_prepare_next_preprocessed_line:
	call	ta_skip_preprocessed_line
	jmp	ta_prepare_preprocessed_line
      ta_preprocessed_source_ok:
	ret
      ta_skip_preprocessed_line:
	add	esi,16
      ta_skip_preprocessed_line_content:
	lods	byte [esi]
	cmp	al,1Ah
	je	ta_skip_preprocessed_symbol
	cmp	al,3Bh
	je	ta_skip_preprocessed_symbol
	cmp	al,22h
	je	ta_skip_preprocessed_string
	or	al,al
	jnz	ta_skip_preprocessed_line_content
	ret
      ta_skip_preprocessed_string:
	lods	dword [esi]
	add	esi,eax
	jmp	ta_skip_preprocessed_line_content
      ta_skip_preprocessed_symbol:
	lods	byte [esi]
	movzx	eax,al
	add	esi,eax
	jmp	ta_skip_preprocessed_line_content
ta_restore_preprocessed_source:
	mov	esi,[ta_memory_start]
	mov	ebp,[ta_source_start]
	test	ebp,ebp
	jnz	ta_restore_preprocessed_line
	mov	ebp,[ta_current_line]
	inc	ebp
      ta_restore_preprocessed_line:
	cmp	esi,ebp
	jae	ta_preprocessed_source_restored
	mov	eax,[ta_memory_start]
	add	[esi],eax
	cmp	[esi],eax
	jne	ta_preprocessed_line_source_restored
	mov	edx,[ta_input_file]
	mov	[esi],edx
      ta_preprocessed_line_source_restored:
	test	byte [esi+7],1 shl 7
	jz	ta_restore_next_preprocessed_line
	add	[esi+8],eax
	add	[esi+12],eax
      ta_restore_next_preprocessed_line:
	call	ta_skip_preprocessed_line
	jmp	ta_restore_preprocessed_line
      ta_preprocessed_source_restored:
	ret
ta_dump_preprocessed_source:
	mov	edi,[ta_free_additional_memory]
	call	ta_setup_dump_header
	mov	esi,[ta_input_file]
	call	ta_copy_asciiz
	cmp	edi,[ta_additional_memory_end]
	jae	ta_out_of_memory
	mov	eax,edi
	sub	eax,ebx
	dec	eax
	mov	[ebx-40h+0Ch],eax
	mov	eax,edi
	sub	eax,ebx
	mov	[ebx-40h+14h],eax
	add	eax,40h
	mov	[ebx-40h+20h],eax
	call	ta_prepare_preprocessed_source
	sub	esi,[ta_memory_start]
	mov	[ebx-40h+24h],esi
	mov	edx,[ta_symbols_file]
	call	ta_create
	jc	ta_write_failed
	mov	edx,[ta_free_additional_memory]
	mov	ecx,[edx+14h]
	add	ecx,40h
	call	ta_write
	jc	ta_write_failed
	mov	edx,[ta_memory_start]
	mov	ecx,esi
	call	ta_write
	jc	ta_write_failed
	call	ta_close
	ret
