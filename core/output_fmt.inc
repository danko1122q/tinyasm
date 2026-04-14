ta_formatter:
	mov	[ta_current_offset],edi
	cmp	[ta_output_file],0
	jne	ta_output_path_ok
	mov	esi,[ta_input_file]
	mov	edi,[ta_free_additional_memory]
      ta_duplicate_output_path:
	lods	byte [esi]
	cmp	edi,[ta_structures_buffer]
	jae	ta_out_of_memory
	stos	byte [edi]
	or	al,al
	jnz	ta_duplicate_output_path
	dec	edi
	mov	eax,edi
      ta_find_extension:
	dec	eax
	cmp	eax,[ta_free_additional_memory]
	jb	ta_extension_found
	cmp	byte [eax],'\'
	je	ta_extension_found
	cmp	byte [eax],'/'
	je	ta_extension_found
	cmp	byte [eax],'.'
	jne	ta_find_extension
	mov	edi,eax
      ta_extension_found:
	lea	eax,[edi+9]
	cmp	eax,[ta_structures_buffer]
	jae	ta_out_of_memory
	cmp	[ta_file_extension],0
	jne	ta_extension_specified
	mov	al,[ta_output_format]
	jb	ta_bin_extension
	cmp	al,4
	je	ta_obj_extension
	cmp	al,5
	je	ta_o_extension
	cmp	al,3
	jne	ta_no_extension
	cmp	[ta_subsystem],1
	je	ta_sys_extension
	cmp	[ta_subsystem],10
	jae	ta_efi_extension
	bt	[ta_format_flags],8
	jnc	ta_exe_extension
	mov	eax,'.dll'
	jmp	ta_make_extension
      ta_sys_extension:
	mov	eax,'.sys'
	jmp	ta_make_extension
      ta_efi_extension:
	mov	eax,'.efi'
	jmp	ta_make_extension
      ta_bin_extension:
	mov	eax,'.bin'
	bt	[ta_format_flags],0
	jnc	ta_make_extension
	mov	eax,'.com'
	jmp	ta_make_extension
      ta_obj_extension:
	mov	eax,'.obj'
	jmp	ta_make_extension
      ta_o_extension:
	mov	eax,'.o'
	bt	[ta_format_flags],0
	jnc	ta_make_extension
      ta_no_extension:
	xor	eax,eax
	jmp	ta_make_extension
      ta_exe_extension:
	mov	eax,'.exe'
      ta_make_extension:
	xchg	eax,[edi]
	scas	dword [edi]
	mov	byte [edi],0
	scas	byte [edi]
	mov	esi,edi
	stos	dword [edi]
	sub	edi,9
	xor	eax,eax
	mov	ebx,ta_characters
      ta_adapt_case:
	mov	al,[esi]
	or	al,al
	jz	ta_adapt_next
	xlat	byte [ebx]
	cmp	al,[esi]
	je	ta_adapt_ok
	sub	byte [edi],20h
      ta_adapt_ok:
	inc	esi
      ta_adapt_next:
	inc	edi
	cmp	byte [edi],0
	jne	ta_adapt_case
	jmp	ta_extension_ok
      ta_extension_specified:
	mov	al,'.'
	stos	byte [edi]
	mov	esi,[ta_file_extension]
      ta_copy_extension:
	lods	byte [esi]
	stos	byte [edi]
	test	al,al
	jnz	ta_copy_extension
	dec	edi
      ta_extension_ok:
	mov	esi,edi
	lea	ecx,[esi+1]
	sub	ecx,[ta_free_additional_memory]
	mov	edi,[ta_structures_buffer]
	dec	edi
	std
	rep	movs byte [edi],[esi]
	cld
	inc	edi
	mov	[ta_structures_buffer],edi
	mov	[ta_output_file],edi
      ta_output_path_ok:
	cmp	[ta_symbols_file],0
	je	ta_labels_table_ok
	mov	ecx,[ta_memory_end]
	sub	ecx,[ta_labels_list]
	mov	edi,[ta_tagged_blocks]
	sub	edi,8
	mov	[edi],ecx
	or	dword [edi+4],-1
	sub	edi,ecx
	cmp	edi,[ta_current_offset]
	jbe	ta_out_of_memory
	mov	[ta_tagged_blocks],edi
	mov	esi,[ta_memory_end]
      ta_copy_labels:
	sub	esi,32
	cmp	esi,[ta_labels_list]
	jb	ta_labels_table_ok
	mov	ecx,32 shr 2
	rep	movs dword [edi],[esi]
	sub	esi,32
	jmp	ta_copy_labels
      ta_labels_table_ok:
	mov	edi,[ta_current_offset]
	cmp	[ta_output_format],5
	jne	ta_common_formatter
	bt	[ta_format_flags],0
	jnc	ta_elf_formatter
      ta_common_formatter:
	mov	eax,edi
	sub	eax,[ta_code_start]
	mov	[ta_real_code_size],eax
	cmp	edi,[ta_undefined_data_end]
	jne	ta_calculate_code_size
	mov	edi,[ta_undefined_data_start]
      ta_calculate_code_size:
	mov	[ta_current_offset],edi
	sub	edi,[ta_code_start]
	mov	[ta_code_size],edi
	and	[ta_written_size],0
	mov	edx,[ta_output_file]
	call	ta_create
	jc	ta_write_failed
	cmp	[ta_output_format],3
	jne	ta_stub_written
	mov	edx,[ta_code_start]
	mov	ecx,[ta_stub_size]
	sub	edx,ecx
	add	[ta_written_size],ecx
	call	ta_write
      ta_stub_written:
      ta_write_output:
	call	ta_write_code
      ta_output_written:
	call	ta_close
	cmp	[ta_symbols_file],0
	jne	ta_dump_symbols
	ret
      ta_write_code:
	mov	eax,[ta_written_size]
	mov	[ta_headers_size],eax
	mov	edx,[ta_code_start]
	mov	ecx,[ta_code_size]
	add	[ta_written_size],ecx
	lea	eax,[edx+ecx]
	call	ta_write
	jc	ta_write_failed
	ret
ta_format_directive:
	cmp	edi,[ta_code_start]
	jne	ta_unexpected_instruction
	mov	ebp,[ta_addressing_space]
	test	byte [ds:ebp+0Ah],1
	jnz	ta_unexpected_instruction
	cmp	[ta_output_format],0
	jne	ta_unexpected_instruction
	lods	byte [esi]
	cmp	al,1Ch
	je	ta_format_prefix
	cmp	al,18h
	jne	ta_invalid_argument
	lods	byte [esi]
      ta_select_format:
	mov	dl,al
	shr	al,4
	mov	[ta_output_format],al
	and	edx,0Fh
	or	[ta_format_flags],edx
	cmp	al,2
	je	ta_illegal_instruction
	cmp	al,3
	je	ta_format_pe
	cmp	al,5
	je	ta_format_elf
      ta_format_defined:
	cmp	byte [esi],86h
	jne	ta_instruction_assembled
	cmp	word [esi+1],'('
	jne	ta_invalid_argument
	mov	eax,[esi+3]
	add	esi,3+4
	mov	[ta_file_extension],esi
	lea	esi,[esi+eax+1]
	jmp	ta_instruction_assembled
      ta_format_prefix:
	lods	byte [esi]
	mov	ah,al
	lods	byte [esi]
	cmp	al,18h
	jne	ta_invalid_argument
	lods	byte [esi]
	mov	edx,eax
	shr	dl,4
	shr	dh,4
	cmp	dl,dh
	jne	ta_invalid_argument
	or	al,ah
	jmp	ta_select_format
ta_entry_directive:
	bts	[ta_format_flags],10h
	jc	ta_setting_already_specified
	mov	al,[ta_output_format]
	cmp	al,3
	je	ta_pe_entry
	cmp	al,5
	jne	ta_illegal_instruction
	bt	[ta_format_flags],0
	jc	ta_elf_entry
	jmp	ta_illegal_instruction
ta_stack_directive:
	bts	[ta_format_flags],11h
	jc	ta_setting_already_specified
	mov	al,[ta_output_format]
	cmp	al,3
	je	ta_pe_stack
	jmp	ta_illegal_instruction
ta_heap_directive:
	bts	[ta_format_flags],12h
	jc	ta_setting_already_specified
	mov	al,[ta_output_format]
	cmp	al,3
	je	ta_pe_heap
	jmp	ta_illegal_instruction
ta_segment_directive:
	mov	al,[ta_output_format]
	cmp	al,5
	je	ta_elf_segment
	jmp	ta_illegal_instruction
ta_section_directive:
	mov	al,[ta_output_format]
	cmp	al,3
	je	ta_pe_section
	cmp	al,5
	je	ta_elf_section
	jmp	ta_illegal_instruction
ta_public_directive:
	mov	al,[ta_output_format]
	cmp	al,5
	jne	ta_illegal_instruction
	bt	[ta_format_flags],0
	jc	ta_illegal_instruction
      ta_public_allowed:
	mov	[ta_base_code],0C0h
	lods	byte [esi]
	cmp	al,2
	je	ta_public_label
	cmp	al,1Dh
	jne	ta_invalid_argument
	lods	byte [esi]
	and	al,7
	add	[ta_base_code],al
	lods	byte [esi]
	cmp	al,2
	jne	ta_invalid_argument
      ta_public_label:
	lods	dword [esi]
	cmp	eax,0Fh
	jb	ta_invalid_use_of_symbol
	je	ta_reserved_word_used_as_symbol
	inc	esi
	mov	dx,[ta_current_pass]
	mov	[eax+18],dx
	or	byte [eax+8],8
	mov	ebx,eax
	call	ta_store_label_reference
	mov	eax,ebx
	mov	ebx,[ta_free_additional_memory]
	lea	edx,[ebx+10h]
	cmp	edx,[ta_structures_buffer]
	jae	ta_out_of_memory
	mov	[ta_free_additional_memory],edx
	mov	[ebx+8],eax
	mov	eax,[ta_current_line]
	mov	[ebx+0Ch],eax
	lods	byte [esi]
	cmp	al,86h
	jne	ta_invalid_argument
	lods	word [esi]
	cmp	ax,'('
	jne	ta_invalid_argument
	mov	[ebx+4],esi
	lods	dword [esi]
	lea	esi,[esi+eax+1]
	mov	al,[ta_base_code]
	mov	[ebx],al
	jmp	ta_instruction_assembled
ta_extrn_directive:
	mov	al,[ta_output_format]
	cmp	al,5
	jne	ta_illegal_instruction
	bt	[ta_format_flags],0
	jc	ta_illegal_instruction
      ta_extrn_allowed:
	lods	word [esi]
	cmp	ax,'('
	jne	ta_invalid_argument
	mov	ebx,esi
	lods	dword [esi]
	lea	esi,[esi+eax+1]
	mov	edx,[ta_free_additional_memory]
	lea	eax,[edx+0Ch]
	cmp	eax,[ta_structures_buffer]
	jae	ta_out_of_memory
	mov	[ta_free_additional_memory],eax
	mov	byte [edx],80h
	mov	[edx+4],ebx
	lods	byte [esi]
	cmp	al,86h
	jne	ta_invalid_argument
	lods	byte [esi]
	cmp	al,2
	jne	ta_invalid_argument
	lods	dword [esi]
	cmp	eax,0Fh
	jb	ta_invalid_use_of_symbol
	je	ta_reserved_word_used_as_symbol
	inc	esi
	mov	ebx,eax
	xor	ah,ah
	lods	byte [esi]
	cmp	al,':'
	je	ta_get_extrn_size
	dec	esi
	cmp	al,11h
	jne	ta_extrn_size_ok
      ta_get_extrn_size:
	lods	word [esi]
	cmp	al,11h
	jne	ta_invalid_argument
      ta_extrn_size_ok:
	mov	[ta_address_symbol],edx
	mov	[ta_label_size],ah
	movzx	ecx,ah
	mov	[edx+8],ecx
	xor	eax,eax
	xor	edx,edx
	xor	ebp,ebp
	mov	[ta_address_sign],0
	mov	ch,2
	test	[ta_format_flags],8
	jz	ta_make_free_label
	mov	ch,4
	jmp	ta_make_free_label
ta_mark_relocation:
	cmp	[ta_value_type],0
	je	ta_relocation_ok
	mov	ebp,[ta_addressing_space]
	test	byte [ds:ebp+0Ah],1
	jnz	ta_relocation_ok
	cmp	[ta_output_format],3
	je	ta_mark_pe_relocation
	cmp	[ta_output_format],5
	je	ta_mark_elf_relocation
      ta_relocation_ok:
	ret
ta_close_pass:
	mov	al,[ta_output_format]
	cmp	al,3
	je	ta_close_pe
	cmp	al,5
	je	ta_close_elf
	ret

ta_recoverable_invalid_address:
	cmp	[ta_error_line],0
	jne	ta_ignore_invalid_address
	push	[ta_current_line]
	pop	[ta_error_line]
	mov	[ta_error],ta_invalid_address
      ta_ignore_invalid_address:
	ret

ta_make_stub:
	mov	[ta_stub_file],edx
	or	edx,edx
	jnz	ta_stub_from_file
	push	esi
	mov	edx,edi
	xor	eax,eax
	mov	ecx,20h
	rep	stos dword [edi]
	mov	eax,40h+ta_default_stub_end-ta_default_stub
	mov	cx,100h+ta_default_stub_end-ta_default_stub
	mov	word [edx],'MZ'
	mov	byte [edx+4],1
	mov	word [edx+2],ax
	mov	byte [edx+8],4
	mov	byte [edx+0Ah],10h
	mov	word [edx+0Ch],0FFFFh
	mov	word [edx+10h],cx
	mov	word [edx+3Ch],ax
	mov	byte [edx+18h],40h
	lea	edi,[edx+40h]
	mov	esi,ta_default_stub
	mov	ecx,ta_default_stub_end-ta_default_stub
	rep	movs byte [edi],[esi]
	pop	esi
	jmp	ta_stub_ok
      ta_default_stub:
	use16
	push	cs
	pop	ds
	mov	dx,stub_message-ta_default_stub
	mov	ah,9
	int	21h
	mov	ax,4C01h
	int	21h
      stub_message db 'This program cannot be run in DOS mode.',0Dh,0Ah,24h
	rq	1
      ta_default_stub_end:
	use32
      ta_stub_from_file:
	push	esi
	mov	esi,edx
	call	ta_open_binary_file
	mov	edx,edi
	mov	ecx,1Ch
	mov	esi,edx
	call	ta_read
	jc	ta_binary_stub
	cmp	word [esi],'MZ'
	jne	ta_binary_stub
	add	edi,1Ch
	movzx	ecx,word [esi+6]
	add	ecx,11b
	and	ecx,not 11b
	add	ecx,(40h-1Ch) shr 2
	lea	eax,[edi+ecx*4]
	cmp	edi,[ta_tagged_blocks]
	jae	ta_out_of_memory
	xor	eax,eax
	rep	stos dword [edi]
	mov	edx,40h
	xchg	dx,[esi+18h]
	xor	al,al
	call	ta_lseek
	movzx	ecx,word [esi+6]
	shl	ecx,2
	lea	edx,[esi+40h]
	call	ta_read
	mov	edx,edi
	sub	edx,esi
	shr	edx,4
	xchg	dx,[esi+8]
	shl	edx,4
	xor	al,al
	call	ta_lseek
	movzx	ecx,word [esi+4]
	dec	ecx
	js	ta_out_of_memory
	shl	ecx,9
	movzx	edx,word [esi+2]
	test	edx,edx
	jnz	ta_stub_header_size_ok
	mov	dx,200h
     ta_stub_header_size_ok:
	add	ecx,edx
	mov	edx,edi
	sub	ecx,eax
	je	ta_read_stub_code
	jb	ta_stub_code_ok
	push	ecx
	dec	ecx
	shr	ecx,3
	inc	ecx
	shl	ecx,1
	lea	eax,[edi+ecx*4]
	cmp	eax,[ta_tagged_blocks]
	jae	ta_out_of_memory
	xor	eax,eax
	rep	stos dword [edi]
	pop	ecx
     ta_read_stub_code:
	call	ta_read
     ta_stub_code_ok:
	call	ta_close
	mov	edx,edi
	sub	edx,esi
	mov	ax,dx
	and	ax,1FFh
	mov	[esi+2],ax
	dec	edx
	shr	edx,9
	inc	edx
	mov	[esi+4],dx
	mov	eax,edi
	sub	eax,esi
	mov	[esi+3Ch],eax
	pop	esi
      ta_stub_ok:
	ret
      ta_binary_stub:
	mov	esi,edi
	mov	ecx,40h shr 2
	xor	eax,eax
	rep	stos dword [edi]
	mov	al,2
	xor	edx,edx
	call	ta_lseek
	push	eax
	xor	al,al
	xor	edx,edx
	call	ta_lseek
	mov	ecx,[esp]
	add	ecx,40h+111b
	and	ecx,not 111b
	mov	ax,cx
	and	ax,1FFh
	mov	[esi+2],ax
	lea	eax,[ecx+1FFh]
	shr	eax,9
	mov	[esi+4],ax
	mov	[esi+3Ch],ecx
	sub	ecx,40h
	mov	eax,10000h
	sub	eax,ecx
	jbe	ta_binary_heap_ok
	shr	eax,4
	mov	[esi+0Ah],ax
      ta_binary_heap_ok:
	mov	word [esi],'MZ'
	mov	byte [esi+8],4
	mov	ax,0FFFFh
	mov	[esi+0Ch],ax
	dec	ax
	mov	[esi+10h],ax
	sub	ax,0Eh
	mov	[esi+0Eh],ax
	mov	[esi+16h],ax
	mov	word [esi+14h],100h
	mov	byte [esi+18h],40h
	mov	eax,[ta_tagged_blocks]
	sub	eax,ecx
	cmp	edi,eax
	jae	ta_out_of_memory
	mov	edx,edi
	shr	ecx,2
	xor	eax,eax
	rep	stos dword [edi]
	pop	ecx
	call	ta_read
	call	ta_close
	pop	esi
	ret

ta_format_pe:
	xor	edx,edx
	mov	[ta_machine],14Ch
	mov	[ta_subsystem],3
	mov	[ta_subsystem_version],3 + 10 shl 16
	mov	[ta_image_base],400000h
	and	[ta_image_base_high],0
	test	[ta_format_flags],8
	jz	ta_pe_settings
	mov	[ta_machine],8664h
	mov	[ta_subsystem_version],5 + 0 shl 16
      ta_pe_settings:
	cmp	byte [esi],84h
	je	ta_get_stub_name
	cmp	byte [esi],80h
	je	ta_get_pe_base
	cmp	byte [esi],1Bh
	jne	ta_pe_settings_ok
	lods	byte [esi]
	lods	byte [esi]
	test	al,80h+40h
	jz	ta_subsystem_setting
	cmp	al,80h
	je	ta_dll_flag
	cmp	al,81h
	je	ta_wdm_flag
	cmp	al,82h
	je	ta_large_flag
	cmp	al,83h
	je	ta_nx_flag
	jmp	ta_pe_settings
      ta_dll_flag:
	bts	[ta_format_flags],8
	jc	ta_setting_already_specified
	jmp	ta_pe_settings
      ta_wdm_flag:
	bts	[ta_format_flags],9
	jc	ta_setting_already_specified
	jmp	ta_pe_settings
      ta_large_flag:
	bts	[ta_format_flags],11
	jc	ta_setting_already_specified
	test	[ta_format_flags],8
	jnz	ta_invalid_argument
	jmp	ta_pe_settings
      ta_nx_flag:
	bts	[ta_format_flags],12
	jc	ta_setting_already_specified
	jmp	ta_pe_settings
      ta_subsystem_setting:
	bts	[ta_format_flags],7
	jc	ta_setting_already_specified
	and	ax,3Fh
	mov	[ta_subsystem],ax
	cmp	ax,10
	jb	ta_subsystem_type_ok
	or	[ta_format_flags],4
      ta_subsystem_type_ok:
	cmp	byte [esi],'('
	jne	ta_pe_settings
	inc	esi
	cmp	byte [esi],'.'
	jne	ta_invalid_value
	inc	esi
	push	edx
	cmp	byte [esi+11],0
	jne	ta_invalid_value
	cmp	byte [esi+10],2
	ja	ta_invalid_value
	mov	dx,[esi+8]
	cmp	dx,8000h
	je	ta_zero_version
	mov	eax,[esi+4]
	cmp	dx,7
	jg	ta_invalid_value
	mov	cx,7
	sub	cx,dx
	mov	eax,[esi+4]
	shr	eax,cl
	mov	ebx,eax
	shr	ebx,24
	cmp	bl,100
	jae	ta_invalid_value
	and	eax,0FFFFFFh
	mov	ecx,100
	mul	ecx
	shrd	eax,edx,24
	jnc	ta_version_value_ok
	inc	eax
      ta_version_value_ok:
	shl	eax,16
	mov	ax,bx
	jmp	ta_subsystem_version_ok
      ta_zero_version:
	xor	eax,eax
      ta_subsystem_version_ok:
	pop	edx
	add	esi,13
	mov	[ta_subsystem_version],eax
	jmp	ta_pe_settings
      ta_get_pe_base:
	bts	[ta_format_flags],10
	jc	ta_setting_already_specified
	lods	word [esi]
	cmp	ah,'('
	jne	ta_invalid_argument
	cmp	byte [esi],'.'
	je	ta_invalid_value
	push	edx edi
	add	edi,[ta_stub_size]
	test	[ta_format_flags],4
	jnz	ta_get_peplus_base
	call	ta_get_dword_value
	mov	[ta_image_base],eax
	jmp	ta_pe_base_ok
      ta_get_peplus_base:
	call	ta_get_qword_value
	mov	[ta_image_base],eax
	mov	[ta_image_base_high],edx
      ta_pe_base_ok:
	pop	edi edx
	cmp	[ta_value_type],0
	jne	ta_invalid_use_of_symbol
	cmp	byte [esi],84h
	jne	ta_pe_settings_ok
      ta_get_stub_name:
	lods	byte [esi]
	lods	word [esi]
	cmp	ax,'('
	jne	ta_invalid_argument
	lods	dword [esi]
	mov	edx,esi
	add	esi,eax
	inc	esi
      ta_pe_settings_ok:
	mov	ebp,[ta_stub_size]
	or	ebp,ebp
	jz	ta_make_pe_stub
	cmp	edx,[ta_stub_file]
	je	ta_pe_stub_ok
	sub	edi,[ta_stub_size]
	mov	[ta_code_start],edi
      ta_make_pe_stub:
	call	ta_make_stub
	mov	eax,edi
	sub	eax,[ta_code_start]
	mov	[ta_stub_size],eax
	mov	[ta_code_start],edi
	mov	ebp,eax
      ta_pe_stub_ok:
	mov	edx,edi
	mov	ecx,18h+0E0h
	test	[ta_format_flags],4
	jz	ta_zero_pe_header
	add	ecx,10h
      ta_zero_pe_header:
	add	ebp,ecx
	shr	ecx,2
	xor	eax,eax
	rep	stos dword [edi]
	mov	word [edx],'PE' 	; signature
	mov	ax,[ta_machine]
	mov	word [edx+4],ax
	mov	byte [edx+38h+1],10h	; section alignment
	mov	byte [edx+3Ch+1],2	; file alignment
	mov	byte [edx+40h],1	; OS version
	mov	eax,[ta_subsystem_version]
	mov	[edx+48h],eax
	mov	ax,[ta_subsystem]
	mov	[edx+5Ch],ax
	cmp	ax,1
	jne	ta_pe_alignment_ok
	mov	eax,20h
	mov	dword [edx+38h],eax
	mov	dword [edx+3Ch],eax
      ta_pe_alignment_ok:
	mov	word [edx+1Ah],VERSION_MAJOR + VERSION_MINOR shl 8
	test	[ta_format_flags],4
	jnz	ta_init_peplus_specific
	mov	byte [edx+14h],0E0h	; size of optional header
	mov	dword [edx+16h],10B010Fh; flags and magic ta_value
	mov	eax,[ta_image_base]
	mov	[edx+34h],eax
	mov	byte [edx+60h+1],10h	; stack reserve
	mov	byte [edx+64h+1],10h	; stack commit
	mov	byte [edx+68h+2],1	; heap reserve
	mov	byte [edx+74h],16	; number of directories
	jmp	ta_pe_header_ok
      ta_init_peplus_specific:
	mov	byte [edx+14h],0F0h	; size of optional header
	mov	dword [edx+16h],20B002Fh; flags and magic ta_value
	mov	eax,[ta_image_base]
	mov	[edx+30h],eax
	mov	eax,[ta_image_base_high]
	mov	[edx+34h],eax
	mov	byte [edx+60h+1],10h	; stack reserve
	mov	byte [edx+68h+1],10h	; stack commit
	mov	byte [edx+70h+2],1	; heap reserve
	mov	byte [edx+84h],16	; number of directories
      ta_pe_header_ok:
	bsf	ecx,[edx+3Ch]
	imul	ebx,[ta_number_of_sections],28h
	or	ebx,ebx
	jnz	ta_reserve_space_for_section_headers
	mov	ebx,28h
      ta_reserve_space_for_section_headers:
	add	ebx,ebp
	dec	ebx
	shr	ebx,cl
	inc	ebx
	shl	ebx,cl
	sub	ebx,ebp
	mov	ecx,ebx
	mov	eax,[ta_tagged_blocks]
	sub	eax,ecx
	cmp	edi,eax
	jae	ta_out_of_memory
	shr	ecx,2
	xor	eax,eax
	rep	stos dword [edi]
	mov	eax,edi
	sub	eax,[ta_code_start]
	add	eax,[ta_stub_size]
	mov	[edx+54h],eax		; size of headers
	mov	ecx,[edx+38h]
	dec	ecx
	add	eax,ecx
	not	ecx
	and	eax,ecx
	bt	[ta_format_flags],8
	jc	ta_pe_entry_init_ok
	mov	[edx+28h],eax		; entry point rva
      ta_pe_entry_init_ok:
	and	[ta_number_of_sections],0
	movzx	ebx,word [edx+14h]
	lea	ebx,[edx+18h+ebx]
	mov	[ta_current_section],ebx
	mov	dword [ebx],'.fla'
	mov	dword [ebx+4],'t'
	mov	[ebx+14h],edi
	mov	[ebx+0Ch],eax
	mov	dword [ebx+24h],0E0000060h
	xor	ecx,ecx
	xor	bl,bl
	not	eax
	not	ecx
	not	bl
	add	eax,1
	adc	ecx,0
	adc	bl,0
	add	eax,edi
	adc	ecx,0
	adc	bl,0
	test	[ta_format_flags],4
	jnz	ta_peplus_org
	sub	eax,[edx+34h]
	sbb	ecx,0
	sbb	bl,0
	jmp	ta_pe_org_ok
      ta_peplus_org:
	sub	eax,[edx+30h]
	sbb	ecx,[edx+34h]
	sbb	bl,0
      ta_pe_org_ok:
	test	[ta_format_flags],8
	jnz	ta_pe64_code
	mov	bh,2
	mov	[ta_code_type],32
	jmp	ta_pe_code_type_ok
      ta_pe64_code:
	mov	bh,4
	mov	[ta_code_type],64
      ta_pe_code_type_ok:
	bt	[ta_resolver_flags],0
	jc	ta_pe_labels_type_ok
	xor	bh,bh
      ta_pe_labels_type_ok:
	push	eax ebx
	call	ta_init_addressing_space
	mov	ebp,ebx
	pop	ebx eax
	mov	[ds:ebp],eax
	mov	[ds:ebp+4],ecx
	mov	[ds:ebp+8],bx
	mov	[ds:ebp+18h],edi
	bt	[ta_format_flags],8
	jnc	ta_dll_flag_ok
	or	byte [edx+16h+1],20h
      ta_dll_flag_ok:
	bt	[ta_format_flags],9
	jnc	ta_wdm_flag_ok
	or	byte [edx+5Eh+1],20h
      ta_wdm_flag_ok:
	bt	[ta_format_flags],11
	jnc	ta_large_flag_ok
	or	byte [edx+16h],20h
      ta_large_flag_ok:
	bt	[ta_format_flags],12
	jnc	ta_nx_ok
	or	byte [edx+5Eh+1],1
      ta_nx_ok:
	jmp	ta_format_defined
ta_pe_section:
	call	ta_close_pe_section
	push	eax ebx
	call	ta_create_addressing_space
	mov	ebp,ebx
	pop	ebx eax
	bts	[ta_format_flags],5
	lea	ecx,[ebx+28h]
	add	edx,[edx+54h]
	sub	edx,[ta_stub_size]
	cmp	ecx,edx
	jbe	ta_new_section
	lea	ebx,[edx-28h]
	or	[ta_next_pass_needed],-1
	push	edi
	mov	edi,ebx
	mov	ecx,28h shr 4
	xor	eax,eax
	rep	stos dword [edi]
	pop	edi
      ta_new_section:
	mov	[ebx+0Ch],eax
	lods	word [esi]
	cmp	ax,'('
	jne	ta_invalid_argument
	lea	edx,[esi+4]
	mov	ecx,[esi]
	lea	esi,[esi+4+ecx+1]
	cmp	ecx,8
	ja	ta_name_too_long
	xor	eax,eax
	mov	[ebx],eax
	mov	[ebx+4],eax
	push	esi edi
	mov	edi,ebx
	mov	esi,edx
	rep	movs byte [edi],[esi]
	pop	edi esi
	and	dword [ebx+24h],0
	mov	[ebx+14h],edi
	mov	edx,[ta_code_start]
	mov	eax,edi
	xor	ecx,ecx
	sub	eax,[ebx+0Ch]
	sbb	ecx,0
	sbb	byte [ds:ebp+8],0
	mov	byte [ds:ebp+9],2
	mov	[ta_code_type],32
	test	[ta_format_flags],8
	jz	ta_pe_section_code_type_ok
	mov	byte [ds:ebp+9],4
	mov	[ta_code_type],64
      ta_pe_section_code_type_ok:
	test	[ta_format_flags],4
	jnz	ta_peplus_section_org
	sub	eax,[edx+34h]
	sbb	ecx,0
	sbb	byte [ds:ebp+8],0
	bt	[ta_resolver_flags],0
	jc	ta_pe_section_org_ok
	mov	byte [ds:ebp+9],0
	jmp	ta_pe_section_org_ok
      ta_peplus_section_org:
	sub	eax,[edx+30h]
	sbb	ecx,[edx+34h]
	sbb	byte [ds:ebp+8],0
	bt	[ta_resolver_flags],0
	jc	ta_pe_section_org_ok
	mov	byte [ds:ebp+9],0
      ta_pe_section_org_ok:
	mov	[ds:ebp],eax
	mov	[ds:ebp+4],ecx
	mov	[ds:ebp+18h],edi
      ta_get_section_flags:
	lods	byte [esi]
	cmp	al,1Ah
	je	ta_set_directory
	cmp	al,19h
	je	ta_section_flag
	dec	esi
	jmp	ta_instruction_assembled
      ta_set_directory:
	movzx	eax,byte [esi]
	inc	esi
	mov	ecx,ebx
	test	[ta_format_flags],4
	jnz	ta_peplus_directory
	xchg	ecx,[edx+78h+eax*8]
	mov	dword [edx+78h+eax*8+4],-1
	jmp	ta_pe_directory_set
      ta_peplus_directory:
	xchg	ecx,[edx+88h+eax*8]
	mov	dword [edx+88h+eax*8+4],-1
      ta_pe_directory_set:
	or	ecx,ecx
	jnz	ta_data_already_defined
	push	ebx edx
	call	ta_generate_pe_data
	pop	edx ebx
	jmp	ta_get_section_flags
      ta_section_flag:
	lods	byte [esi]
	cmp	al,9
	je	ta_invalid_argument
	cmp	al,11
	je	ta_invalid_argument
	mov	cl,al
	mov	eax,1
	shl	eax,cl
	test	dword [ebx+24h],eax
	jnz	ta_setting_already_specified
	or	dword [ebx+24h],eax
	jmp	ta_get_section_flags
      ta_close_pe_section:
	mov	ebx,[ta_current_section]
	mov	edx,[ta_code_start]
	mov	eax,edi
	sub	eax,[ebx+14h]
	jnz	ta_finish_section
	bt	[ta_format_flags],5
	jc	ta_finish_section
	mov	eax,[ebx+0Ch]
	ret
      ta_finish_section:
	mov	[ebx+8],eax
	cmp	edi,[ta_undefined_data_end]
	jne	ta_align_section
	cmp	dword [edx+38h],1000h
	jb	ta_align_section
	mov	edi,[ta_undefined_data_start]
      ta_align_section:
	and	[ta_undefined_data_end],0
	mov	ebp,edi
	sub	ebp,[ebx+14h]
	mov	ecx,[edx+3Ch]
	dec	ecx
	lea	eax,[ebp+ecx]
	not	ecx
	and	eax,ecx
	mov	[ebx+10h],eax
	sub	eax,ebp
	mov	ecx,eax
	xor	al,al
	rep	stos byte [edi]
	mov	eax,[ta_code_start]
	sub	eax,[ta_stub_size]
	sub	[ebx+14h],eax
	mov	ecx,[ebx+10h]
	test	byte [ebx+24h],20h
	jz	ta_pe_code_sum_ok
	add	[edx+1Ch],ecx
	cmp	dword [edx+2Ch],0
	jne	ta_pe_code_sum_ok
	mov	eax,[ebx+0Ch]
	mov	[edx+2Ch],eax
      ta_pe_code_sum_ok:
	test	byte [ebx+24h],40h
	jz	ta_pe_data_sum_ok
	add	[edx+20h],ecx
	test	[ta_format_flags],4
	jnz	ta_pe_data_sum_ok
	cmp	dword [edx+30h],0
	jne	ta_pe_data_sum_ok
	mov	eax,[ebx+0Ch]
	mov	[edx+30h],eax
      ta_pe_data_sum_ok:
	mov	eax,[ebx+8]
	or	eax,eax
	jz	ta_udata_ok
	cmp	dword [ebx+10h],0
	jne	ta_udata_ok
	or	byte [ebx+24h],80h
	add	[edx+24h],ecx
      ta_udata_ok:
	mov	ecx,[edx+38h]
	dec	ecx
	add	eax,ecx
	not	ecx
	and	eax,ecx
	add	eax,[ebx+0Ch]
	add	ebx,28h
	mov	[ta_current_section],ebx
	inc	word [ta_number_of_sections]
	jz	ta_format_limitations_exceeded
	ret
ta_data_directive:
	cmp	[ta_output_format],3
	jne	ta_illegal_instruction
	lods	byte [esi]
	cmp	al,1Ah
	je	ta_predefined_data_type
	cmp	al,'('
	jne	ta_invalid_argument
	call	ta_get_byte_value
	cmp	al,16
	jb	ta_data_type_ok
	jmp	ta_invalid_value
      ta_predefined_data_type:
	movzx	eax,byte [esi]
	inc	esi
      ta_data_type_ok:
	mov	ebx,[ta_current_section]
	mov	ecx,edi
	sub	ecx,[ebx+14h]
	add	ecx,[ebx+0Ch]
	mov	edx,[ta_code_start]
	test	[ta_format_flags],4
	jnz	ta_peplus_data
	xchg	ecx,[edx+78h+eax*8]
	jmp	ta_init_pe_data
      ta_peplus_data:
	xchg	ecx,[edx+88h+eax*8]
      ta_init_pe_data:
	or	ecx,ecx
	jnz	ta_data_already_defined
	call	ta_allocate_structure_data
	mov	word [ebx],ta_data_directive-ta_instruction_handler
	mov	[ebx+2],al
	mov	edx,[ta_current_line]
	mov	[ebx+4],edx
	call	ta_generate_pe_data
	jmp	ta_instruction_assembled
      ta_end_data:
	cmp	[ta_output_format],3
	jne	ta_illegal_instruction
	call	ta_find_structure_data
	jc	ta_unexpected_instruction
	movzx	eax,byte [ebx+2]
	mov	edx,[ta_current_section]
	mov	ecx,edi
	sub	ecx,[edx+14h]
	add	ecx,[edx+0Ch]
	mov	edx,[ta_code_start]
	test	[ta_format_flags],4
	jnz	ta_end_peplus_data
	sub	ecx,[edx+78h+eax*8]
	mov	[edx+78h+eax*8+4],ecx
	jmp	ta_remove_structure_data
      ta_end_peplus_data:
	sub	ecx,[edx+88h+eax*8]
	mov	[edx+88h+eax*8+4],ecx
	jmp	ta_remove_structure_data
ta_pe_entry:
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_argument
	cmp	byte [esi],'.'
	je	ta_invalid_value
	test	[ta_format_flags],8
	jnz	ta_pe64_entry
	call	ta_get_dword_value
	mov	bl,2
	bt	[ta_resolver_flags],0
	jc	ta_check_pe_entry_label_type
	xor	bl,bl
      ta_check_pe_entry_label_type:
	cmp	[ta_value_type],bl
	je	ta_pe_entry_ok
	call	ta_recoverable_invalid_address
      ta_pe_entry_ok:
	cdq
	test	[ta_format_flags],4
	jnz	ta_pe64_entry_type_ok
	mov	edx,[ta_code_start]
	sub	eax,[edx+34h]
	mov	[edx+28h],eax
	jmp	ta_instruction_assembled
      ta_pe64_entry:
	call	ta_get_qword_value
	mov	bl,4
	bt	[ta_resolver_flags],0
	jc	ta_check_pe64_entry_label_type
	xor	bl,bl
      ta_check_pe64_entry_label_type:
	cmp	[ta_value_type],bl
	je	ta_pe64_entry_type_ok
	call	ta_recoverable_invalid_address
      ta_pe64_entry_type_ok:
	mov	ecx,[ta_code_start]
	sub	eax,[ecx+30h]
	sbb	edx,[ecx+34h]
	jz	ta_pe64_entry_range_ok
	call	ta_recoverable_overflow
      ta_pe64_entry_range_ok:
	mov	[ecx+28h],eax
	jmp	ta_instruction_assembled
ta_pe_stack:
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_argument
	cmp	byte [esi],'.'
	je	ta_invalid_value
	test	[ta_format_flags],4
	jnz	ta_peplus_stack
	call	ta_get_count_value
	mov	edx,[ta_code_start]
	mov	[edx+60h],eax
	cmp	byte [esi],','
	jne	ta_default_stack_commit
	lods	byte [esi]
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_argument
	cmp	byte [esi],'.'
	je	ta_invalid_value
	call	ta_get_count_value
	mov	edx,[ta_code_start]
	mov	[edx+64h],eax
	cmp	eax,[edx+60h]
	ja	ta_value_out_of_range
	jmp	ta_instruction_assembled
      ta_default_stack_commit:
	mov	dword [edx+64h],1000h
	mov	eax,[edx+60h]
	cmp	eax,1000h
	ja	ta_instruction_assembled
	mov	dword [edx+64h],eax
	jmp	ta_instruction_assembled
      ta_peplus_stack:
	call	ta_get_qword_value
	cmp	[ta_value_type],0
	jne	ta_invalid_use_of_symbol
	mov	ecx,[ta_code_start]
	mov	[ecx+60h],eax
	mov	[ecx+64h],edx
	cmp	byte [esi],','
	jne	ta_default_peplus_stack_commit
	lods	byte [esi]
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_argument
	cmp	byte [esi],'.'
	je	ta_invalid_value
	call	ta_get_qword_value
	cmp	[ta_value_type],0
	jne	ta_invalid_use_of_symbol
	mov	ecx,[ta_code_start]
	mov	[ecx+68h],eax
	mov	[ecx+6Ch],edx
	cmp	edx,[ecx+64h]
	ja	ta_value_out_of_range
	jb	ta_instruction_assembled
	cmp	eax,[ecx+60h]
	ja	ta_value_out_of_range
	jmp	ta_instruction_assembled
      ta_default_peplus_stack_commit:
	mov	dword [ecx+68h],1000h
	cmp	dword [ecx+64h],0
	jne	ta_instruction_assembled
	mov	eax,[ecx+60h]
	cmp	eax,1000h
	ja	ta_instruction_assembled
	mov	dword [ecx+68h],eax
	jmp	ta_instruction_assembled
ta_pe_heap:
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_argument
	cmp	byte [esi],'.'
	je	ta_invalid_value
	test	[ta_format_flags],4
	jnz	ta_peplus_heap
	call	ta_get_count_value
	mov	edx,[ta_code_start]
	mov	[edx+68h],eax
	cmp	byte [esi],','
	jne	ta_instruction_assembled
	lods	byte [esi]
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_argument
	cmp	byte [esi],'.'
	je	ta_invalid_value
	call	ta_get_count_value
	mov	edx,[ta_code_start]
	mov	[edx+6Ch],eax
	cmp	eax,[edx+68h]
	ja	ta_value_out_of_range
	jmp	ta_instruction_assembled
      ta_peplus_heap:
	call	ta_get_qword_value
	cmp	[ta_value_type],0
	jne	ta_invalid_use_of_symbol
	mov	ecx,[ta_code_start]
	mov	[ecx+70h],eax
	mov	[ecx+74h],edx
	cmp	byte [esi],','
	jne	ta_instruction_assembled
	lods	byte [esi]
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_argument
	cmp	byte [esi],'.'
	je	ta_invalid_value
	call	ta_get_qword_value
	cmp	[ta_value_type],0
	jne	ta_invalid_use_of_symbol
	mov	ecx,[ta_code_start]
	mov	[ecx+78h],eax
	mov	[ecx+7Ch],edx
	cmp	edx,[ecx+74h]
	ja	ta_value_out_of_range
	jb	ta_instruction_assembled
	cmp	eax,[ecx+70h]
	ja	ta_value_out_of_range
	jmp	ta_instruction_assembled
ta_mark_pe_relocation:
	push	eax ebx
	test	[ta_format_flags],4
	jz	ta_check_standard_pe_relocation_type
	cmp	[ta_value_type],4
	je	ta_pe_relocation_type_ok
      ta_check_standard_pe_relocation_type:
	cmp	[ta_value_type],2
	je	ta_pe_relocation_type_ok
	call	ta_recoverable_misuse
      ta_pe_relocation_type_ok:
	mov	ebx,[ta_current_section]
	mov	eax,edi
	sub	eax,[ebx+14h]
	add	eax,[ebx+0Ch]
	mov	ebx,[ta_free_additional_memory]
	inc	[ta_number_of_relocations]
	add	ebx,5
	cmp	ebx,[ta_structures_buffer]
	jae	ta_out_of_memory
	mov	[ta_free_additional_memory],ebx
	mov	[ebx-5],eax
	cmp	[ta_value_type],2
	je	ta_fixup_32bit
	mov	byte [ebx-1],0Ah
	jmp	ta_fixup_ok
      ta_fixup_32bit:
	mov	byte [ebx-1],3
      ta_fixup_ok:
	pop	ebx eax
	ret
ta_generate_pe_data:
	cmp	al,2
	je	ta_make_pe_resource
	cmp	al,5
	je	ta_make_pe_fixups
	ret
ta_make_pe_fixups:
	mov	edx,[ta_code_start]
	and	byte [edx+16h],not 1
	or	byte [edx+5Eh],40h
	bts	[ta_resolver_flags],0
	jc	ta_fixups_ready
	or	[ta_next_pass_needed],-1
      ta_fixups_ready:
	and	[ta_last_fixup_base],0
	call	ta_make_fixups
	xchg	eax,[ta_actual_fixups_size]
	sub	eax,[ta_actual_fixups_size]
	ja	ta_reserve_forward_fixups
	xor	eax,eax
      ta_reserve_forward_fixups:
	mov	[ta_reserved_fixups],edi
	add	edi,eax
	mov	[ta_reserved_fixups_size],eax
	ret
      ta_make_fixups:
	push	esi
	xor	ecx,ecx
	xchg	ecx,[ta_number_of_relocations]
	mov	esi,[ta_free_additional_memory]
	lea	eax,[ecx*5]
	sub	esi,eax
	mov	[ta_free_additional_memory],esi
	mov	edx,[ta_last_fixup_base]
	mov	ebx,[ta_last_fixup_header]
	mov	ebp,edi
	jecxz	ta_fixups_done
      ta_make_fixup:
	cmp	[esi],edx
	jb	ta_store_fixup
	mov	eax,edi
	sub	eax,ebp
	test	eax,11b
	jz	ta_fixups_block
	xor	ax,ax
	stos	word [edi]
	add	dword [ebx],2
      ta_fixups_block:
	mov	eax,edx
	add	edx,1000h
	cmp	[esi],edx
	jae	ta_fixups_block
	stos	dword [edi]
	mov	ebx,edi
	mov	eax,8
	stos	dword [edi]
      ta_store_fixup:
	add	dword [ebx],2
	mov	ah,[esi+1]
	and	ah,0Fh
	mov	al,[esi+4]
	shl	al,4
	or	ah,al
	mov	al,[esi]
	stos	word [edi]
	add	esi,5
	loop	ta_make_fixup
      ta_fixups_done:
	mov	[ta_last_fixup_base],edx
	mov	[ta_last_fixup_header],ebx
	pop	esi
	mov	eax,edi
	sub	eax,ebp
	ret
ta_make_pe_resource:
	cmp	byte [esi],82h
	jne	ta_resource_done
	inc	esi
	lods	word [esi]
	cmp	ax,'('
	jne	ta_invalid_argument
	lods	dword [esi]
	mov	edx,esi
	lea	esi,[esi+eax+1]
	cmp	[ta_next_pass_needed],0
	je	ta_resource_from_file
	cmp	[ta_current_pass],0
	jne	ta_reserve_space_for_resource
	and	[ta_resource_size],0
      ta_reserve_space_for_resource:
	add	edi,[ta_resource_size]
	cmp	edi,[ta_tagged_blocks]
	ja	ta_out_of_memory
	jmp	ta_resource_done
      ta_resource_from_file:
	push	esi
	mov	esi,edx
	call	ta_open_binary_file
	push	ebx
	mov	esi,[ta_free_additional_memory]
	lea	eax,[esi+20h]
	cmp	eax,[ta_structures_buffer]
	ja	ta_out_of_memory
	mov	edx,esi
	mov	ecx,20h
	call	ta_read
	jc	ta_invalid_file_format
	xor	eax,eax
	cmp	[esi],eax
	jne	ta_invalid_file_format
	mov	ax,0FFFFh
	cmp	[esi+8],eax
	jne	ta_invalid_file_format
	cmp	[esi+12],eax
	jne	ta_invalid_file_format
	mov	eax,20h
	cmp	[esi+4],eax
	jne	ta_invalid_file_format
      ta_read_resource_headers:
	test	eax,11b
	jz	ta_resource_file_alignment_ok
	mov	edx,4
	and	eax,11b
	sub	edx,eax
	mov	al,1
	call	ta_lseek
	jc	ta_resource_headers_ok
      ta_resource_file_alignment_ok:
	mov	[esi],eax
	lea	edx,[esi+12]
	mov	ecx,8
	call	ta_read
	jc	ta_resource_headers_ok
	mov	ecx,[esi+16]
	add	[esi],ecx
	lea	edx,[esi+20]
	sub	ecx,8
	mov	[esi+16],ecx
	lea	eax,[edx+ecx]
	cmp	eax,[ta_structures_buffer]
	ja	ta_out_of_memory
	call	ta_read
	jc	ta_invalid_file_format
	mov	edx,[esi]
	add	edx,[esi+12]
	mov	eax,[esi+16]
	lea	ecx,[esi+20]
	lea	esi,[ecx+eax]
	add	ecx,2
	cmp	word [ecx-2],0FFFFh
	je	ta_resource_header_type_ok
      ta_check_resource_header_type:
	cmp	ecx,esi
	jae	ta_invalid_file_format
	cmp	word [ecx],0
	je	ta_resource_header_type_ok
	add	ecx,2
	jmp	ta_check_resource_header_type
      ta_resource_header_type_ok:
	add	ecx,2
	cmp	word [ecx],0FFFFh
	je	ta_resource_header_name_ok
      ta_check_resource_header_name:
	cmp	ecx,esi
	jae	ta_invalid_file_format
	cmp	word [ecx],0
	je	ta_resource_header_name_ok
	add	ecx,2
	jmp	ta_check_resource_header_name
      ta_resource_header_name_ok:
	xor	al,al
	call	ta_lseek
	jnc	ta_read_resource_headers
      ta_resource_headers_ok:
	cmp	esi,[ta_free_additional_memory]
	je	ta_invalid_file_format
	xor	eax,eax
	mov	[esi],eax
	mov	[ta_resource_data],edi
	lea	eax,[edi+16]
	cmp	eax,[ta_tagged_blocks]
	jae	ta_out_of_memory
	xor	eax,eax
	stos	dword [edi]
	call	ta_make_timestamp
	stos	dword [edi]
	xor	eax,eax
	stos	dword [edi]
	stos	dword [edi]
	xor	ebx,ebx
      ta_make_type_name_directory:
	mov	esi,[ta_free_additional_memory]
	xor	edx,edx
      ta_find_type_name:
	cmp	dword [esi],0
	je	ta_type_name_ok
	add	esi,20
	cmp	word [esi],0FFFFh
	je	ta_check_next_type_name
	or	ebx,ebx
	jz	ta_check_this_type_name
	xor	ecx,ecx
      ta_compare_with_previous_type_name:
	mov	ax,[esi+ecx]
	cmp	ax,[ebx+ecx]
	ja	ta_check_this_type_name
	jb	ta_check_next_type_name
	add	ecx,2
	mov	ax,[esi+ecx]
	or	ax,[ebx+ecx]
	jnz	ta_compare_with_previous_type_name
	jmp	ta_check_next_type_name
      ta_check_this_type_name:
	or	edx,edx
	jz	ta_type_name_found
	xor	ecx,ecx
      ta_compare_with_current_type_name:
	mov	ax,[esi+ecx]
	cmp	ax,[edx+ecx]
	ja	ta_check_next_type_name
	jb	ta_type_name_found
	add	ecx,2
	mov	ax,[esi+ecx]
	or	ax,[edx+ecx]
	jnz	ta_compare_with_current_type_name
	jmp	ta_same_type_name
      ta_type_name_found:
	mov	edx,esi
      ta_same_type_name:
	mov	[esi-16],edi
      ta_check_next_type_name:
	mov	eax,[esi-4]
	add	esi,eax
	jmp	ta_find_type_name
      ta_type_name_ok:
	or	edx,edx
	jz	ta_type_name_directory_done
	mov	ebx,edx
      ta_make_type_name_entry:
	mov	eax,[ta_resource_data]
	inc	word [eax+12]
	lea	eax,[edi+8]
	cmp	eax,[ta_tagged_blocks]
	jae	ta_out_of_memory
	mov	eax,ebx
	stos	dword [edi]
	xor	eax,eax
	stos	dword [edi]
	jmp	ta_make_type_name_directory
      ta_type_name_directory_done:
	mov	ebx,-1
      ta_make_type_id_directory:
	mov	esi,[ta_free_additional_memory]
	mov	edx,10000h
      ta_find_type_id:
	cmp	dword [esi],0
	je	ta_type_id_ok
	add	esi,20
	cmp	word [esi],0FFFFh
	jne	ta_check_next_type_id
	movzx	eax,word [esi+2]
	cmp	eax,ebx
	jle	ta_check_next_type_id
	cmp	eax,edx
	jg	ta_check_next_type_id
	mov	edx,eax
	mov	[esi-16],edi
      ta_check_next_type_id:
	mov	eax,[esi-4]
	add	esi,eax
	jmp	ta_find_type_id
      ta_type_id_ok:
	cmp	edx,10000h
	je	ta_type_id_directory_done
	mov	ebx,edx
      ta_make_type_id_entry:
	mov	eax,[ta_resource_data]
	inc	word [eax+14]
	lea	eax,[edi+8]
	cmp	eax,[ta_tagged_blocks]
	jae	ta_out_of_memory
	mov	eax,ebx
	stos	dword [edi]
	xor	eax,eax
	stos	dword [edi]
	jmp	ta_make_type_id_directory
      ta_type_id_directory_done:
	mov	esi,[ta_resource_data]
	add	esi,10h
	mov	ecx,[esi-4]
	or	cx,cx
	jz	ta_resource_directories_ok
      ta_make_resource_directories:
	push	ecx
	push	edi
	mov	edx,edi
	sub	edx,[ta_resource_data]
	bts	edx,31
	mov	[esi+4],edx
	lea	eax,[edi+16]
	cmp	eax,[ta_tagged_blocks]
	jae	ta_out_of_memory
	xor	eax,eax
	stos	dword [edi]
	call	ta_make_timestamp
	stos	dword [edi]
	xor	eax,eax
	stos	dword [edi]
	stos	dword [edi]
	mov	ebp,esi
	xor	ebx,ebx
      ta_make_resource_name_directory:
	mov	esi,[ta_free_additional_memory]
	xor	edx,edx
      ta_find_resource_name:
	cmp	dword [esi],0
	je	ta_resource_name_ok
	push	esi
	cmp	[esi+4],ebp
	jne	ta_check_next_resource_name
	add	esi,20
	call	ta_skip_resource_name
	cmp	word [esi],0FFFFh
	je	ta_check_next_resource_name
	or	ebx,ebx
	jz	ta_check_this_resource_name
	xor	ecx,ecx
      ta_compare_with_previous_resource_name:
	mov	ax,[esi+ecx]
	cmp	ax,[ebx+ecx]
	ja	ta_check_this_resource_name
	jb	ta_check_next_resource_name
	add	ecx,2
	mov	ax,[esi+ecx]
	or	ax,[ebx+ecx]
	jnz	ta_compare_with_previous_resource_name
	jmp	ta_check_next_resource_name
      ta_skip_resource_name:
	cmp	word [esi],0FFFFh
	jne	ta_skip_unicode_string
	add	esi,4
	ret
      ta_skip_unicode_string:
	add	esi,2
	cmp	word [esi-2],0
	jne	ta_skip_unicode_string
	ret
      ta_check_this_resource_name:
	or	edx,edx
	jz	ta_resource_name_found
	xor	ecx,ecx
      ta_compare_with_current_resource_name:
	mov	ax,[esi+ecx]
	cmp	ax,[edx+ecx]
	ja	ta_check_next_resource_name
	jb	ta_resource_name_found
	add	ecx,2
	mov	ax,[esi+ecx]
	or	ax,[edx+ecx]
	jnz	ta_compare_with_current_resource_name
	jmp	ta_same_resource_name
      ta_resource_name_found:
	mov	edx,esi
      ta_same_resource_name:
	mov	eax,[esp]
	mov	[eax+8],edi
      ta_check_next_resource_name:
	pop	esi
	mov	eax,[esi+16]
	lea	esi,[esi+20+eax]
	jmp	ta_find_resource_name
      ta_resource_name_ok:
	or	edx,edx
	jz	ta_resource_name_directory_done
	mov	ebx,edx
      ta_make_resource_name_entry:
	mov	eax,[esp]
	inc	word [eax+12]
	lea	eax,[edi+8]
	cmp	eax,[ta_tagged_blocks]
	jae	ta_out_of_memory
	mov	eax,ebx
	stos	dword [edi]
	xor	eax,eax
	stos	dword [edi]
	jmp	ta_make_resource_name_directory
      ta_resource_name_directory_done:
	mov	ebx,-1
      ta_make_resource_id_directory:
	mov	esi,[ta_free_additional_memory]
	mov	edx,10000h
      ta_find_resource_id:
	cmp	dword [esi],0
	je	ta_resource_id_ok
	push	esi
	cmp	[esi+4],ebp
	jne	ta_check_next_resource_id
	add	esi,20
	call	ta_skip_resource_name
	cmp	word [esi],0FFFFh
	jne	ta_check_next_resource_id
	movzx	eax,word [esi+2]
	cmp	eax,ebx
	jle	ta_check_next_resource_id
	cmp	eax,edx
	jg	ta_check_next_resource_id
	mov	edx,eax
	mov	eax,[esp]
	mov	[eax+8],edi
      ta_check_next_resource_id:
	pop	esi
	mov	eax,[esi+16]
	lea	esi,[esi+20+eax]
	jmp	ta_find_resource_id
      ta_resource_id_ok:
	cmp	edx,10000h
	je	ta_resource_id_directory_done
	mov	ebx,edx
      ta_make_resource_id_entry:
	mov	eax,[esp]
	inc	word [eax+14]
	lea	eax,[edi+8]
	cmp	eax,[ta_tagged_blocks]
	jae	ta_out_of_memory
	mov	eax,ebx
	stos	dword [edi]
	xor	eax,eax
	stos	dword [edi]
	jmp	ta_make_resource_id_directory
      ta_resource_id_directory_done:
	pop	eax
	mov	esi,ebp
	pop	ecx
	add	esi,8
	dec	cx
	jnz	ta_make_resource_directories
      ta_resource_directories_ok:
	shr	ecx,16
	jnz	ta_make_resource_directories
	mov	esi,[ta_resource_data]
	add	esi,10h
	movzx	eax,word [esi-4]
	movzx	edx,word [esi-2]
	add	eax,edx
	lea	esi,[esi+eax*8]
	push	edi			; ta_address of language directories
      ta_update_resource_directories:
	cmp	esi,[esp]
	je	ta_resource_directories_updated
	add	esi,10h
	mov	ecx,[esi-4]
	or	cx,cx
	jz	ta_language_directories_ok
      ta_make_language_directories:
	push	ecx
	push	edi
	mov	edx,edi
	sub	edx,[ta_resource_data]
	bts	edx,31
	mov	[esi+4],edx
	lea	eax,[edi+16]
	cmp	eax,[ta_tagged_blocks]
	jae	ta_out_of_memory
	xor	eax,eax
	stos	dword [edi]
	call	ta_make_timestamp
	stos	dword [edi]
	xor	eax,eax
	stos	dword [edi]
	stos	dword [edi]
	mov	ebp,esi
	mov	ebx,-1
      ta_make_language_id_directory:
	mov	esi,[ta_free_additional_memory]
	mov	edx,10000h
      ta_find_language_id:
	cmp	dword [esi],0
	je	ta_language_id_ok
	push	esi
	cmp	[esi+8],ebp
	jne	ta_check_next_language_id
	add	esi,20
	mov	eax,esi
	call	ta_skip_resource_name
	call	ta_skip_resource_name
	neg	eax
	add	eax,esi
	and	eax,11b
	add	esi,eax
      ta_get_language_id:
	movzx	eax,word [esi+6]
	cmp	eax,ebx
	jle	ta_check_next_language_id
	cmp	eax,edx
	jge	ta_check_next_language_id
	mov	edx,eax
	mov	eax,[esp]
	mov	dword [ta_value],eax
      ta_check_next_language_id:
	pop	esi
	mov	eax,[esi+16]
	lea	esi,[esi+20+eax]
	jmp	ta_find_language_id
      ta_language_id_ok:
	cmp	edx,10000h
	je	ta_language_id_directory_done
	mov	ebx,edx
      ta_make_language_id_entry:
	mov	eax,[esp]
	inc	word [eax+14]
	lea	eax,[edi+8]
	cmp	eax,[ta_tagged_blocks]
	jae	ta_out_of_memory
	mov	eax,ebx
	stos	dword [edi]
	mov	eax,dword [ta_value]
	stos	dword [edi]
	jmp	ta_make_language_id_directory
      ta_language_id_directory_done:
	pop	eax
	mov	esi,ebp
	pop	ecx
	add	esi,8
	dec	cx
	jnz	ta_make_language_directories
      ta_language_directories_ok:
	shr	ecx,16
	jnz	ta_make_language_directories
	jmp	ta_update_resource_directories
      ta_resource_directories_updated:
	mov	esi,[ta_resource_data]
	push	edi
      ta_make_name_strings:
	add	esi,10h
	movzx	eax,word [esi-2]
	movzx	ecx,word [esi-4]
	add	eax,ecx
	lea	eax,[esi+eax*8]
	push	eax
	or	ecx,ecx
	jz	ta_string_entries_processed
      ta_process_string_entries:
	push	ecx
	mov	edx,edi
	sub	edx,[ta_resource_data]
	bts	edx,31
	xchg	[esi],edx
	mov	ebx,edi
	xor	ax,ax
	stos	word [edi]
      ta_copy_string_data:
	lea	eax,[edi+2]
	cmp	eax,[ta_tagged_blocks]
	jae	ta_out_of_memory
	mov	ax,[edx]
	or	ax,ax
	jz	ta_string_data_copied
	stos	word [edi]
	inc	word [ebx]
	add	edx,2
	jmp	ta_copy_string_data
      ta_string_data_copied:
	add	esi,8
	pop	ecx
	loop	ta_process_string_entries
      ta_string_entries_processed:
	pop	esi
	cmp	esi,[esp]
	jb	ta_make_name_strings
	mov	eax,edi
	sub	eax,[ta_resource_data]
	test	al,11b
	jz	ta_resource_strings_alignment_ok
	xor	ax,ax
	stos	word [edi]
      ta_resource_strings_alignment_ok:
	pop	edx
	pop	ebx			; ta_address of language directories
	mov	ebp,edi
      ta_update_language_directories:
	add	ebx,10h
	movzx	eax,word [ebx-2]
	movzx	ecx,word [ebx-4]
	add	ecx,eax
      ta_make_data_records:
	push	ecx
	mov	esi,edi
	sub	esi,[ta_resource_data]
	xchg	esi,[ebx+4]
	lea	eax,[edi+16]
	cmp	eax,[ta_tagged_blocks]
	jae	ta_out_of_memory
	mov	eax,esi
	stos	dword [edi]
	mov	eax,[esi+12]
	stos	dword [edi]
	xor	eax,eax
	stos	dword [edi]
	stos	dword [edi]
	pop	ecx
	add	ebx,8
	loop	ta_make_data_records
	cmp	ebx,edx
	jb	ta_update_language_directories
	pop	ebx			; file handle
	mov	esi,ebp
	mov	ebp,edi
      ta_update_data_records:
	push	ebp
	mov	ecx,edi
	mov	eax,[ta_current_section]
	sub	ecx,[eax+14h]
	add	ecx,[eax+0Ch]
	xchg	ecx,[esi]
	mov	edx,[ecx]
	xor	al,al
	call	ta_lseek
	mov	edx,edi
	mov	ecx,[esi+4]
	add	edi,ecx
	cmp	edi,[ta_tagged_blocks]
	ja	ta_out_of_memory
	call	ta_read
	mov	eax,edi
	sub	eax,[ta_resource_data]
	and	eax,11b
	jz	ta_resource_data_alignment_ok
	mov	ecx,4
	sub	ecx,eax
	xor	al,al
	rep	stos byte [edi]
      ta_resource_data_alignment_ok:
	pop	ebp
	add	esi,16
	cmp	esi,ebp
	jb	ta_update_data_records
	pop	esi
	call	ta_close
	mov	eax,edi
	sub	eax,[ta_resource_data]
	mov	[ta_resource_size],eax
      ta_resource_done:
	ret
ta_close_pe:
	call	ta_close_pe_section
	mov	edx,[ta_code_start]
	mov	[edx+50h],eax
	call	ta_make_timestamp
	mov	edx,[ta_code_start]
	mov	[edx+8],eax
	mov	eax,[ta_number_of_sections]
	mov	[edx+6],ax
	imul	eax,28h
	movzx	ecx,word [edx+14h]
	lea	eax,[eax+18h+ecx]
	add	eax,[ta_stub_size]
	mov	ecx,[edx+3Ch]
	dec	ecx
	add	eax,ecx
	not	ecx
	and	eax,ecx
	cmp	eax,[edx+54h]
	je	ta_pe_sections_ok
	or	[ta_next_pass_needed],-1
      ta_pe_sections_ok:
	xor	ecx,ecx
	add	edx,78h
	test	[ta_format_flags],4
	jz	ta_process_directories
	add	edx,10h
      ta_process_directories:
	mov	eax,[edx+ecx*8]
	or	eax,eax
	jz	ta_directory_ok
	cmp	dword [edx+ecx*8+4],-1
	jne	ta_directory_ok
      ta_section_data:
	mov	ebx,[edx+ecx*8]
	mov	eax,[ebx+0Ch]
	mov	[edx+ecx*8],eax 	; directory rva
	mov	eax,[ebx+8]
	mov	[edx+ecx*8+4],eax	; directory size
      ta_directory_ok:
	inc	cl
	cmp	cl,10h
	jb	ta_process_directories
	cmp	dword [edx+5*8],0
	jne	ta_finish_pe_relocations
	mov	eax,[ta_number_of_relocations]
	shl	eax,2
	sub	[ta_free_additional_memory],eax
	btr	[ta_resolver_flags],0
	jnc	ta_pe_relocations_ok
	or	[ta_next_pass_needed],-1
	jmp	ta_pe_relocations_ok
      ta_finish_pe_relocations:
	push	edi
	mov	edi,[ta_reserved_fixups]
	call	ta_make_fixups
	pop	edi
	add	[ta_actual_fixups_size],eax
	cmp	eax,[ta_reserved_fixups_size]
	je	ta_pe_relocations_ok
	or	[ta_next_pass_needed],-1
      ta_pe_relocations_ok:
	mov	ebx,[ta_code_start]
	sub	ebx,[ta_stub_size]
	mov	ecx,edi
	sub	ecx,ebx
	mov	ebp,ecx
	shr	ecx,1
	xor	eax,eax
	cdq
      ta_calculate_checksum:
	mov	dx,[ebx]
	add	eax,edx
	mov	dx,ax
	shr	eax,16
	add	eax,edx
	add	ebx,2
	loop	ta_calculate_checksum
	add	eax,ebp
	mov	ebx,[ta_code_start]
	mov	[ebx+58h],eax
	ret

      ta_close_coff_section:
	mov	ebx,[ta_current_section]
	mov	eax,edi
	mov	edx,[ebx+8]
	sub	eax,edx
	mov	[ebx+0Ch],eax
	xor	eax,eax
	xchg	[ta_undefined_data_end],eax
	cmp	eax,edi
	jne	ta_coff_section_ok
	cmp	edx,[ta_undefined_data_start]
	jne	ta_coff_section_ok
	mov	edi,edx
	or	byte [ebx+14h],80h
      ta_coff_section_ok:
	ret
ta_mark_coff_relocation:
	cmp	[ta_value_type],3
	je	ta_coff_relocation_relative
	push	ebx eax
	test	[ta_format_flags],8
	jnz	ta_coff_64bit_relocation
	mov	al,6
	cmp	[ta_value_type],2
	je	ta_coff_relocation
	cmp	[ta_value_type],5
	jne	ta_invalid_use_of_symbol
	inc	al
	jmp	ta_coff_relocation
      ta_coff_64bit_relocation:
	mov	al,1
	cmp	[ta_value_type],4
	je	ta_coff_relocation
	mov	al,2
	cmp	[ta_value_type],2
	je	ta_coff_relocation
	cmp	[ta_value_type],5
	jne	ta_invalid_use_of_symbol
	inc	al
	jmp	ta_coff_relocation
      ta_coff_relocation_relative:
	push	ebx
	bt	[ta_format_flags],0
	jnc	ta_relative_ok
	mov	ebx,[ta_current_section]
	mov	ebx,[ebx+8]
	sub	ebx,edi
	sub	eax,ebx
	add	eax,4
      ta_relative_ok:
	mov	ebx,[ta_addressing_space]
	push	eax
	mov	al,20
	test	[ta_format_flags],8
	jnz	ta_relative_coff_64bit_relocation
	cmp	byte [ebx+9],2
	jne	ta_invalid_use_of_symbol
	jmp	ta_coff_relocation
      ta_relative_coff_64bit_relocation:
	mov	al,4
	cmp	byte [ebx+9],4
	jne	ta_invalid_use_of_symbol
      ta_coff_relocation:
	mov	ebx,[ta_free_additional_memory]
	add	ebx,0Ch
	cmp	ebx,[ta_structures_buffer]
	jae	ta_out_of_memory
	mov	[ta_free_additional_memory],ebx
	mov	byte [ebx-0Ch],al
	mov	eax,[ta_current_section]
	mov	eax,[eax+8]
	neg	eax
	add	eax,edi
	mov	[ebx-0Ch+4],eax
	mov	eax,[ta_symbol_identifier]
	mov	[ebx-0Ch+8],eax
	pop	eax ebx
	ret

      ta_prepare_default_section:
	mov	ebx,[ta_symbols_stream]
	cmp	dword [ebx+0Ch],0
	jne	ta_default_section_ok
	cmp	[ta_number_of_sections],0
	je	ta_default_section_ok
	mov	edx,ebx
      ta_find_references_to_default_section:
	cmp	ebx,[ta_free_additional_memory]
	jne	ta_check_reference
	add	[ta_symbols_stream],20h
	ret
      ta_check_reference:
	mov	al,[ebx]
	or	al,al
	jz	ta_skip_other_section
	cmp	al,0C0h
	jae	ta_check_public_reference
	cmp	al,80h
	jae	ta_next_reference
	cmp	edx,[ebx+8]
	je	ta_default_section_ok
      ta_next_reference:
	add	ebx,0Ch
	jmp	ta_find_references_to_default_section
      ta_check_public_reference:
	mov	eax,[ebx+8]
	add	ebx,10h
	test	byte [eax+8],1
	jz	ta_find_references_to_default_section
	mov	cx,[ta_current_pass]
	cmp	cx,[eax+16]
	jne	ta_find_references_to_default_section
	cmp	edx,[eax+20]
	je	ta_default_section_ok
	jmp	ta_find_references_to_default_section
      ta_skip_other_section:
	add	ebx,20h
	jmp	ta_find_references_to_default_section
      ta_default_section_ok:
	inc	[ta_number_of_sections]
	ret
      ta_symbols_enumerated:
	mov	[ebx+0Ch],eax
	mov	ebp,edi
	sub	ebp,ebx
	push	ebp
	lea	edi,[ebx+14h]
	mov	esi,[ta_symbols_stream]
      ta_find_section:
	cmp	esi,[ta_free_additional_memory]
	je	ta_sections_finished
	mov	al,[esi]
	or	al,al
	jz	ta_section_found
	add	esi,0Ch
	cmp	al,0C0h
	jb	ta_find_section
	add	esi,4
	jmp	ta_find_section
      ta_section_found:
	push	esi edi
	mov	esi,[esi+4]
	or	esi,esi
	jz	ta_default_section
	mov	ecx,[esi]
	add	esi,4
	rep	movs byte [edi],[esi]
	jmp	ta_section_name_ok
      ta_default_section:
	mov	al,'.'
	stos	byte [edi]
	mov	eax,'flat'
	stos	dword [edi]
      ta_section_name_ok:
	pop	edi esi
	mov	eax,[esi+0Ch]
	mov	[edi+10h],eax
	mov	eax,[esi+14h]
	mov	[edi+24h],eax
	test	al,80h
	jnz	ta_section_ptr_ok
	mov	eax,[esi+8]
	sub	eax,[ta_code_start]
	add	eax,ebp
	mov	[edi+14h],eax
      ta_section_ptr_ok:
	mov	ebx,[ta_code_start]
	mov	edx,[ta_code_size]
	add	ebx,edx
	add	edx,ebp
	xor	ecx,ecx
	add	esi,20h
      ta_find_relocations:
	cmp	esi,[ta_free_additional_memory]
	je	ta_section_relocations_done
	mov	al,[esi]
	or	al,al
	jz	ta_section_relocations_done
	cmp	al,80h
	jb	ta_add_relocation
	cmp	al,0C0h
	jb	ta_next_relocation
	add	esi,10h
	jmp	ta_find_relocations
      ta_add_relocation:
	lea	eax,[ebx+0Ah]
	cmp	eax,[ta_tagged_blocks]
	ja	ta_out_of_memory
	mov	eax,[esi+4]
	mov	[ebx],eax
	mov	eax,[esi+8]
	mov	eax,[eax]
	shr	eax,8
	mov	[ebx+4],eax
	movzx	ax,byte [esi]
	mov	[ebx+8],ax
	add	ebx,0Ah
	inc	ecx
      ta_next_relocation:
	add	esi,0Ch
	jmp	ta_find_relocations
      ta_section_relocations_done:
	cmp	ecx,10000h
	jb	ta_section_relocations_count_16bit
	bt	[ta_format_flags],0
	jnc	ta_format_limitations_exceeded
	mov	word [edi+20h],0FFFFh
	or	dword [edi+24h],1000000h
	mov	[edi+18h],edx
	push	esi edi
	push	ecx
	lea	esi,[ebx-1]
	add	ebx,0Ah
	lea	edi,[ebx-1]
	imul	ecx,0Ah
	std
	rep	movs byte [edi],[esi]
	cld
	pop	ecx
	inc	esi
	inc	ecx
	mov	[esi],ecx
	xor	eax,eax
	mov	[esi+4],eax
	mov	[esi+8],ax
	pop	edi esi
	jmp	ta_section_relocations_ok
      ta_section_relocations_count_16bit:
	mov	[edi+20h],cx
	jcxz	ta_section_relocations_ok
	mov	[edi+18h],edx
      ta_section_relocations_ok:
	sub	ebx,[ta_code_start]
	mov	[ta_code_size],ebx
	add	edi,28h
	jmp	ta_find_section
      ta_sections_finished:
	mov	edx,[ta_free_additional_memory]
	mov	ebx,[ta_code_size]
	add	ebp,ebx
	mov	[edx+8],ebp
	add	ebx,[ta_code_start]
	mov	edi,ebx
	mov	ecx,[edx+0Ch]
	imul	ecx,12h shr 1
	xor	eax,eax
	shr	ecx,1
	jnc	ta_zero_symbols_table
	stos	word [edi]
      ta_zero_symbols_table:
	rep	stos dword [edi]
	mov	edx,edi
	stos	dword [edi]
	mov	esi,[ta_symbols_stream]
      ta_make_symbols_table:
	cmp	esi,[ta_free_additional_memory]
	je	ta_symbols_table_ok
	mov	al,[esi]
	cmp	al,0C0h
	jae	ta_add_public_symbol
	cmp	al,80h
	jae	ta_add_extrn_symbol
	or	al,al
	jz	ta_add_section_symbol
	add	esi,0Ch
	jmp	ta_make_symbols_table
      ta_add_section_symbol:
	call	ta_store_symbol_name
	movzx	eax,word [esi+1Eh]
	mov	[ebx+0Ch],ax
	mov	byte [ebx+10h],3
	add	esi,20h
	add	ebx,12h
	jmp	ta_make_symbols_table
      ta_add_extrn_symbol:
	call	ta_store_symbol_name
	mov	byte [ebx+10h],2
	add	esi,0Ch
	add	ebx,12h
	jmp	ta_make_symbols_table
      ta_add_public_symbol:
	call	ta_store_symbol_name
	mov	eax,[esi+0Ch]
	mov	[ta_current_line],eax
	mov	eax,[esi+8]
	test	byte [eax+8],1
	jz	ta_undefined_coff_public
	mov	cx,[ta_current_pass]
	cmp	cx,[eax+16]
	jne	ta_undefined_coff_public
	mov	cl,[eax+11]
	or	cl,cl
	jz	ta_public_constant
	test	[ta_format_flags],8
	jnz	ta_check_64bit_public_symbol
	cmp	cl,2
	je	ta_public_symbol_type_ok
	jmp	ta_invalid_use_of_symbol
      ta_undefined_coff_public:
	mov	[ta_error_info],eax
	jmp	ta_undefined_symbol
      ta_check_64bit_public_symbol:
	cmp	cl,4
	jne	ta_invalid_use_of_symbol
      ta_public_symbol_type_ok:
	mov	ecx,[eax+20]
	cmp	byte [ecx],80h
	je	ta_alias_symbol
	cmp	byte [ecx],0
	jne	ta_invalid_use_of_symbol
	mov	cx,[ecx+1Eh]
	mov	[ebx+0Ch],cx
      ta_public_symbol_section_ok:
	movzx	ecx,byte [eax+9]
	shr	cl,1
	and	cl,1
	neg	ecx
	cmp	ecx,[eax+4]
	jne	ta_value_out_of_range
	xor	ecx,[eax]
	js	ta_value_out_of_range
	mov	eax,[eax]
	mov	[ebx+8],eax
	mov	al,2
	cmp	byte [esi],0C0h
	je	ta_store_symbol_class
	inc	al
	cmp	byte [esi],0C1h
	je	ta_store_symbol_class
	mov	al,105
      ta_store_symbol_class:
	mov	byte [ebx+10h],al
	add	esi,10h
	add	ebx,12h
	jmp	ta_make_symbols_table
      ta_alias_symbol:
	bt	[ta_format_flags],0
	jnc	ta_invalid_use_of_symbol
	mov	ecx,[eax]
	or	ecx,[eax+4]
	jnz	ta_invalid_use_of_symbol
	mov	byte [ebx+10h],69h
	mov	byte [ebx+11h],1
	add	ebx,12h
	mov	ecx,[eax+20]
	mov	ecx,[ecx]
	shr	ecx,8
	mov	[ebx],ecx
	mov	byte [ebx+4],3
	add	esi,10h
	add	ebx,12h
	jmp	ta_make_symbols_table
      ta_public_constant:
	mov	word [ebx+0Ch],0FFFFh
	jmp	ta_public_symbol_section_ok
      ta_symbols_table_ok:
	mov	eax,edi
	sub	eax,edx
	mov	[edx],eax
	sub	edi,[ta_code_start]
	mov	[ta_code_size],edi
	and	[ta_written_size],0
	mov	edx,[ta_output_file]
	call	ta_create
	jc	ta_write_failed
	mov	edx,[ta_free_additional_memory]
	pop	ecx
	add	[ta_written_size],ecx
	call	ta_write
	jc	ta_write_failed
	jmp	ta_write_output
      ta_store_symbol_name:
	push	esi
	mov	esi,[esi+4]
	or	esi,esi
	jz	ta_default_name
	lods	dword [esi]
	mov	ecx,eax
	cmp	ecx,8
	ja	ta_add_string
	push	edi
	mov	edi,ebx
	rep	movs byte [edi],[esi]
	pop	edi esi
	ret
      ta_default_name:
	mov	dword [ebx],'.fla'
	mov	dword [ebx+4],'t'
	pop	esi
	ret
      ta_add_string:
	mov	eax,edi
	sub	eax,edx
	mov	[ebx+4],eax
	inc	ecx
	rep	movs byte [edi],[esi]
	pop	esi
	ret

ta_format_elf:
	test	[ta_format_flags],8
	jnz	ta_format_elf64
	mov	edx,edi
	mov	ecx,34h shr 2
	lea	eax,[edi+ecx*4]
	cmp	eax,[ta_tagged_blocks]
	jae	ta_out_of_memory
	xor	eax,eax
	rep	stos dword [edi]
	mov	dword [edx],7Fh + 'ELF' shl 8
	mov	al,1
	mov	[edx+4],al
	mov	[edx+5],al
	mov	[edx+6],al
	mov	[edx+14h],al
	mov	byte [edx+12h],3
	mov	byte [edx+28h],34h
	mov	byte [edx+2Eh],28h
	mov	[ta_code_type],32
	mov	byte [edx+10h],2
	cmp	word [esi],1D19h
	je	ta_format_elf_exe
	mov	byte [edx+10h],3
	cmp	word [esi],021Eh
	je	ta_format_elf_exe
      ta_elf_header_ok:
	mov	byte [edx+10h],1
	mov	eax,[ta_additional_memory]
	mov	[ta_symbols_stream],eax
	mov	ebx,eax
	add	eax,20h
	cmp	eax,[ta_structures_buffer]
	jae	ta_out_of_memory
	mov	[ta_free_additional_memory],eax
	xor	eax,eax
	mov	[ta_current_section],ebx
	mov	[ta_number_of_sections],eax
	mov	[ebx],al
	mov	[ebx+4],eax
	mov	[ebx+8],edi
	mov	al,111b
	mov	[ebx+14h],eax
	mov	al,4
	mov	[ebx+10h],eax
	mov	edx,ebx
	call	ta_init_addressing_space
	xchg	edx,ebx
	mov	[edx+14h],ebx
	mov	byte [edx+9],2
	test	[ta_format_flags],8
	jz	ta_format_defined
	mov	byte [edx+9],4
	mov	byte [ebx+10h],8
	jmp	ta_format_defined
      ta_format_elf64:
	mov	edx,edi
	mov	ecx,40h shr 2
	lea	eax,[edi+ecx*4]
	cmp	eax,[ta_tagged_blocks]
	jae	ta_out_of_memory
	xor	eax,eax
	rep	stos dword [edi]
	mov	dword [edx],7Fh + 'ELF' shl 8
	mov	al,1
	mov	[edx+5],al
	mov	[edx+6],al
	mov	[edx+14h],al
	mov	byte [edx+4],2
	mov	byte [edx+12h],62
	mov	byte [edx+34h],40h
	mov	byte [edx+3Ah],40h
	mov	[ta_code_type],64
	mov	byte [edx+10h],2
	cmp	word [esi],1D19h
	je	ta_format_elf64_exe
	mov	byte [edx+10h],3
	cmp	word [esi],021Eh
	je	ta_format_elf64_exe
	jmp	ta_elf_header_ok
ta_elf_section:
	bt	[ta_format_flags],0
	jc	ta_illegal_instruction
	call	ta_close_coff_section
	mov	ebx,[ta_free_additional_memory]
	lea	eax,[ebx+20h]
	cmp	eax,[ta_structures_buffer]
	jae	ta_out_of_memory
	mov	[ta_free_additional_memory],eax
	mov	[ta_current_section],ebx
	inc	word [ta_number_of_sections]
	jz	ta_format_limitations_exceeded
	xor	eax,eax
	mov	[ebx],al
	mov	[ebx+8],edi
	mov	[ebx+10h],eax
	mov	al,10b
	mov	[ebx+14h],eax
	mov	edx,ebx
	call	ta_create_addressing_space
	xchg	edx,ebx
	mov	[edx+14h],ebx
	mov	byte [edx+9],2
	test	[ta_format_flags],8
	jz	ta_elf_labels_type_ok
	mov	byte [edx+9],4
      ta_elf_labels_type_ok:
	lods	word [esi]
	cmp	ax,'('
	jne	ta_invalid_argument
	mov	[ebx+4],esi
	mov	ecx,[esi]
	lea	esi,[esi+4+ecx+1]
      ta_elf_section_flags:
	cmp	byte [esi],8Ch
	je	ta_elf_section_alignment
	cmp	byte [esi],19h
	jne	ta_elf_section_settings_ok
	inc	esi
	lods	byte [esi]
	sub	al,28
	xor	al,11b
	test	al,not 10b
	jnz	ta_invalid_argument
	mov	cl,al
	mov	al,1
	shl	al,cl
	test	byte [ebx+14h],al
	jnz	ta_setting_already_specified
	or	byte [ebx+14h],al
	jmp	ta_elf_section_flags
      ta_elf_section_alignment:
	inc	esi
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_argument
	cmp	byte [esi],'.'
	je	ta_invalid_value
	push	ebx
	call	ta_get_count_value
	pop	ebx
	mov	edx,eax
	dec	edx
	test	eax,edx
	jnz	ta_invalid_value
	or	eax,eax
	jz	ta_invalid_value
	xchg	[ebx+10h],eax
	or	eax,eax
	jnz	ta_setting_already_specified
	jmp	ta_elf_section_flags
      ta_elf_section_settings_ok:
	cmp	dword [ebx+10h],0
	jne	ta_instruction_assembled
	mov	dword [ebx+10h],4
	test	[ta_format_flags],8
	jz	ta_instruction_assembled
	mov	byte [ebx+10h],8
	jmp	ta_instruction_assembled
ta_mark_elf_relocation:
	test	[ta_format_flags],1
	jnz	ta_invalid_use_of_symbol
	push	ebx
	mov	ebx,[ta_addressing_space]
	cmp	[ta_value_type],3
	je	ta_elf_relocation_relative
	cmp	[ta_value_type],7
	je	ta_elf_relocation_relative
	push	eax
	cmp	[ta_value_type],5
	je	ta_elf_gotoff_relocation
	ja	ta_invalid_use_of_symbol
	mov	al,1			; R_386_32 / R_AMD64_64
	test	[ta_format_flags],8
	jz	ta_coff_relocation
	cmp	[ta_value_type],4
	je	ta_coff_relocation
	mov	al,11			; R_AMD64_32S
	jmp	ta_coff_relocation
      ta_elf_gotoff_relocation:
	test	[ta_format_flags],8
	jnz	ta_invalid_use_of_symbol
	mov	al,9			; R_386_GOTOFF
	jmp	ta_coff_relocation
      ta_elf_relocation_relative:
	cmp	byte [ebx+9],0
	je	ta_invalid_use_of_symbol
	mov	ebx,[ta_current_section]
	mov	ebx,[ebx+8]
	sub	ebx,edi
	sub	eax,ebx
	push	eax
	mov	al,2			; R_386_PC32 / R_AMD64_PC32
	cmp	[ta_value_type],3
	je	ta_coff_relocation
	mov	al,4			; R_386_PLT32 / R_AMD64_PLT32
	jmp	ta_coff_relocation
ta_close_elf:
	bt	[ta_format_flags],0
	jc	ta_close_elf_exe
	call	ta_close_coff_section
	cmp	[ta_next_pass_needed],0
	je	ta_elf_closed
	mov	eax,[ta_symbols_stream]
	mov	[ta_free_additional_memory],eax
      ta_elf_closed:
	ret
ta_elf_formatter:
	mov	ecx,edi
	sub	ecx,[ta_code_start]
	neg	ecx
	and	ecx,111b
	test	[ta_format_flags],8
	jnz	ta_align_elf_structures
	and	ecx,11b
      ta_align_elf_structures:
	xor	al,al
	rep	stos byte [edi]
	push	edi
	call	ta_prepare_default_section
	mov	esi,[ta_symbols_stream]
	mov	edi,[ta_free_additional_memory]
	xor	eax,eax
	mov	ecx,4
	rep	stos dword [edi]
	test	[ta_format_flags],8
	jz	ta_find_first_section
	mov	ecx,2
	rep	stos dword [edi]
      ta_find_first_section:
	mov	al,[esi]
	or	al,al
	jz	ta_first_section_found
	cmp	al,0C0h
	jb	ta_skip_other_symbol
	add	esi,4
      ta_skip_other_symbol:
	add	esi,0Ch
	jmp	ta_find_first_section
      ta_first_section_found:
	mov	ebx,esi
	mov	ebp,esi
	add	esi,20h
	xor	ecx,ecx
	xor	edx,edx
      ta_find_next_section:
	cmp	esi,[ta_free_additional_memory]
	je	ta_make_section_symbol
	mov	al,[esi]
	or	al,al
	jz	ta_make_section_symbol
	cmp	al,0C0h
	jae	ta_skip_public
	cmp	al,80h
	jae	ta_skip_extrn
	or	byte [ebx+14h],40h
      ta_skip_extrn:
	add	esi,0Ch
	jmp	ta_find_next_section
      ta_skip_public:
	add	esi,10h
	jmp	ta_find_next_section
      ta_make_section_symbol:
	mov	eax,edi
	xchg	eax,[ebx+4]
	stos	dword [edi]
	test	[ta_format_flags],8
	jnz	ta_elf64_section_symbol
	xor	eax,eax
	stos	dword [edi]
	stos	dword [edi]
	call	ta_store_section_index
	jmp	ta_section_symbol_ok
      ta_store_section_index:
	inc	ecx
	mov	eax,ecx
	shl	eax,8
	mov	[ebx],eax
	inc	dx
	jz	ta_format_limitations_exceeded
	mov	eax,edx
	shl	eax,16
	mov	al,3
	test	byte [ebx+14h],40h
	jz	ta_section_index_ok
	or	ah,-1
	inc	dx
	jz	ta_format_limitations_exceeded
      ta_section_index_ok:
	stos	dword [edi]
	ret
      ta_elf64_section_symbol:
	call	ta_store_section_index
	xor	eax,eax
	stos	dword [edi]
	stos	dword [edi]
	stos	dword [edi]
	stos	dword [edi]
      ta_section_symbol_ok:
	mov	ebx,esi
	add	esi,20h
	cmp	ebx,[ta_free_additional_memory]
	jne	ta_find_next_section
	inc	dx
	jz	ta_format_limitations_exceeded
	mov	[ta_current_section],edx
	mov	esi,[ta_symbols_stream]
      ta_find_other_symbols:
	cmp	esi,[ta_free_additional_memory]
	je	ta_elf_symbol_table_ok
	mov	al,[esi]
	or	al,al
	jz	ta_skip_section
	cmp	al,0C0h
	jae	ta_make_public_symbol
	cmp	al,80h
	jae	ta_make_extrn_symbol
	add	esi,0Ch
	jmp	ta_find_other_symbols
      ta_skip_section:
	add	esi,20h
	jmp	ta_find_other_symbols
      ta_make_public_symbol:
	mov	eax,[esi+0Ch]
	mov	[ta_current_line],eax
	cmp	byte [esi],0C0h
	jne	ta_invalid_argument
	mov	ebx,[esi+8]
	test	byte [ebx+8],1
	jz	ta_undefined_public
	mov	ax,[ta_current_pass]
	cmp	ax,[ebx+16]
	jne	ta_undefined_public
	mov	dl,[ebx+11]
	or	dl,dl
	jz	ta_public_absolute
	mov	eax,[ebx+20]
	cmp	byte [eax],0
	jne	ta_invalid_use_of_symbol
	mov	eax,[eax+4]
	test	[ta_format_flags],8
	jnz	ta_elf64_public
	cmp	dl,2
	jne	ta_invalid_use_of_symbol
	mov	dx,[eax+0Eh]
	jmp	ta_section_for_public_ok
      ta_undefined_public:
	mov	[ta_error_info],ebx
	jmp	ta_undefined_symbol
      ta_elf64_public:
	cmp	dl,4
	jne	ta_invalid_use_of_symbol
	mov	dx,[eax+6]
	jmp	ta_section_for_public_ok
      ta_public_absolute:
	mov	dx,0FFF1h
      ta_section_for_public_ok:
	mov	eax,[esi+4]
	stos	dword [edi]
	test	[ta_format_flags],8
	jnz	ta_elf64_public_symbol
	movzx	eax,byte [ebx+9]
	shr	al,1
	and	al,1
	neg	eax
	cmp	eax,[ebx+4]
	jne	ta_value_out_of_range
	xor	eax,[ebx]
	js	ta_value_out_of_range
	mov	eax,[ebx]
	stos	dword [edi]
	xor	eax,eax
	mov	al,[ebx+10]
	stos	dword [edi]
	mov	eax,edx
	shl	eax,16
	mov	al,10h
	cmp	byte [ebx+10],0
	je	ta_elf_public_function
	or	al,1
	jmp	ta_store_elf_public_info
      ta_elf_public_function:
	or	al,2
      ta_store_elf_public_info:
	stos	dword [edi]
	jmp	ta_public_symbol_ok
      ta_elf64_public_symbol:
	mov	eax,edx
	shl	eax,16
	mov	al,10h
	cmp	byte [ebx+10],0
	je	ta_elf64_public_function
	or	al,1
	jmp	ta_store_elf64_public_info
      ta_elf64_public_function:
	or	al,2
      ta_store_elf64_public_info:
	stos	dword [edi]
	mov	al,[ebx+9]
	shl	eax,31-1
	xor	eax,[ebx+4]
	js	ta_value_out_of_range
	mov	eax,[ebx]
	stos	dword [edi]
	mov	eax,[ebx+4]
	stos	dword [edi]
	mov	al,[ebx+10]
	stos	dword [edi]
	xor	al,al
	stos	dword [edi]
      ta_public_symbol_ok:
	inc	ecx
	mov	eax,ecx
	shl	eax,8
	mov	al,0C0h
	mov	[esi],eax
	add	esi,10h
	jmp	ta_find_other_symbols
      ta_make_extrn_symbol:
	mov	eax,[esi+4]
	stos	dword [edi]
	test	[ta_format_flags],8
	jnz	ta_elf64_extrn_symbol
	xor	eax,eax
	stos	dword [edi]
	mov	eax,[esi+8]
	stos	dword [edi]
	mov	eax,10h
	stos	dword [edi]
	jmp	ta_extrn_symbol_ok
      ta_elf64_extrn_symbol:
	mov	eax,10h
	stos	dword [edi]
	xor	al,al
	stos	dword [edi]
	stos	dword [edi]
	mov	eax,[esi+8]
	stos	dword [edi]
	xor	eax,eax
	stos	dword [edi]
      ta_extrn_symbol_ok:
	inc	ecx
	mov	eax,ecx
	shl	eax,8
	mov	al,80h
	mov	[esi],eax
	add	esi,0Ch
	jmp	ta_find_other_symbols
      ta_elf_symbol_table_ok:
	mov	edx,edi
	mov	ebx,[ta_free_additional_memory]
	xor	al,al
	stos	byte [edi]
	add	edi,16
	mov	[edx+1],edx
	add	ebx,10h
	test	[ta_format_flags],8
	jz	ta_make_string_table
	add	ebx,8
      ta_make_string_table:
	cmp	ebx,edx
	je	ta_elf_string_table_ok
	test	[ta_format_flags],8
	jnz	ta_make_elf64_string
	cmp	byte [ebx+0Dh],0
	je	ta_rel_prefix_ok
	mov	byte [ebx+0Dh],0
	mov	eax,'.rel'
	stos	dword [edi]
      ta_rel_prefix_ok:
	mov	esi,edi
	sub	esi,edx
	xchg	esi,[ebx]
	add	ebx,10h
      ta_make_elf_string:
	or	esi,esi
	jz	ta_default_string
	lods	dword [esi]
	mov	ecx,eax
	rep	movs byte [edi],[esi]
	xor	al,al
	stos	byte [edi]
	jmp	ta_make_string_table
      ta_make_elf64_string:
	cmp	byte [ebx+5],0
	je	ta_elf64_rel_prefix_ok
	mov	byte [ebx+5],0
	mov	eax,'.rel'
	stos	dword [edi]
	mov	al,'a'
	stos	byte [edi]
      ta_elf64_rel_prefix_ok:
	mov	esi,edi
	sub	esi,edx
	xchg	esi,[ebx]
	add	ebx,18h
	jmp	ta_make_elf_string
      ta_default_string:
	mov	eax,'.fla'
	stos	dword [edi]
	mov	ax,'t'
	stos	word [edi]
	jmp	ta_make_string_table
      ta_elf_string_table_ok:
	mov	[edx+1+8],edi
	mov	ebx,[ta_code_start]
	mov	eax,edi
	sub	eax,[ta_free_additional_memory]
	xor	ecx,ecx
	sub	ecx,eax
	test	[ta_format_flags],8
	jnz	ta_finish_elf64_header
	and	ecx,11b
	add	eax,ecx
	mov	[ebx+20h],eax
	mov	eax,[ta_current_section]
	inc	ax
	jz	ta_format_limitations_exceeded
	mov	[ebx+32h],ax
	inc	ax
	jz	ta_format_limitations_exceeded
	mov	[ebx+30h],ax
	jmp	ta_elf_header_finished
      ta_finish_elf64_header:
	and	ecx,111b
	add	eax,ecx
	mov	[ebx+28h],eax
	mov	eax,[ta_current_section]
	inc	ax
	jz	ta_format_limitations_exceeded
	mov	[ebx+3Eh],ax
	inc	ax
	jz	ta_format_limitations_exceeded
	mov	[ebx+3Ch],ax
      ta_elf_header_finished:
	xor	eax,eax
	add	ecx,10*4
	rep	stos byte [edi]
	test	[ta_format_flags],8
	jz	ta_elf_null_section_ok
	mov	ecx,6*4
	rep	stos byte [edi]
      ta_elf_null_section_ok:
	mov	esi,ebp
	xor	ecx,ecx
      ta_make_section_entry:
	mov	ebx,edi
	mov	eax,[esi+4]
	mov	eax,[eax]
	stos	dword [edi]
	mov	eax,1
	cmp	dword [esi+0Ch],0
	je	ta_bss_section
	test	byte [esi+14h],80h
	jz	ta_section_type_ok
      ta_bss_section:
	mov	al,8
      ta_section_type_ok:
	stos	dword [edi]
	mov	eax,[esi+14h]
	and	al,3Fh
	call	ta_store_elf_machine_word
	xor	eax,eax
	call	ta_store_elf_machine_word
	mov	eax,[esi+8]
	mov	[ta_image_base],eax
	sub	eax,[ta_code_start]
	call	ta_store_elf_machine_word
	mov	eax,[esi+0Ch]
	call	ta_store_elf_machine_word
	xor	eax,eax
	stos	dword [edi]
	stos	dword [edi]
	mov	eax,[esi+10h]
	call	ta_store_elf_machine_word
	xor	eax,eax
	call	ta_store_elf_machine_word
	inc	ecx
	add	esi,20h
	xchg	edi,[esp]
	mov	ebp,edi
      ta_convert_relocations:
	cmp	esi,[ta_free_additional_memory]
	je	ta_relocations_converted
	mov	al,[esi]
	or	al,al
	jz	ta_relocations_converted
	cmp	al,80h
	jb	ta_make_relocation_entry
	cmp	al,0C0h
	jb	ta_relocation_entry_ok
	add	esi,10h
	jmp	ta_convert_relocations
      ta_make_relocation_entry:
	test	[ta_format_flags],8
	jnz	ta_make_elf64_relocation_entry
	mov	eax,[esi+4]
	stos	dword [edi]
	mov	eax,[esi+8]
	mov	eax,[eax]
	mov	al,[esi]
	stos	dword [edi]
	jmp	ta_relocation_entry_ok
      ta_make_elf64_relocation_entry:
	mov	eax,[esi+4]
	stos	dword [edi]
	xor	eax,eax
	stos	dword [edi]
	movzx	eax,byte [esi]
	stos	dword [edi]
	mov	eax,[esi+8]
	mov	eax,[eax]
	shr	eax,8
	stos	dword [edi]
	xor	eax,eax
	push	edx
	mov	edx,[esi+4]
	add	edx,[ta_image_base]
	xchg	eax,[edx]
	stos	dword [edi]
	cmp	byte [esi],1
	je	ta_addend_64bit
	pop	edx
	sar	eax,31
	stos	dword [edi]
	jmp	ta_relocation_entry_ok
      ta_addend_64bit:
	xor	eax,eax
	xchg	eax,[edx+4]
	stos	dword [edi]
	pop	edx
      ta_relocation_entry_ok:
	add	esi,0Ch
	jmp	ta_convert_relocations
      ta_store_elf_machine_word:
	stos	dword [edi]
	test	[ta_format_flags],8
	jz	ta_elf_machine_word_ok
	and	dword [edi],0
	add	edi,4
      ta_elf_machine_word_ok:
	ret
      ta_relocations_converted:
	cmp	edi,ebp
	xchg	edi,[esp]
	je	ta_rel_section_ok
	mov	eax,[ebx]
	sub	eax,4
	test	[ta_format_flags],8
	jz	ta_store_relocations_name_offset
	dec	eax
      ta_store_relocations_name_offset:
	stos	dword [edi]
	test	[ta_format_flags],8
	jnz	ta_rela_section
	mov	eax,9
	jmp	ta_store_relocations_type
      ta_rela_section:
	mov	eax,4
      ta_store_relocations_type:
	stos	dword [edi]
	xor	al,al
	call	ta_store_elf_machine_word
	call	ta_store_elf_machine_word
	mov	eax,ebp
	sub	eax,[ta_code_start]
	call	ta_store_elf_machine_word
	mov	eax,[esp]
	sub	eax,ebp
	call	ta_store_elf_machine_word
	mov	eax,[ta_current_section]
	stos	dword [edi]
	mov	eax,ecx
	stos	dword [edi]
	inc	ecx
	test	[ta_format_flags],8
	jnz	ta_finish_elf64_rela_section
	mov	eax,4
	stos	dword [edi]
	mov	al,8
	stos	dword [edi]
	jmp	ta_rel_section_ok
      ta_finish_elf64_rela_section:
	mov	eax,8
	stos	dword [edi]
	xor	al,al
	stos	dword [edi]
	mov	al,24
	stos	dword [edi]
	xor	al,al
	stos	dword [edi]
      ta_rel_section_ok:
	cmp	esi,[ta_free_additional_memory]
	jne	ta_make_section_entry
	pop	eax
	mov	ebx,[ta_code_start]
	sub	eax,ebx
	mov	[ta_code_size],eax
	mov	ecx,20h
	test	[ta_format_flags],8
	jz	ta_adjust_elf_section_headers_offset
	mov	ecx,28h
      ta_adjust_elf_section_headers_offset:
	add	[ebx+ecx],eax
	mov	eax,1
	stos	dword [edi]
	mov	al,2
	stos	dword [edi]
	xor	al,al
	call	ta_store_elf_machine_word
	call	ta_store_elf_machine_word
	mov	eax,[ta_code_size]
	call	ta_store_elf_machine_word
	mov	eax,[edx+1]
	sub	eax,[ta_free_additional_memory]
	call	ta_store_elf_machine_word
	mov	eax,[ta_current_section]
	inc	eax
	stos	dword [edi]
	mov	eax,[ta_number_of_sections]
	inc	eax
	stos	dword [edi]
	test	[ta_format_flags],8
	jnz	ta_finish_elf64_sym_section
	mov	eax,4
	stos	dword [edi]
	mov	al,10h
	stos	dword [edi]
	jmp	ta_sym_section_ok
      ta_finish_elf64_sym_section:
	mov	eax,8
	stos	dword [edi]
	xor	al,al
	stos	dword [edi]
	mov	al,18h
	stos	dword [edi]
	xor	al,al
	stos	dword [edi]
      ta_sym_section_ok:
	mov	al,1+8
	stos	dword [edi]
	mov	al,3
	stos	dword [edi]
	xor	al,al
	call	ta_store_elf_machine_word
	call	ta_store_elf_machine_word
	mov	eax,[edx+1]
	sub	eax,[ta_free_additional_memory]
	add	eax,[ta_code_size]
	call	ta_store_elf_machine_word
	mov	eax,[edx+1+8]
	sub	eax,[edx+1]
	call	ta_store_elf_machine_word
	xor	eax,eax
	stos	dword [edi]
	stos	dword [edi]
	mov	al,1
	call	ta_store_elf_machine_word
	xor	eax,eax
	call	ta_store_elf_machine_word
	mov	eax,'tab'
	mov	dword [edx+1],'.sym'
	mov	[edx+1+4],eax
	mov	dword [edx+1+8],'.str'
	mov	[edx+1+8+4],eax
	mov	[ta_resource_data],edx
	mov	[ta_written_size],0
	mov	edx,[ta_output_file]
	call	ta_create
	jc	ta_write_failed
	call	ta_write_code
	mov	ecx,edi
	mov	edx,[ta_free_additional_memory]
	sub	ecx,edx
	add	[ta_written_size],ecx
	call	ta_write
	jc	ta_write_failed
	jmp	ta_output_written

ta_format_elf_exe:
	add	esi,2
	or	[ta_format_flags],1
	cmp	byte [esi],'('
	jne	ta_elf_exe_brand_ok
	inc	esi
	cmp	byte [esi],'.'
	je	ta_invalid_value
	push	edx
	call	ta_get_byte_value
	cmp	[ta_value_type],0
	jne	ta_invalid_use_of_symbol
	pop	edx
	mov	[edx+7],al
      ta_elf_exe_brand_ok:
	mov	[ta_image_base],8048000h
	cmp	byte [esi],80h
	jne	ta_elf_exe_base_ok
	lods	word [esi]
	cmp	ah,'('
	jne	ta_invalid_argument
	cmp	byte [esi],'.'
	je	ta_invalid_value
	push	edx
	call	ta_get_dword_value
	cmp	[ta_value_type],0
	jne	ta_invalid_use_of_symbol
	mov	[ta_image_base],eax
	pop	edx
      ta_elf_exe_base_ok:
	mov	byte [edx+2Ah],20h
	mov	ebx,edi
	mov	ecx,20h shr 2
	cmp	[ta_current_pass],0
	je	ta_init_elf_segments
	imul	ecx,[ta_number_of_sections]
      ta_init_elf_segments:
	xor	eax,eax
	rep	stos dword [edi]
	and	[ta_number_of_sections],0
	mov	byte [ebx],1
	mov	word [ebx+1Ch],1000h
	mov	byte [ebx+18h],111b
	mov	ebp,[ta_image_base]
	and	dword [ebx+4],0
	mov	[ebx+8],ebp
	mov	[ebx+0Ch],ebp
	mov	eax,edi
	sub	eax,[ta_code_start]
	add	eax,ebp
	mov	[edx+18h],eax
	and	[ta_image_base_high],0
      ta_elf_exe_addressing_setup:
	call	ta_init_addressing_space
	call	ta_setup_elf_exe_labels_type
	mov	eax,[ta_code_start]
	xor	edx,edx
	xor	cl,cl
	sub	eax,[ta_image_base]
	sbb	edx,[ta_image_base_high]
	sbb	cl,0
	mov	[ebx],eax
	mov	[ebx+4],edx
	mov	[ebx+8],cl
	mov	[ta_symbols_stream],edi
	jmp	ta_format_defined
      ta_format_elf64_exe:
	add	esi,2
	or	[ta_format_flags],1
	cmp	byte [esi],'('
	jne	ta_elf64_exe_brand_ok
	inc	esi
	cmp	byte [esi],'.'
	je	ta_invalid_value
	push	edx
	call	ta_get_byte_value
	cmp	[ta_value_type],0
	jne	ta_invalid_use_of_symbol
	pop	edx
	mov	[edx+7],al
      ta_elf64_exe_brand_ok:
	mov	[ta_image_base],400000h
	and	[ta_image_base_high],0
	cmp	byte [esi],80h
	jne	ta_elf64_exe_base_ok
	lods	word [esi]
	cmp	ah,'('
	jne	ta_invalid_argument
	cmp	byte [esi],'.'
	je	ta_invalid_value
	push	edx
	call	ta_get_qword_value
	cmp	[ta_value_type],0
	jne	ta_invalid_use_of_symbol
	mov	[ta_image_base],eax
	mov	[ta_image_base_high],edx
	pop	edx
      ta_elf64_exe_base_ok:
	mov	byte [edx+36h],38h
	mov	ebx,edi
	mov	ecx,38h shr 2
	cmp	[ta_current_pass],0
	je	ta_init_elf64_segments
	imul	ecx,[ta_number_of_sections]
      ta_init_elf64_segments:
	xor	eax,eax
	rep	stos dword [edi]
	and	[ta_number_of_sections],0
	mov	byte [ebx],1
	mov	word [ebx+30h],1000h
	mov	byte [ebx+4],111b
	mov	ebp,[ta_image_base]
	mov	ecx,[ta_image_base_high]
	and	dword [ebx+8],0
	mov	[ebx+10h],ebp
	mov	[ebx+10h+4],ecx
	mov	[ebx+18h],ebp
	mov	[ebx+18h+4],ecx
	mov	eax,edi
	sub	eax,[ta_code_start]
	add	eax,ebp
	adc	ecx,0
	mov	[edx+18h],eax
	mov	[edx+18h+4],ecx
	jmp	ta_elf_exe_addressing_setup
      ta_setup_elf_exe_labels_type:
	mov	eax,[ta_code_start]
	cmp	byte [eax+10h],3
	jne	ta_elf_exe_labels_type_ok
	mov	byte [ebx+9],2
	test	[ta_format_flags],8
	jz	ta_elf_exe_labels_type_ok
	mov	byte [ebx+9],4
      ta_elf_exe_labels_type_ok:
	ret
ta_elf_entry:
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_argument
	cmp	byte [esi],'.'
	je	ta_invalid_value
	test	[ta_format_flags],8
	jnz	ta_elf64_entry
	call	ta_get_dword_value
	mov	edx,[ta_code_start]
	mov	[edx+18h],eax
	jmp	ta_instruction_assembled
      ta_elf64_entry:
	call	ta_get_qword_value
	mov	ebx,[ta_code_start]
	mov	[ebx+18h],eax
	mov	[ebx+1Ch],edx
	jmp	ta_instruction_assembled
ta_elf_segment:
	bt	[ta_format_flags],0
	jnc	ta_illegal_instruction
	test	[ta_format_flags],8
	jnz	ta_elf64_segment
	call	ta_close_elf_segment
	push	eax
	call	ta_create_addressing_space
	call	ta_setup_elf_exe_labels_type
	mov	ebp,ebx
	mov	ebx,[ta_number_of_sections]
	shl	ebx,5
	add	ebx,[ta_code_start]
	add	ebx,34h
	cmp	ebx,[ta_symbols_stream]
	jb	ta_new_elf_segment
	mov	ebx,[ta_symbols_stream]
	sub	ebx,20h
	or	[ta_next_pass_needed],-1
      ta_new_elf_segment:
	mov	byte [ebx],1
	and	dword [ebx+18h],0
	mov	word [ebx+1Ch],1000h
      ta_elf_segment_flags:
	cmp	byte [esi],1Eh
	je	ta_elf_segment_type
	cmp	byte [esi],19h
	jne	ta_elf_segment_flags_ok
	lods	word [esi]
	sub	ah,28
	jbe	ta_invalid_argument
	cmp	ah,1
	je	ta_mark_elf_segment_flag
	cmp	ah,3
	ja	ta_invalid_argument
	xor	ah,1
	cmp	ah,2
	je	ta_mark_elf_segment_flag
	inc	ah
      ta_mark_elf_segment_flag:
	test	[ebx+18h],ah
	jnz	ta_setting_already_specified
	or	[ebx+18h],ah
	jmp	ta_elf_segment_flags
      ta_elf_segment_type:
	cmp	byte [ebx],1
	jne	ta_setting_already_specified
	lods	word [esi]
	mov	ecx,[ta_number_of_sections]
	jecxz	ta_elf_segment_type_ok
	mov	edx,[ta_code_start]
	add	edx,34h
      ta_scan_elf_segment_types:
	cmp	edx,[ta_symbols_stream]
	jae	ta_elf_segment_type_ok
	cmp	[edx],ah
	je	ta_data_already_defined
	add	edx,20h
	loop	ta_scan_elf_segment_types
      ta_elf_segment_type_ok:
	mov	[ebx],ah
	mov	word [ebx+1Ch],1
	cmp	ah,50h
	jb	ta_elf_segment_flags
	or	dword [ebx],6474E500h
	jmp	ta_elf_segment_flags
      ta_elf_segment_flags_ok:
	pop	edx
	cmp	byte [ebx],1
	jne	ta_no_elf_segment_merging
	cmp	[ta_merge_segment],0
	jne	ta_merge_elf_segment
      ta_no_elf_segment_merging:
	mov	eax,edi
	sub	eax,[ta_code_start]
	mov	[ebx+4],eax
	and	eax,0FFFh
	add	eax,edx
	mov	[ebx+8],eax
	mov	[ebx+0Ch],eax
	xor	edx,edx
      ta_elf_segment_addressing_setup:
	xor	cl,cl
	not	eax
	not	edx
	not	cl
	add	eax,1
	adc	edx,0
	adc	cl,0
	add	eax,edi
	adc	edx,0
	adc	cl,0
	mov	[ds:ebp],eax
	mov	[ds:ebp+4],edx
	mov	[ds:ebp+8],cl
	inc	[ta_number_of_sections]
	jmp	ta_instruction_assembled
      ta_merge_elf_segment:
	xor	ecx,ecx
	xchg	ecx,[ta_merge_segment]
	cmp	ecx,-1
	je	ta_merge_elf_header
	mov	eax,[ecx+8]
	mov	ecx,[ecx+4]
      ta_elf_segment_separated_base:
	mov	[ebx+8],eax
	mov	[ebx+0Ch],eax
	mov	[ebx+4],ecx
	sub	eax,ecx
	add	eax,edi
	sub	eax,[ta_code_start]
	xor	edx,edx
	jmp	ta_elf_segment_addressing_setup
      ta_merge_elf_header:
	mov	eax,[ta_image_base]
	xor	ecx,ecx
	jmp	ta_elf_segment_separated_base
      ta_close_elf_segment:
	cmp	[ta_number_of_sections],0
	jne	ta_finish_elf_segment
	cmp	edi,[ta_symbols_stream]
	jne	ta_first_elf_segment_ok
	or	[ta_merge_segment],-1
	mov	eax,[ta_image_base]
	ret
      ta_first_elf_segment_ok:
	and	[ta_merge_segment],0
	inc	[ta_number_of_sections]
      ta_finish_elf_segment:
	mov	ebx,[ta_number_of_sections]
	dec	ebx
	shl	ebx,5
	add	ebx,[ta_code_start]
	add	ebx,34h
	mov	eax,edi
	sub	eax,[ta_code_start]
	sub	eax,[ebx+4]
	mov	edx,edi
	cmp	edi,[ta_undefined_data_end]
	jne	ta_elf_segment_size_ok
	cmp	byte [ebx],1
	jne	ta_elf_segment_size_ok
	mov	edi,[ta_undefined_data_start]
      ta_elf_segment_size_ok:
	mov	[ebx+14h],eax
	add	eax,edi
	sub	eax,edx
	mov	[ebx+10h],eax
	and	[ta_undefined_data_end],0
	mov	eax,[ebx+8]
	cmp	byte [ebx],1
	je	ta_elf_segment_position_move_and_align
	cmp	[ta_merge_segment],0
	jne	ta_elf_segment_position_move
	cmp	byte [ebx],4
	je	ta_elf_segment_position_ok
	cmp	byte [ebx],51h
	je	ta_elf_segment_position_ok
	mov	[ta_merge_segment],ebx
      ta_elf_segment_position_move:
	add	eax,[ebx+14h]
	jmp	ta_elf_segment_position_ok
      ta_elf_segment_position_move_and_align:
	add	eax,[ebx+14h]
	add	eax,0FFFh
      ta_elf_segment_position_ok:
	and	eax,not 0FFFh
	ret
      ta_elf64_segment:
	call	ta_close_elf64_segment
	push	eax edx
	call	ta_create_addressing_space
	call	ta_setup_elf_exe_labels_type
	mov	ebp,ebx
	mov	ebx,[ta_number_of_sections]
	imul	ebx,38h
	add	ebx,[ta_code_start]
	add	ebx,40h
	cmp	ebx,[ta_symbols_stream]
	jb	ta_new_elf64_segment
	or	[ta_next_pass_needed],-1
      ta_new_elf64_segment:
	mov	byte [ebx],1
	and	dword [ebx+4],0
	mov	word [ebx+30h],1000h
      ta_elf64_segment_flags:
	cmp	byte [esi],1Eh
	je	ta_elf64_segment_type
	cmp	byte [esi],19h
	jne	ta_elf64_segment_flags_ok
	lods	word [esi]
	sub	ah,28
	jbe	ta_invalid_argument
	cmp	ah,1
	je	ta_mark_elf64_segment_flag
	cmp	ah,3
	ja	ta_invalid_argument
	xor	ah,1
	cmp	ah,2
	je	ta_mark_elf64_segment_flag
	inc	ah
      ta_mark_elf64_segment_flag:
	test	[ebx+4],ah
	jnz	ta_setting_already_specified
	or	[ebx+4],ah
	jmp	ta_elf64_segment_flags
      ta_elf64_segment_type:
	cmp	byte [ebx],1
	jne	ta_setting_already_specified
	lods	word [esi]
	mov	ecx,[ta_number_of_sections]
	jecxz	ta_elf64_segment_type_ok
	mov	edx,[ta_code_start]
	add	edx,40h
      ta_scan_elf64_segment_types:
	cmp	edx,[ta_symbols_stream]
	jae	ta_elf64_segment_type_ok
	cmp	[edx],ah
	je	ta_data_already_defined
	add	edx,38h
	loop	ta_scan_elf64_segment_types
      ta_elf64_segment_type_ok:
	mov	[ebx],ah
	mov	word [ebx+30h],1
	cmp	ah,50h
	jb	ta_elf64_segment_flags
	or	dword [ebx],6474E500h
	jmp	ta_elf64_segment_flags
      ta_elf64_segment_flags_ok:
	pop	edx eax
	cmp	byte [ebx],1
	jne	ta_no_elf64_segment_merging
	cmp	[ta_merge_segment],0
	jne	ta_merge_elf64_segment
      ta_no_elf64_segment_merging:
	mov	ecx,edi
	sub	ecx,[ta_code_start]
	mov	[ebx+8],ecx
	and	ecx,0FFFh
	add	eax,ecx
	adc	edx,0
	mov	[ebx+10h],eax
	mov	[ebx+10h+4],edx
	mov	[ebx+18h],eax
	mov	[ebx+18h+4],edx
	jmp	ta_elf_segment_addressing_setup
      ta_merge_elf64_segment:
	xor	ecx,ecx
	xchg	ecx,[ta_merge_segment]
	cmp	ecx,-1
	je	ta_merge_elf64_header
	mov	eax,[ecx+10h]
	mov	edx,[ecx+10h+4]
	mov	ecx,[ecx+8]
      ta_elf64_segment_separated_base:
	mov	[ebx+10h],eax
	mov	[ebx+10h+4],edx
	mov	[ebx+18h],eax
	mov	[ebx+18h+4],edx
	mov	[ebx+8],ecx
	neg	ecx
	add	ecx,edi
	sub	ecx,[ta_code_start]
	add	eax,ecx
	adc	edx,0
	jmp	ta_elf_segment_addressing_setup
      ta_merge_elf64_header:
	mov	eax,[ta_image_base]
	mov	edx,[ta_image_base_high]
	xor	ecx,ecx
	jmp	ta_elf64_segment_separated_base
      ta_close_elf64_segment:
	cmp	[ta_number_of_sections],0
	jne	ta_finish_elf64_segment
	cmp	edi,[ta_symbols_stream]
	jne	ta_first_elf64_segment_ok
	or	[ta_merge_segment],-1
	mov	eax,[ta_image_base]
	mov	edx,[ta_image_base_high]
	ret
      ta_first_elf64_segment_ok:
	and	[ta_merge_segment],0
	inc	[ta_number_of_sections]
      ta_finish_elf64_segment:
	mov	ebx,[ta_number_of_sections]
	dec	ebx
	imul	ebx,38h
	add	ebx,[ta_code_start]
	add	ebx,40h
	mov	eax,edi
	sub	eax,[ta_code_start]
	sub	eax,[ebx+8]
	mov	edx,edi
	cmp	edi,[ta_undefined_data_end]
	jne	ta_elf64_segment_size_ok
	cmp	byte [ebx],1
	jne	ta_elf64_segment_size_ok
	mov	edi,[ta_undefined_data_start]
      ta_elf64_segment_size_ok:
	mov	[ebx+28h],eax
	add	eax,edi
	sub	eax,edx
	mov	[ebx+20h],eax
	and	[ta_undefined_data_end],0
	mov	eax,[ebx+10h]
	mov	edx,[ebx+10h+4]
	cmp	byte [ebx],1
	je	ta_elf64_segment_position_move_and_align
	cmp	[ta_merge_segment],0
	jne	ta_elf64_segment_position_move
	cmp	byte [ebx],4
	je	ta_elf64_segment_position_ok
	cmp	byte [ebx],51h
	je	ta_elf64_segment_position_ok
	mov	[ta_merge_segment],ebx
      ta_elf64_segment_position_move:
	add	eax,[ebx+28h]
	adc	edx,0
	jmp	ta_elf64_segment_position_ok
      ta_elf64_segment_position_move_and_align:
	add	eax,[ebx+28h]
	adc	edx,0
	add	eax,0FFFh
	adc	edx,0
      ta_elf64_segment_position_ok:
	and	eax,not 0FFFh
	ret
ta_close_elf_exe:
	test	[ta_format_flags],8
	jnz	ta_close_elf64_exe
	call	ta_close_elf_segment
	mov	edx,[ta_code_start]
	mov	eax,[ta_number_of_sections]
	mov	byte [edx+1Ch],34h
	mov	[edx+2Ch],ax
	shl	eax,5
	add	eax,edx
	add	eax,34h
	cmp	eax,[ta_symbols_stream]
	je	ta_elf_exe_ok
	or	[ta_next_pass_needed],-1
      ta_elf_exe_ok:
	ret
      ta_close_elf64_exe:
	call	ta_close_elf64_segment
	mov	edx,[ta_code_start]
	mov	eax,[ta_number_of_sections]
	mov	byte [edx+20h],40h
	mov	[edx+38h],ax
	imul	eax,38h
	add	eax,edx
	add	eax,40h
	cmp	eax,[ta_symbols_stream]
	je	ta_elf64_exe_ok
	or	[ta_next_pass_needed],-1
      ta_elf64_exe_ok:
	ret
