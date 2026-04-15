ta_avx_single_source_pd_instruction_er_evex:
	or	[ta_vex_required],8
ta_avx_single_source_pd_instruction_er:
	or	[ta_operand_flags],2+4+8
	jmp	ta_avx_pd_instruction
ta_avx_single_source_pd_instruction_sae_evex:
	or	[ta_vex_required],8
	or	[ta_operand_flags],2+4
	jmp	ta_avx_pd_instruction
ta_avx_pd_instruction_imm8:
	mov	[ta_immediate_size],1
	jmp	ta_avx_pd_instruction
ta_avx_pd_instruction_er:
	or	[ta_operand_flags],8
ta_avx_pd_instruction_sae:
	or	[ta_operand_flags],4
ta_avx_pd_instruction:
	mov	[ta_opcode_prefix],66h
	or	[ta_rex_prefix],80h
	mov	cx,0800h
	jmp	ta_avx_instruction_with_broadcast
ta_avx_pd_instruction_38_evex:
	or	[ta_vex_required],8
	mov	[ta_supplemental_code],al
	mov	al,38h
	jmp	ta_avx_pd_instruction
ta_avx_cvtps2dq_instruction:
	mov	[ta_opcode_prefix],66h
	jmp	ta_avx_single_source_ps_instruction_er
ta_avx_cvtudq2ps_instruction:
	mov	[ta_opcode_prefix],0F2h
ta_avx_single_source_ps_instruction_er_evex:
	or	[ta_vex_required],8
ta_avx_single_source_ps_instruction_er:
	or	[ta_operand_flags],2+4+8
	jmp	ta_avx_ps_instruction
ta_avx_single_source_ps_instruction_noevex:
	or	[ta_operand_flags],2
	or	[ta_vex_required],2
	jmp	ta_avx_ps_instruction
ta_avx_ps_instruction_imm8:
	mov	[ta_immediate_size],1
	jmp	ta_avx_ps_instruction
ta_avx_ps_instruction_er:
	or	[ta_operand_flags],8
ta_avx_ps_instruction_sae:
	or	[ta_operand_flags],4
ta_avx_ps_instruction:
	mov	cx,0400h
	jmp	ta_avx_instruction_with_broadcast
ta_avx_ps_instruction_66_38_evex:
	or	[ta_vex_required],8
	mov	[ta_opcode_prefix],66h
	mov	[ta_supplemental_code],al
	mov	al,38h
	jmp	ta_avx_ps_instruction
ta_avx_sd_instruction_er:
	or	[ta_operand_flags],8
ta_avx_sd_instruction_sae:
	or	[ta_operand_flags],4
ta_avx_sd_instruction:
	mov	[ta_opcode_prefix],0F2h
	or	[ta_rex_prefix],80h
	mov	cl,8
	jmp	ta_avx_instruction
ta_avx_ss_instruction_er:
	or	[ta_operand_flags],8
ta_avx_ss_instruction_sae:
	or	[ta_operand_flags],4
ta_avx_ss_instruction:
	mov	[ta_opcode_prefix],0F3h
	mov	cl,4
	jmp	ta_avx_instruction
ta_avx_ss_instruction_noevex:
	or	[ta_vex_required],2
	jmp	ta_avx_ss_instruction
ta_avx_single_source_q_instruction_38_evex:
	or	[ta_operand_flags],2
ta_avx_q_instruction_38_evex:
	or	[ta_vex_required],8
ta_avx_q_instruction_38:
	mov	[ta_supplemental_code],al
	mov	al,38h
	jmp	ta_avx_q_instruction
ta_avx_q_instruction_38_w1_evex:
	or	[ta_vex_required],8
ta_avx_q_instruction_38_w1:
	or	[ta_rex_prefix],8
	jmp	ta_avx_q_instruction_38
ta_avx_q_instruction_3a_imm8_w1:
	or	[ta_rex_prefix],8
	jmp	ta_avx_q_instruction_3a_imm8
ta_avx_q_instruction_3a_imm8_evex:
	or	[ta_vex_required],8
ta_avx_q_instruction_3a_imm8:
	mov	[ta_immediate_size],1
	mov	[ta_supplemental_code],al
	mov	al,3Ah
	jmp	ta_avx_q_instruction
ta_avx_q_instruction_evex:
	or	[ta_vex_required],8
ta_avx_q_instruction:
	or	[ta_rex_prefix],80h
	mov	ch,8
	jmp	ta_avx_pi_instruction
ta_avx_single_source_d_instruction_38_evex_w1:
	or	[ta_rex_prefix],8
ta_avx_single_source_d_instruction_38_evex:
	or	[ta_vex_required],8
ta_avx_single_source_d_instruction_38:
	or	[ta_operand_flags],2
	jmp	ta_avx_d_instruction_38
ta_avx_d_instruction_38_evex:
	or	[ta_vex_required],8
ta_avx_d_instruction_38:
	mov	[ta_supplemental_code],al
	mov	al,38h
	jmp	ta_avx_d_instruction
ta_avx_d_instruction_3a_imm8_evex:
	mov	[ta_immediate_size],1
	or	[ta_vex_required],8
	mov	[ta_supplemental_code],al
	mov	al,3Ah
	jmp	ta_avx_d_instruction
ta_avx_single_source_d_instruction_imm8:
	or	[ta_operand_flags],2
	mov	[ta_immediate_size],1
	jmp	ta_avx_d_instruction
ta_avx_d_instruction_evex:
	or	[ta_vex_required],8
ta_avx_d_instruction:
	mov	ch,4
	jmp	ta_avx_pi_instruction
ta_avx_bw_instruction_3a_imm8_w1_evex:
	or	[ta_rex_prefix],8
ta_avx_bw_instruction_3a_imm8_evex:
	mov	[ta_immediate_size],1
	or	[ta_vex_required],8
	mov	[ta_supplemental_code],al
	mov	al,3Ah
	jmp	ta_avx_bw_instruction
ta_avx_single_source_bw_instruction_38:
	or	[ta_operand_flags],2
ta_avx_bw_instruction_38:
	mov	[ta_supplemental_code],al
	mov	al,38h
ta_avx_bw_instruction:
	xor	ch,ch
      ta_avx_pi_instruction:
	mov	[ta_opcode_prefix],66h
	xor	cl,cl
	jmp	ta_avx_instruction_with_broadcast
ta_avx_bw_instruction_38_w1_evex:
	or	[ta_rex_prefix],8
ta_avx_bw_instruction_38_evex:
	or	[ta_vex_required],8
	jmp	ta_avx_bw_instruction_38
ta_avx_pd_instruction_noevex:
	xor	cl,cl
	or	[ta_vex_required],2
	mov	[ta_opcode_prefix],66h
	jmp	ta_avx_instruction
ta_avx_ps_instruction_noevex:
	or	[ta_vex_required],2
	mov	[ta_opcode_prefix],0F2h
	xor	cl,cl
	jmp	ta_avx_instruction
ta_avx_instruction:
	xor	ch,ch
      ta_avx_instruction_with_broadcast:
	mov	[ta_mmx_size],cl
	mov	[ta_broadcast_size],ch
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
      ta_avx_xop_common:
	or	[ta_vex_required],1
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
      ta_avx_reg:
	lods	byte [esi]
	call	ta_convert_avx_register
	mov	[ta_postbyte_register],al
	call	ta_take_avx512_mask
      ta_avx_vex_reg:
	test	[ta_operand_flags],2
	jnz	ta_avx_vex_reg_ok
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_avx_register
	mov	[ta_vex_register],al
      ta_avx_vex_reg_ok:
	mov	al,[ta_mmx_size]
	or	al,al
	jz	ta_avx_regs_size_ok
	mov	ah,[ta_operand_size]
	or	ah,ah
	jz	ta_avx_regs_size_ok
	cmp	al,ah
	je	ta_avx_regs_size_ok
	ja	ta_invalid_operand_size
	cmp	ah,16
	jne	ta_invalid_operand_size
      ta_avx_regs_size_ok:
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
      ta_avx_regs_rm:
	call	ta_take_avx_rm
	jc	ta_avx_regs_reg
	mov	al,[ta_immediate_size]
	cmp	al,1
	je	ta_mmx_imm8
	jb	ta_instruction_ready
	cmp	al,-4
	je	ta_sse_cmp_mem_ok
	cmp	byte [esi],','
	jne	ta_invalid_operand
	inc	esi
	call	ta_take_avx_register
	shl	al,4
	jc	ta_invalid_operand
	or	byte [ta_value],al
	test	al,80h
	jz	ta_avx_regs_mem_reg_store
	cmp	[ta_code_type],64
	jne	ta_invalid_operand
      ta_avx_regs_mem_reg_store:
	call	ta_take_imm4_if_needed
	call	ta_store_instruction_with_imm8
	jmp	ta_instruction_assembled
      ta_avx_regs_reg:
	mov	bl,al
	call	ta_take_avx512_rounding
	mov	al,[ta_immediate_size]
	cmp	al,1
	je	ta_mmx_nomem_imm8
	jb	ta_nomem_instruction_ready
	cmp	al,-4
	je	ta_sse_cmp_nomem_ok
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	mov	al,bl
	shl	al,4
	jc	ta_invalid_operand
	or	byte [ta_value],al
	test	al,80h
	jz	ta_avx_regs_reg_
	cmp	[ta_code_type],64
	jne	ta_invalid_operand
      ta_avx_regs_reg_:
	call	ta_take_avx_rm
	jc	ta_avx_regs_reg_reg
	cmp	[ta_immediate_size],-2
	jg	ta_invalid_operand
	or	[ta_rex_prefix],8
	call	ta_take_imm4_if_needed
	call	ta_store_instruction_with_imm8
	jmp	ta_instruction_assembled
      ta_avx_regs_reg_reg:
	shl	al,4
	jc	ta_invalid_operand
	and	byte [ta_value],1111b
	or	byte [ta_value],al
	call	ta_take_imm4_if_needed
	call	ta_store_nomem_instruction
	mov	al,byte [ta_value]
	stos	byte [edi]
	jmp	ta_instruction_assembled
      ta_take_avx_rm:
	xor	cl,cl
	xchg	cl,[ta_operand_size]
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	je	ta_take_avx_mem
	cmp	al,10h
	jne	ta_invalid_operand
	mov	[ta_operand_size],cl
	lods	byte [esi]
	call	ta_convert_avx_register
	or	cl,cl
	jnz	ta_avx_reg_ok
	or	cl,[ta_mmx_size]
	jz	ta_avx_reg_ok
	cmp	ah,cl
	je	ta_avx_reg_ok
	jb	ta_invalid_operand_size
	cmp	ah,16
	jne	ta_invalid_operand_size
      ta_avx_reg_ok:
	stc
	ret
      ta_take_avx_mem:
	push	ecx
	call	ta_get_address
	cmp	byte [esi],'{'
	jne	ta_avx_mem_ok
	inc	esi
	lods	byte [esi]
	cmp	al,1Fh
	jne	ta_invalid_operand
	mov	al,[esi]
	shr	al,4
	cmp	al,1
	jne	ta_invalid_operand
	mov	al,[ta_mmx_size]
	or	al,al
	jnz	ta_avx_mem_broadcast_check
	mov	eax,[esp]
	or	al,al
	jnz	ta_avx_mem_broadcast_check
	mov	al,[ta_broadcast_size]
	mov	[ta_mmx_size],al
	mov	ah,cl
	lods	byte [esi]
	and	al,1111b
	mov	cl,al
	mov	al,[ta_broadcast_size]
	shl	al,cl
	mov	[esp],al
	mov	cl,ah
	jmp	ta_avx_mem_broadcast_ok
      ta_avx_mem_broadcast_check:
	bsf	eax,eax
	xchg	al,[ta_broadcast_size]
	mov	[ta_mmx_size],al
	bsf	eax,eax
	jz	ta_invalid_operand
	mov	ah,[ta_broadcast_size]
	sub	ah,al
	lods	byte [esi]
	and	al,1111b
	cmp	al,ah
	jne	ta_invalid_operand_size
      ta_avx_mem_broadcast_ok:
	or	[ta_vex_required],40h
	lods	byte [esi]
	cmp	al,'}'
	jne	ta_invalid_operand
      ta_avx_mem_ok:
	pop	eax
	or	al,al
	jz	ta_avx_mem_size_deciding
	xchg	al,[ta_operand_size]
	cmp	[ta_mmx_size],0
	jne	ta_avx_mem_size_enforced
	or	al,al
	jz	ta_avx_mem_size_ok
	cmp	al,[ta_operand_size]
	jne	ta_operand_sizes_do_not_match
      ta_avx_mem_size_ok:
	clc
	ret
      ta_avx_mem_size_deciding:
	mov	al,[ta_operand_size]
	cmp	[ta_mmx_size],0
	jne	ta_avx_mem_size_enforced
	cmp	al,16
	je	ta_avx_mem_size_ok
	cmp	al,32
	je	ta_avx_mem_size_ok
	cmp	al,64
	je	ta_avx_mem_size_ok
	or	al,al
	jnz	ta_invalid_operand_size
	call	ta_recoverable_unknown_size
      ta_avx_mem_size_enforced:
	or	al,al
	jz	ta_avx_mem_size_ok
	cmp	al,[ta_mmx_size]
	je	ta_avx_mem_size_ok
	jmp	ta_invalid_operand_size
      ta_take_imm4_if_needed:
	cmp	[ta_immediate_size],-3
	jne	ta_imm4_ok
	push	ebx ecx edx
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	cmp	al,'('
	jne	ta_invalid_operand
	call	ta_get_byte_value
	test	al,11110000b
	jnz	ta_value_out_of_range
	or	byte [ta_value],al
	pop	edx ecx ebx
      ta_imm4_ok:
	ret
      ta_take_avx512_mask:
	cmp	byte [esi],'{'
	jne	ta_avx512_masking_ok
	test	[ta_operand_flags],10h
	jnz	ta_invalid_operand
	inc	esi
	lods	byte [esi]
	cmp	al,14h
	jne	ta_invalid_operand
	lods	byte [esi]
	mov	ah,al
	shr	ah,4
	cmp	ah,5
	jne	ta_invalid_operand
	and	al,111b
	or	al,al
	jz	ta_invalid_operand
	mov	[ta_mask_register],al
	or	[ta_vex_required],20h
	lods	byte [esi]
	cmp	al,'}'
	jne	ta_invalid_operand
	cmp	byte [esi],'{'
	jne	ta_avx512_masking_ok
	test	[ta_operand_flags],20h
	jnz	ta_invalid_operand
	inc	esi
	lods	byte [esi]
	cmp	al,1Fh
	jne	ta_invalid_operand
	lods	byte [esi]
	or	al,al
	jnz	ta_invalid_operand
	or	[ta_mask_register],80h
	lods	byte [esi]
	cmp	al,'}'
	jne	ta_invalid_operand
      ta_avx512_masking_ok:
	retn
      ta_take_avx512_rounding:
	test	[ta_operand_flags],4+8
	jz	ta_avx512_rounding_done
	test	[ta_operand_flags],8
	jz	ta_avx512_rounding_allowed
	cmp	[ta_mmx_size],0
	jne	ta_avx512_rounding_allowed
	cmp	[ta_operand_size],64
	jne	ta_avx512_rounding_done
      ta_avx512_rounding_allowed:
	cmp	byte [esi],','
	jne	ta_avx512_rounding_done
	cmp	byte [esi+1],'{'
	jne	ta_avx512_rounding_done
	add	esi,2
	mov	[ta_rounding_mode],0
	or	[ta_vex_required],40h
	test	[ta_operand_flags],8
	jz	ta_take_sae
	or	[ta_vex_required],80h
	lods	byte [esi]
	cmp	al,1Fh
	jne	ta_invalid_operand
	lods	byte [esi]
	mov	ah,al
	shr	ah,4
	cmp	ah,2
	jne	ta_invalid_operand
	and	al,11b
	mov	[ta_rounding_mode],al
	lods	byte [esi]
	cmp	al,'-'
	jne	ta_invalid_operand
      ta_take_sae:
	lods	byte [esi]
	cmp	al,1Fh
	jne	ta_invalid_operand
	lods	byte [esi]
	cmp	al,30h
	jne	ta_invalid_operand
	lods	byte [esi]
	cmp	al,'}'
	jne	ta_invalid_operand
      ta_avx512_rounding_done:
	retn

ta_avx_movdqu_instruction:
	mov	ah,0F3h
	jmp	ta_avx_movdq_instruction
ta_avx_movdqa_instruction:
	mov	ah,66h
      ta_avx_movdq_instruction:
	mov	[ta_opcode_prefix],ah
	or	[ta_vex_required],2
	jmp	ta_avx_movps_instruction
ta_avx512_movdqu16_instruction:
	or	[ta_rex_prefix],8
ta_avx512_movdqu8_instruction:
	mov	ah,0F2h
	jmp	ta_avx_movdq_instruction_evex
ta_avx512_movdqu64_instruction:
	or	[ta_rex_prefix],8
ta_avx512_movdqu32_instruction:
	mov	ah,0F3h
	jmp	ta_avx_movdq_instruction_evex
ta_avx512_movdqa64_instruction:
	or	[ta_rex_prefix],8
ta_avx512_movdqa32_instruction:
	mov	ah,66h
      ta_avx_movdq_instruction_evex:
	mov	[ta_opcode_prefix],ah
	or	[ta_vex_required],8
	jmp	ta_avx_movps_instruction
ta_avx_movpd_instruction:
	mov	[ta_opcode_prefix],66h
	or	[ta_rex_prefix],80h
ta_avx_movps_instruction:
	or	[ta_operand_flags],2
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	or	[ta_vex_required],1
	xor	al,al
	mov	[ta_mmx_size],al
	mov	[ta_broadcast_size],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_avx_reg
	inc	[ta_extended_code]
	test	[ta_extended_code],1
	jnz	ta_avx_mem
	add	[ta_extended_code],-1+10h
      ta_avx_mem:
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	or	[ta_operand_flags],20h
	call	ta_take_avx512_mask
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_avx_register
	mov	[ta_postbyte_register],al
	jmp	ta_instruction_ready
ta_avx_movntpd_instruction:
	or	[ta_rex_prefix],80h
ta_avx_movntdq_instruction:
	mov	[ta_opcode_prefix],66h
ta_avx_movntps_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	or	[ta_vex_required],1
	or	[ta_operand_flags],10h
	mov	[ta_mmx_size],0
	lods	byte [esi]
	call	ta_get_size_operator
	jmp	ta_avx_mem
ta_avx_compress_q_instruction:
	or	[ta_rex_prefix],8
ta_avx_compress_d_instruction:
	or	[ta_vex_required],8
	mov	[ta_mmx_size],0
	call	ta_setup_66_0f_38
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_avx_mem
	lods	byte [esi]
	call	ta_convert_avx_register
	mov	bl,al
	call	ta_take_avx512_mask
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_avx_register
	mov	[ta_postbyte_register],al
	jmp	ta_nomem_instruction_ready
ta_avx_lddqu_instruction:
	mov	ah,0F2h
	or	[ta_vex_required],2
      ta_avx_load_instruction:
	mov	[ta_opcode_prefix],ah
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	mov	[ta_mmx_size],0
	or	[ta_vex_required],1
	call	ta_take_avx_register
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
ta_avx_movntdqa_instruction:
	mov	[ta_supplemental_code],al
	mov	al,38h
	mov	ah,66h
	jmp	ta_avx_load_instruction
ta_avx_movq_instruction:
	or	[ta_rex_prefix],8
	mov	[ta_mmx_size],8
	jmp	ta_avx_mov_instruction
ta_avx_movd_instruction:
	mov	[ta_mmx_size],4
      ta_avx_mov_instruction:
	or	[ta_vex_required],1
	mov	[ta_opcode_prefix],66h
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],7Eh
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_avx_movd_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	mov	al,[ta_mmx_size]
	not	al
	and	[ta_operand_size],al
	jnz	ta_invalid_operand_size
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_avx_register
	cmp	ah,16
	jne	ta_invalid_operand_size
	mov	[ta_postbyte_register],al
	cmp	[ta_mmx_size],8
	jne	ta_instruction_ready
	and	[ta_rex_prefix],not 8
	or	[ta_rex_prefix],80h
	mov	[ta_extended_code],0D6h
	jmp	ta_instruction_ready
      ta_avx_movd_reg:
	lods	byte [esi]
	cmp	al,0C0h
	jae	ta_avx_movd_xmmreg
	call	ta_convert_register
	cmp	ah,[ta_mmx_size]
	jne	ta_invalid_operand_size
	mov	[ta_operand_size],0
	mov	bl,al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_avx_register
	cmp	ah,16
	jne	ta_invalid_operand_size
	mov	[ta_postbyte_register],al
      ta_avx_movd_reg_ready:
	test	[ta_rex_prefix],8
	jz	ta_nomem_instruction_ready
	cmp	[ta_code_type],64
	jne	ta_illegal_instruction
	jmp	ta_nomem_instruction_ready
      ta_avx_movd_xmmreg:
	sub	[ta_extended_code],10h
	call	ta_convert_avx_register
	cmp	ah,16
	jne	ta_invalid_operand_size
	mov	[ta_postbyte_register],al
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_avx_movd_xmmreg_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	mov	al,[ta_mmx_size]
	cmp	al,8
	jne	ta_avx_movd_xmmreg_mem_ready
	call	ta_avx_movq_xmmreg_xmmreg_opcode
      ta_avx_movd_xmmreg_mem_ready:
	not	al
	test	[ta_operand_size],al
	jnz	ta_invalid_operand_size
	jmp	ta_instruction_ready
      ta_avx_movd_xmmreg_reg:
	lods	byte [esi]
	cmp	al,0C0h
	jae	ta_avx_movq_xmmreg_xmmreg
	call	ta_convert_register
	cmp	ah,[ta_mmx_size]
	jne	ta_invalid_operand_size
	mov	bl,al
	jmp	ta_avx_movd_reg_ready
      ta_avx_movq_xmmreg_xmmreg:
	cmp	[ta_mmx_size],8
	jne	ta_invalid_operand
	call	ta_avx_movq_xmmreg_xmmreg_opcode
	call	ta_convert_avx_register
	cmp	ah,16
	jne	ta_invalid_operand_size
	mov	bl,al
	jmp	ta_nomem_instruction_ready
      ta_avx_movq_xmmreg_xmmreg_opcode:
	and	[ta_rex_prefix],not 8
	or	[ta_rex_prefix],80h
	add	[ta_extended_code],10h
	mov	[ta_opcode_prefix],0F3h
	ret
ta_avx_movddup_instruction:
	or	[ta_vex_required],1
	mov	[ta_opcode_prefix],0F2h
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	or	[ta_rex_prefix],80h
	xor	al,al
	mov	[ta_mmx_size],al
	mov	[ta_broadcast_size],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_avx_register
	mov	[ta_postbyte_register],al
	cmp	ah,16
	ja	ta_avx_movddup_size_ok
	mov	[ta_mmx_size],8
      ta_avx_movddup_size_ok:
	call	ta_take_avx512_mask
	jmp	ta_avx_vex_reg_ok
ta_avx_movlpd_instruction:
	mov	[ta_opcode_prefix],66h
	or	[ta_rex_prefix],80h
ta_avx_movlps_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	mov	[ta_mmx_size],8
	mov	[ta_broadcast_size],0
	or	[ta_vex_required],1
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_avx_movlps_mem
	lods	byte [esi]
	call	ta_convert_avx_register
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_avx_register
	mov	[ta_vex_register],al
	cmp	[ta_operand_size],16
	jne	ta_invalid_operand
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_avx_rm
	jc	ta_invalid_operand
	jmp	ta_instruction_ready
      ta_avx_movlps_mem:
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
      ta_avx_movlps_mem_:
	mov	al,[ta_operand_size]
	or	al,al
	jz	ta_avx_movlps_mem_size_ok
	cmp	al,[ta_mmx_size]
	jne	ta_invalid_operand_size
	mov	[ta_operand_size],0
      ta_avx_movlps_mem_size_ok:
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_avx_register
	cmp	ah,16
	jne	ta_invalid_operand
	mov	[ta_postbyte_register],al
	inc	[ta_extended_code]
	jmp	ta_instruction_ready
ta_avx_movhlps_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	or	[ta_vex_required],1
	call	ta_take_avx_register
	cmp	ah,16
	jne	ta_invalid_operand
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_avx_register
	mov	[ta_vex_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_avx_register
	mov	bl,al
	jmp	ta_nomem_instruction_ready
ta_avx_movsd_instruction:
	mov	al,0F2h
	mov	cl,8
	or	[ta_rex_prefix],80h
	jmp	ta_avx_movs_instruction
ta_avx_movss_instruction:
	mov	al,0F3h
	mov	cl,4
      ta_avx_movs_instruction:
	mov	[ta_opcode_prefix],al
	mov	[ta_mmx_size],cl
	or	[ta_vex_required],1
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],10h
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_avx_movs_mem
	lods	byte [esi]
	call	ta_convert_avx_register
	cmp	ah,16
	jne	ta_invalid_operand
	mov	[ta_postbyte_register],al
	call	ta_take_avx512_mask
	xor	cl,cl
	xchg	cl,[ta_operand_size]
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_avx_movs_reg_mem
	mov	[ta_operand_size],cl
	lods	byte [esi]
	call	ta_convert_avx_register
	mov	[ta_vex_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_avx_register
	mov	bl,al
	cmp	bl,8
	jb	ta_nomem_instruction_ready
	inc	[ta_extended_code]
	xchg	bl,[ta_postbyte_register]
	jmp	ta_nomem_instruction_ready
      ta_avx_movs_reg_mem:
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	mov	al,[ta_operand_size]
	or	al,al
	jz	ta_avx_movs_reg_mem_ok
	cmp	al,[ta_mmx_size]
	jne	ta_invalid_operand_size
      ta_avx_movs_reg_mem_ok:
	jmp	ta_instruction_ready
      ta_avx_movs_mem:
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	or	[ta_operand_flags],20h
	call	ta_take_avx512_mask
	jmp	ta_avx_movlps_mem_

ta_avx_comiss_instruction:
	or	[ta_operand_flags],2+4+10h
	mov	cl,4
	jmp	ta_avx_instruction
ta_avx_comisd_instruction:
	or	[ta_operand_flags],2+4+10h
	mov	[ta_opcode_prefix],66h
	or	[ta_rex_prefix],80h
	mov	cl,8
	jmp	ta_avx_instruction
ta_avx_movshdup_instruction:
	or	[ta_operand_flags],2
	mov	[ta_opcode_prefix],0F3h
	xor	cl,cl
	jmp	ta_avx_instruction
ta_avx_cvtqq2pd_instruction:
	mov	[ta_opcode_prefix],0F3h
	or	[ta_vex_required],8
	or	[ta_operand_flags],2+4+8
	or	[ta_rex_prefix],8
	mov	cx,0800h
	jmp	ta_avx_instruction_with_broadcast
ta_avx_pshuf_w_instruction:
	mov	[ta_opcode_prefix],al
	or	[ta_operand_flags],2
	mov	[ta_immediate_size],1
	mov	al,70h
	xor	cl,cl
	jmp	ta_avx_instruction
ta_avx_single_source_128bit_instruction_38_noevex:
	or	[ta_operand_flags],2
ta_avx_128bit_instruction_38_noevex:
	mov	cl,16
	jmp	ta_avx_instruction_38_noevex
ta_avx_single_source_instruction_38_noevex:
	or	[ta_operand_flags],2
	jmp	ta_avx_pi_instruction_38_noevex
ta_avx_pi_instruction_38_noevex:
	xor	cl,cl
      ta_avx_instruction_38_noevex:
	or	[ta_vex_required],2
      ta_avx_instruction_38:
	mov	[ta_opcode_prefix],66h
	mov	[ta_supplemental_code],al
	mov	al,38h
	jmp	ta_avx_instruction
ta_avx_ss_instruction_3a_imm8_noevex:
	mov	cl,4
	jmp	ta_avx_instruction_3a_imm8_noevex
ta_avx_sd_instruction_3a_imm8_noevex:
	mov	cl,8
	jmp	ta_avx_instruction_3a_imm8_noevex
ta_avx_single_source_128bit_instruction_3a_imm8_noevex:
	or	[ta_operand_flags],2
ta_avx_128bit_instruction_3a_imm8_noevex:
	mov	cl,16
	jmp	ta_avx_instruction_3a_imm8_noevex
ta_avx_triple_source_instruction_3a_noevex:
	xor	cl,cl
	mov	[ta_immediate_size],-1
	mov	byte [ta_value],0
	jmp	ta_avx_instruction_3a_noevex
ta_avx_single_source_instruction_3a_imm8_noevex:
	or	[ta_operand_flags],2
ta_avx_pi_instruction_3a_imm8_noevex:
	xor	cl,cl
      ta_avx_instruction_3a_imm8_noevex:
	mov	[ta_immediate_size],1
      ta_avx_instruction_3a_noevex:
	or	[ta_vex_required],2
      ta_avx_instruction_3a:
	mov	[ta_opcode_prefix],66h
	mov	[ta_supplemental_code],al
	mov	al,3Ah
	jmp	ta_avx_instruction
ta_avx_pi_instruction_3a_imm8:
	xor	cl,cl
	mov	[ta_immediate_size],1
	jmp	ta_avx_instruction_3a
ta_avx_pclmulqdq_instruction:
	mov	byte [ta_value],al
	mov	[ta_immediate_size],-4
	xor	cl,cl
	mov	al,44h
	or	[ta_operand_flags],10h
	jmp	ta_avx_instruction_3a
ta_avx_instruction_38_nomask:
	or	[ta_operand_flags],10h
	xor	cl,cl
	jmp	ta_avx_instruction_38

ta_avx512_single_source_pd_instruction_sae_imm8:
	or	[ta_operand_flags],2
ta_avx512_pd_instruction_sae_imm8:
	or	[ta_rex_prefix],8
	mov	cx,0800h
	jmp	ta_avx512_instruction_sae_imm8
ta_avx512_single_source_ps_instruction_sae_imm8:
	or	[ta_operand_flags],2
ta_avx512_ps_instruction_sae_imm8:
	mov	cx,0400h
	jmp	ta_avx512_instruction_sae_imm8
ta_avx512_sd_instruction_sae_imm8:
	or	[ta_rex_prefix],8
	mov	cx,0008h
	jmp	ta_avx512_instruction_sae_imm8
ta_avx512_ss_instruction_sae_imm8:
	mov	cx,0004h
      ta_avx512_instruction_sae_imm8:
	or	[ta_operand_flags],4
      ta_avx512_instruction_imm8:
	or	[ta_vex_required],8
	mov	[ta_opcode_prefix],66h
	mov	[ta_immediate_size],1
	mov	[ta_supplemental_code],al
	mov	al,3Ah
	jmp	ta_avx_instruction_with_broadcast
ta_avx512_pd_instruction_er:
	or	[ta_operand_flags],4+8
	jmp	ta_avx512_pd_instruction
ta_avx512_single_source_pd_instruction_sae:
	or	[ta_operand_flags],4
ta_avx512_single_source_pd_instruction:
	or	[ta_operand_flags],2
ta_avx512_pd_instruction:
	or	[ta_rex_prefix],8
	mov	cx,0800h
	jmp	ta_avx512_instruction
ta_avx512_ps_instruction_er:
	or	[ta_operand_flags],4+8
	jmp	ta_avx512_ps_instruction
ta_avx512_single_source_ps_instruction_sae:
	or	[ta_operand_flags],4
ta_avx512_single_source_ps_instruction:
	or	[ta_operand_flags],2
ta_avx512_ps_instruction:
	mov	cx,0400h
	jmp	ta_avx512_instruction
ta_avx512_sd_instruction_er:
	or	[ta_operand_flags],8
ta_avx512_sd_instruction_sae:
	or	[ta_operand_flags],4
ta_avx512_sd_instruction:
	or	[ta_rex_prefix],8
	mov	cx,0008h
	jmp	ta_avx512_instruction
ta_avx512_ss_instruction_er:
	or	[ta_operand_flags],8
ta_avx512_ss_instruction_sae:
	or	[ta_operand_flags],4
ta_avx512_ss_instruction:
	mov	cx,0004h
      ta_avx512_instruction:
	or	[ta_vex_required],8
	mov	[ta_opcode_prefix],66h
	mov	[ta_supplemental_code],al
	mov	al,38h
	jmp	ta_avx_instruction_with_broadcast
ta_avx512_exp2pd_instruction:
	or	[ta_rex_prefix],8
	or	[ta_operand_flags],2+4
	mov	cx,0840h
	jmp	ta_avx512_instruction
ta_avx512_exp2ps_instruction:
	or	[ta_operand_flags],2+4
	mov	cx,0440h
	jmp	ta_avx512_instruction

ta_fma_instruction_pd:
	or	[ta_rex_prefix],8
	mov	cx,0800h
	jmp	ta_fma_instruction
ta_fma_instruction_ps:
	mov	cx,0400h
	jmp	ta_fma_instruction
ta_fma_instruction_sd:
	or	[ta_rex_prefix],8
	mov	cx,0008h
	jmp	ta_fma_instruction
ta_fma_instruction_ss:
	mov	cx,0004h
      ta_fma_instruction:
	or	[ta_operand_flags],4+8
	mov	[ta_opcode_prefix],66h
	mov	[ta_supplemental_code],al
	mov	al,38h
	jmp	ta_avx_instruction_with_broadcast

ta_fma4_instruction_p:
	xor	cl,cl
	jmp	ta_fma4_instruction
ta_fma4_instruction_sd:
	mov	cl,8
	jmp	ta_fma4_instruction
ta_fma4_instruction_ss:
	mov	cl,4
      ta_fma4_instruction:
	mov	[ta_immediate_size],-2
	mov	byte [ta_value],0
	jmp	ta_avx_instruction_3a_noevex

ta_avx_cmp_pd_instruction:
	mov	[ta_opcode_prefix],66h
	or	[ta_rex_prefix],80h
	mov	cx,0800h
	jmp	ta_avx_cmp_instruction
ta_avx_cmp_ps_instruction:
	mov	cx,0400h
	jmp	ta_avx_cmp_instruction
ta_avx_cmp_sd_instruction:
	mov	[ta_opcode_prefix],0F2h
	or	[ta_rex_prefix],80h
	mov	cx,0008h
	jmp	ta_avx_cmp_instruction
ta_avx_cmp_ss_instruction:
	mov	[ta_opcode_prefix],0F3h
	mov	cx,0004h
      ta_avx_cmp_instruction:
	mov	byte [ta_value],al
	mov	[ta_immediate_size],-4
	or	[ta_operand_flags],4+20h
	mov	al,0C2h
	jmp	ta_avx_cmp_common
ta_avx_cmpeqq_instruction:
	or	[ta_rex_prefix],80h
	mov	ch,8
	mov	[ta_supplemental_code],al
	mov	al,38h
	jmp	ta_avx_cmp_pi_instruction
ta_avx_cmpeqd_instruction:
	mov	ch,4
	jmp	ta_avx_cmp_pi_instruction
ta_avx_cmpeqb_instruction:
	xor	ch,ch
	jmp	ta_avx_cmp_pi_instruction
ta_avx512_cmp_uq_instruction:
	or	[ta_rex_prefix],8
	mov	ch,8
	mov	ah,1Eh
	jmp	ta_avx_cmp_pi_instruction_evex
ta_avx512_cmp_ud_instruction:
	mov	ch,4
	mov	ah,1Eh
	jmp	ta_avx_cmp_pi_instruction_evex
ta_avx512_cmp_q_instruction:
	or	[ta_rex_prefix],8
	mov	ch,8
	mov	ah,1Fh
	jmp	ta_avx_cmp_pi_instruction_evex
ta_avx512_cmp_d_instruction:
	mov	ch,4
	mov	ah,1Fh
	jmp	ta_avx_cmp_pi_instruction_evex
ta_avx512_cmp_uw_instruction:
	or	[ta_rex_prefix],8
ta_avx512_cmp_ub_instruction:
	xor	ch,ch
	mov	ah,3Eh
	jmp	ta_avx_cmp_pi_instruction_evex
ta_avx512_cmp_w_instruction:
	or	[ta_rex_prefix],8
ta_avx512_cmp_b_instruction:
	xor	ch,ch
	mov	ah,3Fh
      ta_avx_cmp_pi_instruction_evex:
	mov	byte [ta_value],al
	mov	[ta_immediate_size],-4
	mov	[ta_supplemental_code],ah
	mov	al,3Ah
	or	[ta_vex_required],8
      ta_avx_cmp_pi_instruction:
	xor	cl,cl
	or	[ta_operand_flags],20h
	mov	[ta_opcode_prefix],66h
      ta_avx_cmp_common:
	mov	[ta_mmx_size],cl
	mov	[ta_broadcast_size],ch
	mov	[ta_extended_code],al
	mov	[ta_base_code],0Fh
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,14h
	je	ta_avx_maskreg
	cmp	al,10h
	jne	ta_invalid_operand
	or	[ta_vex_required],2
	jmp	ta_avx_reg
      ta_avx_maskreg:
	cmp	[ta_operand_size],0
	jne	ta_invalid_operand_size
	or	[ta_vex_required],8
	lods	byte [esi]
	call	ta_convert_mask_register
	mov	[ta_postbyte_register],al
	call	ta_take_avx512_mask
	jmp	ta_avx_vex_reg
ta_avx512_fpclasspd_instruction:
	or	[ta_rex_prefix],8
	mov	cx,0800h
	jmp	ta_avx_fpclass_instruction
ta_avx512_fpclassps_instruction:
	mov	cx,0400h
	jmp	ta_avx_fpclass_instruction
ta_avx512_fpclasssd_instruction:
	or	[ta_rex_prefix],8
	mov	cx,0008h
	jmp	ta_avx_fpclass_instruction
ta_avx512_fpclassss_instruction:
	mov	cx,0004h
      ta_avx_fpclass_instruction:
	mov	[ta_broadcast_size],ch
	mov	[ta_mmx_size],cl
	or	[ta_operand_flags],2
	call	ta_setup_66_0f_3a
	mov	[ta_immediate_size],1
	lods	byte [esi]
	cmp	al,14h
	je	ta_avx_maskreg
	jmp	ta_invalid_operand
ta_avx512_ptestnmd_instruction:
	mov	ch,4
	jmp	ta_avx512_ptestnm_instruction
ta_avx512_ptestnmq_instruction:
	or	[ta_rex_prefix],8
	mov	ch,8
	jmp	ta_avx512_ptestnm_instruction
ta_avx512_ptestnmw_instruction:
	or	[ta_rex_prefix],8
ta_avx512_ptestnmb_instruction:
	xor	ch,ch
      ta_avx512_ptestnm_instruction:
	mov	ah,0F3h
	jmp	ta_avx512_ptest_instruction
ta_avx512_ptestmd_instruction:
	mov	ch,4
	jmp	ta_avx512_ptestm_instruction
ta_avx512_ptestmq_instruction:
	or	[ta_rex_prefix],8
	mov	ch,8
	jmp	ta_avx512_ptestm_instruction
ta_avx512_ptestmw_instruction:
	or	[ta_rex_prefix],8
ta_avx512_ptestmb_instruction:
	xor	ch,ch
      ta_avx512_ptestm_instruction:
	mov	ah,66h
      ta_avx512_ptest_instruction:
	xor	cl,cl
	mov	[ta_opcode_prefix],ah
	mov	[ta_supplemental_code],al
	mov	al,38h
	or	[ta_vex_required],8
	jmp	ta_avx_cmp_common

ta_mask_shift_instruction_q:
	or	[ta_rex_prefix],8
ta_mask_shift_instruction_d:
	or	[ta_operand_flags],2
	or	[ta_immediate_size],1
	mov	[ta_opcode_prefix],66h
	mov	[ta_supplemental_code],al
	mov	al,3Ah
	jmp	ta_mask_instruction
ta_mask_instruction_single_source_b:
	mov	[ta_opcode_prefix],66h
	jmp	ta_mask_instruction_single_source_w
ta_mask_instruction_single_source_d:
	mov	[ta_opcode_prefix],66h
ta_mask_instruction_single_source_q:
	or	[ta_rex_prefix],8
ta_mask_instruction_single_source_w:
	or	[ta_operand_flags],2
	jmp	ta_mask_instruction
ta_mask_instruction_b:
	mov	[ta_opcode_prefix],66h
	jmp	ta_mask_instruction_w
ta_mask_instruction_d:
	mov	[ta_opcode_prefix],66h
ta_mask_instruction_q:
	or	[ta_rex_prefix],8
ta_mask_instruction_w:
	mov	[ta_operand_size],32
ta_mask_instruction:
	or	[ta_vex_required],1
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	call	ta_take_mask_register
	mov	[ta_postbyte_register],al
	test	[ta_operand_flags],2
	jnz	ta_mask_instruction_nds_ok
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_mask_register
	mov	[ta_vex_register],al
      ta_mask_instruction_nds_ok:
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_mask_register
	mov	bl,al
	cmp	[ta_immediate_size],0
	jne	ta_mmx_nomem_imm8
	jmp	ta_nomem_instruction_ready
ta_take_mask_register:
	lods	byte [esi]
	cmp	al,14h
	jne	ta_invalid_operand
	lods	byte [esi]
ta_convert_mask_register:
	mov	ah,al
	shr	ah,4
	cmp	ah,5
	jne	ta_invalid_operand
	and	al,1111b
	ret
ta_kmov_instruction:
	mov	[ta_mmx_size],al
	or	[ta_vex_required],1
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],90h
	lods	byte [esi]
	cmp	al,14h
	je	ta_kmov_maskreg
	cmp	al,10h
	je	ta_kmov_reg
	call	ta_get_size_operator
	inc	[ta_extended_code]
	cmp	al,'['
	jne	ta_invalid_argument
	call	ta_get_address
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_mask_register
	mov	[ta_postbyte_register],al
      ta_kmov_with_mem:
	mov	ah,[ta_mmx_size]
	mov	al,[ta_operand_size]
	or	al,al
	jz	ta_kmov_mem_size_ok
	cmp	al,ah
	jne	ta_invalid_operand_size
      ta_kmov_mem_size_ok:
	call	ta_setup_kmov_prefix
	jmp	ta_instruction_ready
      ta_setup_kmov_prefix:
	cmp	ah,4
	jb	ta_kmov_w_ok
	or	[ta_rex_prefix],8
      ta_kmov_w_ok:
	test	ah,1 or 4
	jz	ta_kmov_prefix_ok
	mov	[ta_opcode_prefix],66h
      ta_kmov_prefix_ok:
	ret
      ta_kmov_maskreg:
	lods	byte [esi]
	call	ta_convert_mask_register
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	cmp	al,14h
	je	ta_kmov_maskreg_maskreg
	cmp	al,10h
	je	ta_kmov_maskreg_reg
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_argument
	call	ta_get_address
	jmp	ta_kmov_with_mem
      ta_kmov_maskreg_maskreg:
	lods	byte [esi]
	call	ta_convert_mask_register
	mov	bl,al
	mov	ah,[ta_mmx_size]
	call	ta_setup_kmov_prefix
	jmp	ta_nomem_instruction_ready
      ta_kmov_maskreg_reg:
	add	[ta_extended_code],2
	lods	byte [esi]
	call	ta_convert_register
      ta_kmov_with_reg:
	mov	bl,al
	mov	al,[ta_mmx_size]
	mov	ah,4
	cmp	al,ah
	jbe	ta_kmov_reg_size_check
	mov	ah,al
      ta_kmov_reg_size_check:
	cmp	ah,[ta_operand_size]
	jne	ta_invalid_operand_size
	cmp	al,8
	je	ta_kmov_f2_w1
	cmp	al,2
	ja	ta_kmov_f2
	je	ta_nomem_instruction_ready
	mov	[ta_opcode_prefix],66h
	jmp	ta_nomem_instruction_ready
      ta_kmov_f2_w1:
	or	[ta_rex_prefix],8
	cmp	[ta_code_type],64
	jne	ta_illegal_instruction
      ta_kmov_f2:
	mov	[ta_opcode_prefix],0F2h
	jmp	ta_nomem_instruction_ready
      ta_kmov_reg:
	add	[ta_extended_code],3
	lods	byte [esi]
	call	ta_convert_register
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_mask_register
	jmp	ta_kmov_with_reg
ta_avx512_pmov_m2_instruction_w1:
	or	[ta_rex_prefix],8
ta_avx512_pmov_m2_instruction:
	or	[ta_vex_required],8
	call	ta_setup_f3_0f_38
	call	ta_take_avx_register
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_mask_register
	mov	bl,al
	jmp	ta_nomem_instruction_ready
ta_avx512_pmov_2m_instruction_w1:
	or	[ta_rex_prefix],8
ta_avx512_pmov_2m_instruction:
	or	[ta_vex_required],8
	call	ta_setup_f3_0f_38
	call	ta_take_mask_register
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_avx_register
	mov	bl,al
	jmp	ta_nomem_instruction_ready
      ta_setup_f3_0f_38:
	mov	[ta_extended_code],38h
	mov	[ta_supplemental_code],al
	mov	[ta_base_code],0Fh
	mov	[ta_opcode_prefix],0F3h
	ret

ta_vzeroall_instruction:
	mov	[ta_operand_size],32
ta_vzeroupper_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	and	[ta_displacement_compression],0
	call	ta_store_vex_instruction_code
	jmp	ta_instruction_assembled
ta_vstmxcsr_instruction:
	or	[ta_vex_required],2
	jmp	ta_stmxcsr_instruction

ta_avx_perm2f128_instruction:
	or	[ta_vex_required],2
	xor	ch,ch
      ta_avx_instruction_imm8_without_128bit:
	mov	[ta_immediate_size],1
	mov	ah,3Ah
	jmp	ta_avx_instruction_without_128bit
ta_avx512_shuf_q_instruction:
	or	[ta_rex_prefix],8
	or	[ta_vex_required],8
	mov	ch,8
	jmp	ta_avx_instruction_imm8_without_128bit
ta_avx512_shuf_d_instruction:
	or	[ta_vex_required],8
	mov	ch,4
	jmp	ta_avx_instruction_imm8_without_128bit
ta_avx_permd_instruction:
	mov	ah,38h
	mov	ch,4
      ta_avx_instruction_without_128bit:
	xor	cl,cl
	call	ta_setup_avx_66_supplemental
	call	ta_take_avx_register
	cmp	ah,32
	jb	ta_invalid_operand_size
	mov	[ta_postbyte_register],al
	call	ta_take_avx512_mask
	jmp	ta_avx_vex_reg
      ta_setup_avx_66_supplemental:
	mov	[ta_opcode_prefix],66h
	mov	[ta_broadcast_size],ch
	mov	[ta_mmx_size],cl
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],ah
	mov	[ta_supplemental_code],al
	or	[ta_vex_required],1
	ret
ta_avx_permq_instruction:
	or	[ta_rex_prefix],8
	mov	ch,8
	jmp	ta_avx_permil_instruction
ta_avx_permilpd_instruction:
	or	[ta_rex_prefix],80h
	mov	ch,8
	jmp	ta_avx_permil_instruction
ta_avx_permilps_instruction:
	mov	ch,4
      ta_avx_permil_instruction:
	or	[ta_operand_flags],2
	xor	cl,cl
	mov	ah,3Ah
	call	ta_setup_avx_66_supplemental
	call	ta_take_avx_register
	cmp	[ta_supplemental_code],4
	jae	ta_avx_permil_size_ok
	cmp	ah,32
	jb	ta_invalid_operand_size
      ta_avx_permil_size_ok:
	mov	[ta_postbyte_register],al
	call	ta_take_avx512_mask
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_avx_rm
	jnc	ta_mmx_imm8
	mov	bl,al
	cmp	byte [esi],','
	jne	ta_invalid_operand
	mov	al,[esi+1]
	cmp	al,11h
	jne	ta_avx_permil_rm_or_imm8
	mov	al,[esi+3]
      ta_avx_permil_rm_or_imm8:
	cmp	al,'('
	je	ta_mmx_nomem_imm8
	mov	[ta_vex_register],bl
	inc	esi
	mov	[ta_extended_code],38h
	mov	al,[ta_supplemental_code]
	cmp	al,4
	jb	ta_avx_permq_rm
	add	[ta_supplemental_code],8
	jmp	ta_avx_regs_rm
      ta_avx_permq_rm:
	or	[ta_vex_required],8
	shl	al,5
	neg	al
	add	al,36h
	mov	[ta_supplemental_code],al
	jmp	ta_avx_regs_rm
ta_vpermil_2pd_instruction:
	mov	[ta_immediate_size],-2
	mov	byte [ta_value],al
	mov	al,49h
	jmp	ta_vpermil2_instruction_setup
ta_vpermil_2ps_instruction:
	mov	[ta_immediate_size],-2
	mov	byte [ta_value],al
	mov	al,48h
	jmp	ta_vpermil2_instruction_setup
ta_vpermil2_instruction:
	mov	[ta_immediate_size],-3
	mov	byte [ta_value],0
      ta_vpermil2_instruction_setup:
	or	[ta_vex_required],2
	mov	[ta_base_code],0Fh
	mov	[ta_supplemental_code],al
	mov	al,3Ah
	xor	cl,cl
	jmp	ta_avx_instruction

ta_avx_shift_q_instruction_evex:
	or	[ta_vex_required],8
ta_avx_shift_q_instruction:
	or	[ta_rex_prefix],80h
	mov	cl,8
	jmp	ta_avx_shift_instruction
ta_avx_shift_d_instruction:
	mov	cl,4
	jmp	ta_avx_shift_instruction
ta_avx_shift_bw_instruction:
	xor	cl,cl
       ta_avx_shift_instruction:
	mov	[ta_broadcast_size],cl
	mov	[ta_mmx_size],0
	mov	[ta_opcode_prefix],66h
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	or	[ta_vex_required],1
	call	ta_take_avx_register
	mov	[ta_postbyte_register],al
	call	ta_take_avx512_mask
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	xor	cl,cl
	xchg	cl,[ta_operand_size]
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	je	ta_avx_shift_reg_mem
	mov	[ta_operand_size],cl
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_avx_register
	mov	[ta_vex_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	push	esi
	xor	cl,cl
	xchg	cl,[ta_operand_size]
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_avx_shift_reg_reg_reg
	pop	esi
	cmp	al,'['
	je	ta_avx_shift_reg_reg_mem
	xchg	cl,[ta_operand_size]
	test	cl,not 1
	jnz	ta_invalid_operand_size
	dec	esi
	call	ta_convert_avx_shift_opcode
	mov	bl,al
	jmp	ta_mmx_nomem_imm8
      ta_convert_avx_shift_opcode:
	mov	al,[ta_extended_code]
	mov	ah,al
	and	ah,1111b
	add	ah,70h
	mov	[ta_extended_code],ah
	shr	al,4
	sub	al,0Ch
	shl	al,1
	xchg	al,[ta_postbyte_register]
	xchg	al,[ta_vex_register]
	ret
      ta_avx_shift_reg_reg_reg:
	pop	eax
	lods	byte [esi]
	call	ta_convert_xmm_register
	xchg	cl,[ta_operand_size]
	mov	bl,al
	jmp	ta_nomem_instruction_ready
      ta_avx_shift_reg_reg_mem:
	mov	[ta_mmx_size],16
	push	ecx
	lods	byte [esi]
	call	ta_get_size_operator
	call	ta_get_address
	pop	eax
	xchg	al,[ta_operand_size]
	test	al,al
	jz	ta_instruction_ready
	cmp	al,16
	jne	ta_invalid_operand_size
	jmp	ta_instruction_ready
      ta_avx_shift_reg_mem:
	or	[ta_vex_required],8
	call	ta_take_avx_mem
	call	ta_convert_avx_shift_opcode
	jmp	ta_mmx_imm8
ta_avx_shift_dq_instruction:
	mov	[ta_postbyte_register],al
	mov	[ta_opcode_prefix],66h
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],73h
	or	[ta_vex_required],1
	mov	[ta_mmx_size],0
	call	ta_take_avx_register
	mov	[ta_vex_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	je	ta_avx_shift_dq_reg_mem
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_avx_register
	mov	bl,al
	jmp	ta_mmx_nomem_imm8
      ta_avx_shift_dq_reg_mem:
	or	[ta_vex_required],8
	call	ta_get_address
	jmp	ta_mmx_imm8
ta_avx512_rotate_q_instruction:
	mov	cl,8
	or	[ta_rex_prefix],cl
	jmp	ta_avx512_rotate_instruction
ta_avx512_rotate_d_instruction:
	mov	cl,4
      ta_avx512_rotate_instruction:
	mov	[ta_broadcast_size],cl
	mov	[ta_postbyte_register],al
	mov	[ta_opcode_prefix],66h
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],72h
	or	[ta_vex_required],8
	mov	[ta_mmx_size],0
	mov	[ta_immediate_size],1
	call	ta_take_avx_register
	mov	[ta_vex_register],al
	call	ta_take_avx512_mask
	jmp	ta_avx_vex_reg_ok

ta_avx_pmovsxbq_instruction:
	mov	cl,2
	jmp	ta_avx_pmovsx_instruction
ta_avx_pmovsxbd_instruction:
	mov	cl,4
	jmp	ta_avx_pmovsx_instruction
ta_avx_pmovsxbw_instruction:
	mov	cl,8
      ta_avx_pmovsx_instruction:
	mov	[ta_mmx_size],cl
	or	[ta_vex_required],1
	call	ta_setup_66_0f_38
	call	ta_take_avx_register
	mov	[ta_postbyte_register],al
	call	ta_take_avx512_mask
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	xor	al,al
	xchg	al,[ta_operand_size]
	bsf	ecx,eax
	sub	cl,4
	shl	[ta_mmx_size],cl
	push	eax
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_avx_pmovsx_reg_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	pop	eax
	xchg	al,[ta_operand_size]
	or	al,al
	jz	ta_instruction_ready
	cmp	al,[ta_mmx_size]
	jne	ta_invalid_operand_size
	jmp	ta_instruction_ready
      ta_avx_pmovsx_reg_reg:
	lods	byte [esi]
	call	ta_convert_avx_register
	mov	bl,al
	cmp	ah,[ta_mmx_size]
	je	ta_avx_pmovsx_xmmreg_reg_size_ok
	jb	ta_invalid_operand_size
	cmp	ah,16
	jne	ta_invalid_operand_size
      ta_avx_pmovsx_xmmreg_reg_size_ok:
	pop	eax
	mov	[ta_operand_size],al
	jmp	ta_nomem_instruction_ready
ta_avx512_pmovqb_instruction:
	mov	cl,2
	jmp	ta_avx512_pmov_instruction
ta_avx512_pmovdb_instruction:
	mov	cl,4
	jmp	ta_avx512_pmov_instruction
ta_avx512_pmovwb_instruction:
	mov	cl,8
      ta_avx512_pmov_instruction:
	mov	[ta_mmx_size],cl
	or	[ta_vex_required],8
	mov	[ta_extended_code],38h
	mov	[ta_supplemental_code],al
	mov	[ta_base_code],0Fh
	mov	[ta_opcode_prefix],0F3h
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_avx512_pmov_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	or	[ta_operand_flags],20h
	call	ta_avx512_pmov_common
	or	al,al
	jz	ta_instruction_ready
	cmp	al,[ta_mmx_size]
	jne	ta_invalid_operand_size
	jmp	ta_instruction_ready
      ta_avx512_pmov_common:
	call	ta_take_avx512_mask
	xor	al,al
	xchg	al,[ta_operand_size]
	push	eax
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_avx_register
	mov	[ta_postbyte_register],al
	mov	al,ah
	mov	ah,cl
	bsf	ecx,eax
	sub	cl,4
	shl	[ta_mmx_size],cl
	mov	cl,ah
	pop	eax
	ret
      ta_avx512_pmov_reg:
	lods	byte [esi]
	call	ta_convert_avx_register
	mov	bl,al
	call	ta_avx512_pmov_common
	cmp	al,[ta_mmx_size]
	je	ta_nomem_instruction_ready
	jb	ta_invalid_operand_size
	cmp	al,16
	jne	ta_invalid_operand_size
	jmp	ta_nomem_instruction_ready

ta_avx_broadcast_128_instruction_noevex:
	or	[ta_vex_required],2
	mov	cl,10h
	jmp	ta_avx_broadcast_instruction
ta_avx512_broadcast_32x2_instruction:
	mov	cl,08h
	jmp	ta_avx_broadcast_instruction_evex
ta_avx512_broadcast_32x4_instruction:
	mov	cl,10h
	jmp	ta_avx_broadcast_instruction_evex
ta_avx512_broadcast_32x8_instruction:
	mov	cl,20h
	jmp	ta_avx_broadcast_instruction_evex
ta_avx512_broadcast_64x2_instruction:
	mov	cl,10h
	jmp	ta_avx_broadcast_instruction_w1_evex
ta_avx512_broadcast_64x4_instruction:
	mov	cl,20h
      ta_avx_broadcast_instruction_w1_evex:
	or	[ta_rex_prefix],8
      ta_avx_broadcast_instruction_evex:
	or	[ta_vex_required],8
	jmp	ta_avx_broadcast_instruction
ta_avx_broadcastss_instruction:
	mov	cl,4
	jmp	ta_avx_broadcast_instruction
ta_avx_broadcastsd_instruction:
	or	[ta_rex_prefix],80h
	mov	cl,8
	jmp	ta_avx_broadcast_instruction
ta_avx_pbroadcastb_instruction:
	mov	cl,1
	jmp	ta_avx_broadcast_pi_instruction
ta_avx_pbroadcastw_instruction:
	mov	cl,2
	jmp	ta_avx_broadcast_pi_instruction
ta_avx_pbroadcastd_instruction:
	mov	cl,4
	jmp	ta_avx_broadcast_pi_instruction
ta_avx_pbroadcastq_instruction:
	mov	cl,8
	or	[ta_rex_prefix],80h
      ta_avx_broadcast_pi_instruction:
	or	[ta_operand_flags],40h
      ta_avx_broadcast_instruction:
	mov	[ta_opcode_prefix],66h
	mov	[ta_supplemental_code],al
	mov	al,38h
	mov	[ta_mmx_size],cl
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	or	[ta_vex_required],1
	call	ta_take_avx_register
	cmp	ah,[ta_mmx_size]
	je	ta_invalid_operand_size
	test	[ta_operand_flags],40h
	jnz	ta_avx_broadcast_destination_size_ok
	cmp	[ta_mmx_size],4
	je	ta_avx_broadcast_destination_size_ok
	cmp	[ta_supplemental_code],59h
	je	ta_avx_broadcast_destination_size_ok
	cmp	ah,16
	je	ta_invalid_operand_size
      ta_avx_broadcast_destination_size_ok:
	xor	ah,ah
	xchg	ah,[ta_operand_size]
	push	eax
	call	ta_take_avx512_mask
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_avx_broadcast_reg_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	pop	eax
	xchg	ah,[ta_operand_size]
	mov	[ta_postbyte_register],al
	mov	al,[ta_broadcast_size]
	mov	al,[ta_mmx_size]
	cmp	al,ah
	je	ta_instruction_ready
	or	al,al
	jz	ta_instruction_ready
	or	ah,ah
	jz	ta_instruction_ready
	jmp	ta_invalid_operand_size
      ta_avx_broadcast_reg_reg:
	lods	byte [esi]
	test	[ta_operand_flags],40h
	jz	ta_avx_broadcast_reg_avx_reg
	cmp	al,60h
	jb	ta_avx_broadcast_reg_general_reg
	cmp	al,80h
	jb	ta_avx_broadcast_reg_avx_reg
	cmp	al,0C0h
	jb	ta_avx_broadcast_reg_general_reg
      ta_avx_broadcast_reg_avx_reg:
	call	ta_convert_avx_register
	mov	bl,al
	mov	al,[ta_mmx_size]
	or	al,al
	jz	ta_avx_broadcast_reg_avx_reg_size_ok
	cmp	ah,16
	jne	ta_invalid_operand_size
	cmp	al,ah
	jae	ta_invalid_operand
      ta_avx_broadcast_reg_avx_reg_size_ok:
	pop	eax
	xchg	ah,[ta_operand_size]
	mov	[ta_postbyte_register],al
	test	[ta_vex_required],2
	jnz	ta_invalid_operand
	jmp	ta_nomem_instruction_ready
      ta_avx_broadcast_reg_general_reg:
	call	ta_convert_register
	mov	bl,al
	mov	al,[ta_mmx_size]
	or	al,al
	jz	ta_avx_broadcast_reg_general_reg_size_ok
	cmp	al,ah
	je	ta_avx_broadcast_reg_general_reg_size_ok
	ja	ta_invalid_operand_size
	cmp	ah,4
	jne	ta_invalid_operand_size
      ta_avx_broadcast_reg_general_reg_size_ok:
	cmp	al,4
	jb	ta_avx_broadcast_reg_general_reg_ready
	cmp	al,8
	mov	al,3
	jne	ta_avx_broadcast_reg_general_reg_ready
	or	[ta_rex_prefix],8
      ta_avx_broadcast_reg_general_reg_ready:
	add	al,7Ah-1
	mov	[ta_supplemental_code],al
	or	[ta_vex_required],8
	pop	eax
	xchg	ah,[ta_operand_size]
	mov	[ta_postbyte_register],al
	jmp	ta_nomem_instruction_ready

ta_avx512_extract_64x4_instruction:
	or	[ta_rex_prefix],8
ta_avx512_extract_32x8_instruction:
	or	[ta_vex_required],8
	mov	cl,32
	jmp	ta_avx_extractf_instruction
ta_avx512_extract_64x2_instruction:
	or	[ta_rex_prefix],8
ta_avx512_extract_32x4_instruction:
	or	[ta_vex_required],8
	mov	cl,16
	jmp	ta_avx_extractf_instruction
ta_avx_extractf128_instruction:
	or	[ta_vex_required],2
	mov	cl,16
      ta_avx_extractf_instruction:
	mov	[ta_mmx_size],cl
	call	ta_setup_66_0f_3a
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_avx_extractf_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	xor	al,al
	xchg	al,[ta_operand_size]
	or	al,al
	jz	ta_avx_extractf_mem_size_ok
	cmp	al,[ta_mmx_size]
	jne	ta_invalid_operand_size
      ta_avx_extractf_mem_size_ok:
	call	ta_take_avx512_mask
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_avx_register
	cmp	ah,[ta_mmx_size]
	jbe	ta_invalid_operand_size
	mov	[ta_postbyte_register],al
	jmp	ta_mmx_imm8
      ta_avx_extractf_reg:
	lods	byte [esi]
	call	ta_convert_avx_register
	cmp	ah,[ta_mmx_size]
	jne	ta_invalid_operand_size
	push	eax
	call	ta_take_avx512_mask
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_avx_register
	cmp	ah,[ta_mmx_size]
	jbe	ta_invalid_operand_size
	mov	[ta_postbyte_register],al
	pop	ebx
	jmp	ta_mmx_nomem_imm8
ta_avx512_insert_64x4_instruction:
	or	[ta_rex_prefix],8
ta_avx512_insert_32x8_instruction:
	or	[ta_vex_required],8
	mov	cl,32
	jmp	ta_avx_insertf_instruction
ta_avx512_insert_64x2_instruction:
	or	[ta_rex_prefix],8
ta_avx512_insert_32x4_instruction:
	or	[ta_vex_required],8
	mov	cl,16
	jmp	ta_avx_insertf_instruction
ta_avx_insertf128_instruction:
	or	[ta_vex_required],2
	mov	cl,16
      ta_avx_insertf_instruction:
	mov	[ta_mmx_size],cl
	mov	[ta_broadcast_size],0
	call	ta_setup_66_0f_3a
	call	ta_take_avx_register
	cmp	ah,[ta_mmx_size]
	jbe	ta_invalid_operand
	mov	[ta_postbyte_register],al
	call	ta_take_avx512_mask
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_avx_register
	mov	[ta_vex_register],al
	mov	al,[ta_mmx_size]
	xchg	al,[ta_operand_size]
	push	eax
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_avx_insertf_reg_reg_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	pop	eax
	mov	[ta_operand_size],al
	jmp	ta_mmx_imm8
      ta_avx_insertf_reg_reg_reg:
	lods	byte [esi]
	call	ta_convert_avx_register
	mov	bl,al
	pop	eax
	mov	[ta_operand_size],al
	jmp	ta_mmx_nomem_imm8
ta_avx_extract_b_instruction:
	mov	cl,1
	jmp	ta_avx_extract_instruction
ta_avx_extract_w_instruction:
	mov	cl,2
	jmp	ta_avx_extract_instruction
ta_avx_extract_q_instruction:
	or	[ta_rex_prefix],8
	mov	cl,8
	jmp	ta_avx_extract_instruction
ta_avx_extract_d_instruction:
	mov	cl,4
      ta_avx_extract_instruction:
	mov	[ta_mmx_size],cl
	call	ta_setup_66_0f_3a
	or	[ta_vex_required],1
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_avx_extractps_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	mov	al,[ta_mmx_size]
	not	al
	and	[ta_operand_size],al
	jnz	ta_invalid_operand_size
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_avx_register
	cmp	ah,16
	jne	ta_invalid_operand_size
	mov	[ta_postbyte_register],al
	jmp	ta_mmx_imm8
      ta_avx_extractps_reg:
	lods	byte [esi]
	call	ta_convert_register
	mov	bl,al
	mov	al,[ta_mmx_size]
	cmp	ah,al
	jb	ta_invalid_operand_size
	cmp	ah,4
	je	ta_avx_extractps_reg_size_ok
	cmp	ah,8
	jne	ta_invalid_operand_size
	cmp	[ta_code_type],64
	jne	ta_invalid_operand
	cmp	al,4
	jae	ta_avx_extractps_reg_size_ok
	or	[ta_rex_prefix],8
      ta_avx_extractps_reg_size_ok:
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_avx_register
	cmp	ah,16
	jne	ta_invalid_operand_size
	mov	[ta_postbyte_register],al
	cmp	[ta_supplemental_code],15h
	jne	ta_mmx_nomem_imm8
	mov	[ta_extended_code],0C5h
	xchg	bl,[ta_postbyte_register]
	jmp	ta_mmx_nomem_imm8
ta_avx_insertps_instruction:
	mov	[ta_immediate_size],1
	or	[ta_operand_flags],10h
	mov	[ta_opcode_prefix],66h
	mov	[ta_supplemental_code],al
	mov	al,3Ah
	mov	cl,4
	jmp	ta_avx_instruction
ta_avx_pinsrb_instruction:
	mov	cl,1
	jmp	ta_avx_pinsr_instruction_3a
ta_avx_pinsrw_instruction:
	mov	cl,2
	jmp	ta_avx_pinsr_instruction
ta_avx_pinsrd_instruction:
	mov	cl,4
	jmp	ta_avx_pinsr_instruction_3a
ta_avx_pinsrq_instruction:
	cmp	[ta_code_type],64
	jne	ta_illegal_instruction
	mov	cl,8
	or	[ta_rex_prefix],8
      ta_avx_pinsr_instruction_3a:
	mov	[ta_supplemental_code],al
	mov	al,3Ah
      ta_avx_pinsr_instruction:
	mov	[ta_opcode_prefix],66h
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	mov	[ta_mmx_size],cl
	or	[ta_vex_required],1
	call	ta_take_avx_register
	cmp	ah,16
	jne	ta_invalid_operand_size
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_avx_register
	mov	[ta_vex_register],al
	jmp	ta_pinsr_xmmreg

ta_avx_cvtudq2pd_instruction:
	or	[ta_vex_required],8
ta_avx_cvtdq2pd_instruction:
	mov	[ta_opcode_prefix],0F3h
	mov	cl,4
	jmp	ta_avx_cvt_d_instruction
ta_avx_cvtps2qq_instruction:
	or	[ta_operand_flags],8
ta_avx_cvttps2qq_instruction:
	or	[ta_operand_flags],4
	or	[ta_vex_required],8
	mov	[ta_opcode_prefix],66h
	mov	cl,4
	jmp	ta_avx_cvt_d_instruction
ta_avx_cvtps2pd_instruction:
	or	[ta_operand_flags],4
	mov	cl,4
      ta_avx_cvt_d_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	or	[ta_vex_required],1
	mov	[ta_broadcast_size],cl
	call	ta_take_avx_register
	mov	[ta_postbyte_register],al
	call	ta_take_avx512_mask
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	xor	ecx,ecx
	xchg	cl,[ta_operand_size]
	mov	al,cl
	shr	al,1
	mov	[ta_mmx_size],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	je	ta_avx_cvt_d_reg_mem
	cmp	al,10h
	jne	ta_invalid_operand
	mov	[ta_operand_size],0
	lods	byte [esi]
	call	ta_convert_avx_register
	cmp	ah,[ta_mmx_size]
	je	ta_avx_cvt_d_reg_reg_size_ok
	jb	ta_invalid_operand_size
	cmp	ah,16
	jne	ta_invalid_operand_size
      ta_avx_cvt_d_reg_reg_size_ok:
	mov	bl,al
	mov	[ta_operand_size],cl
	call	ta_take_avx512_rounding
	jmp	ta_nomem_instruction_ready
      ta_avx_cvt_d_reg_mem:
	call	ta_take_avx_mem
	jmp	ta_instruction_ready
ta_avx_cvtpd2dq_instruction:
	or	[ta_operand_flags],4+8
	mov	[ta_opcode_prefix],0F2h
	jmp	ta_avx_cvt_q_instruction
ta_avx_cvtuqq2ps_instruction:
	mov	[ta_opcode_prefix],0F2h
ta_avx_cvtpd2udq_instruction:
	or	[ta_operand_flags],8
ta_avx_cvttpd2udq_instruction:
	or	[ta_operand_flags],4
	or	[ta_vex_required],8
	jmp	ta_avx_cvt_q_instruction
ta_avx_cvtpd2ps_instruction:
	or	[ta_operand_flags],8
ta_avx_cvttpd2dq_instruction:
	or	[ta_operand_flags],4
	mov	[ta_opcode_prefix],66h
      ta_avx_cvt_q_instruction:
	mov	[ta_broadcast_size],8
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	or	[ta_vex_required],1
	or	[ta_rex_prefix],80h
	call	ta_take_avx_register
	mov	[ta_postbyte_register],al
	push	eax
	call	ta_take_avx512_mask
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	xor	al,al
	mov	[ta_operand_size],al
	mov	[ta_mmx_size],al
	call	ta_take_avx_rm
	jnc	ta_avx_cvt_q_reg_mem
	mov	bl,al
	pop	eax
	call	ta_avx_cvt_q_check_size
	call	ta_take_avx512_rounding
	jmp	ta_nomem_instruction_ready
      ta_avx_cvt_q_reg_mem:
	pop	eax
	call	ta_avx_cvt_q_check_size
	jmp	ta_instruction_ready
      ta_avx_cvt_q_check_size:
	mov	al,[ta_operand_size]
	or	al,al
	jz	ta_avx_cvt_q_size_not_specified
	cmp	al,64
	ja	ta_invalid_operand_size
	shr	al,1
	cmp	al,ah
	je	ta_avx_cvt_q_size_ok
	ja	ta_invalid_operand_size
	cmp	ah,16
	jne	ta_invalid_operand_size
      ta_avx_cvt_q_size_ok:
	ret
      ta_avx_cvt_q_size_not_specified:
	cmp	ah,64 shr 1
	jne	ta_recoverable_unknown_size
	mov	[ta_operand_size],64
	ret
ta_avx_cvttps2udq_instruction:
	or	[ta_vex_required],8
	or	[ta_operand_flags],2+4
	mov	cx,0400h
	jmp	ta_avx_instruction_with_broadcast
ta_avx_cvttps2dq_instruction:
	mov	[ta_opcode_prefix],0F3h
	or	[ta_operand_flags],2+4
	mov	cx,0400h
	jmp	ta_avx_instruction_with_broadcast
ta_avx_cvtph2ps_instruction:
	mov	[ta_opcode_prefix],66h
	mov	[ta_supplemental_code],al
	or	[ta_operand_flags],4
	mov	al,38h
	xor	cl,cl
	jmp	ta_avx_cvt_d_instruction
ta_avx_cvtps2ph_instruction:
	call	ta_setup_66_0f_3a
	or	[ta_vex_required],1
	or	[ta_operand_flags],4
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_vcvtps2ph_reg
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	call	ta_take_avx512_mask
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	shl	[ta_operand_size],1
	call	ta_take_avx_register
	mov	[ta_postbyte_register],al
	shr	ah,1
	mov	[ta_mmx_size],ah
	jmp	ta_mmx_imm8
      ta_vcvtps2ph_reg:
	lods	byte [esi]
	call	ta_convert_avx_register
	mov	bl,al
	call	ta_take_avx512_mask
	xor	cl,cl
	xchg	cl,[ta_operand_size]
	shl	cl,1
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_avx_register
	mov	[ta_postbyte_register],al
	or	cl,cl
	jz	ta_vcvtps2ph_reg_size_ok
	cmp	cl,ah
	je	ta_vcvtps2ph_reg_size_ok
	jb	ta_invalid_operand_size
	cmp	ah,16
	jne	ta_invalid_operand_size
      ta_vcvtps2ph_reg_size_ok:
	call	ta_take_avx512_rounding
	jmp	ta_mmx_nomem_imm8

ta_avx_cvtsd2usi_instruction:
	or	[ta_operand_flags],8
ta_avx_cvttsd2usi_instruction:
	or	[ta_vex_required],8
	jmp	ta_avx_cvttsd2si_instruction
ta_avx_cvtsd2si_instruction:
	or	[ta_operand_flags],8
ta_avx_cvttsd2si_instruction:
	mov	ah,0F2h
	mov	cl,8
	jmp	ta_avx_cvt_2si_instruction
ta_avx_cvtss2usi_instruction:
	or	[ta_operand_flags],8
ta_avx_cvttss2usi_instruction:
	or	[ta_vex_required],8
	jmp	ta_avx_cvttss2si_instruction
ta_avx_cvtss2si_instruction:
	or	[ta_operand_flags],8
ta_avx_cvttss2si_instruction:
	mov	ah,0F3h
	mov	cl,4
      ta_avx_cvt_2si_instruction:
	or	[ta_operand_flags],2+4
	mov	[ta_mmx_size],cl
	mov	[ta_broadcast_size],0
	mov	[ta_opcode_prefix],ah
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	or	[ta_vex_required],1
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_register
	mov	[ta_postbyte_register],al
	mov	[ta_operand_size],0
	cmp	ah,4
	je	ta_avx_cvt_2si_reg
	cmp	ah,8
	jne	ta_invalid_operand_size
	call	ta_operand_64bit
      ta_avx_cvt_2si_reg:
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_avx_rm
	jnc	ta_instruction_ready
	mov	bl,al
	call	ta_take_avx512_rounding
	jmp	ta_nomem_instruction_ready
ta_avx_cvtusi2sd_instruction:
	or	[ta_vex_required],8
ta_avx_cvtsi2sd_instruction:
	mov	ah,0F2h
	mov	cl,8
	jmp	ta_avx_cvtsi_instruction
ta_avx_cvtusi2ss_instruction:
	or	[ta_vex_required],8
ta_avx_cvtsi2ss_instruction:
	mov	ah,0F3h
	mov	cl,4
      ta_avx_cvtsi_instruction:
	or	[ta_operand_flags],2+4+8
	mov	[ta_mmx_size],cl
	mov	[ta_opcode_prefix],ah
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	or	[ta_vex_required],1
	call	ta_take_avx_register
	cmp	ah,16
	jne	ta_invalid_operand_size
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_avx_register
	mov	[ta_vex_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	mov	[ta_operand_size],0
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	je	ta_avx_cvtsi_reg_reg_mem
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_register
	mov	bl,al
	cmp	ah,4
	je	ta_avx_cvtsi_reg_reg_reg32
	cmp	ah,8
	jne	ta_invalid_operand_size
	call	ta_operand_64bit
      ta_avx_cvtsi_rounding:
	call	ta_take_avx512_rounding
	jmp	ta_nomem_instruction_ready
      ta_avx_cvtsi_reg_reg_reg32:
	cmp	[ta_mmx_size],8
	jne	ta_avx_cvtsi_rounding
	jmp	ta_nomem_instruction_ready
      ta_avx_cvtsi_reg_reg_mem:
	call	ta_get_address
	mov	al,[ta_operand_size]
	mov	[ta_mmx_size],al
	or	al,al
	jz	ta_single_mem_nosize
	cmp	al,4
	je	ta_instruction_ready
	cmp	al,8
	jne	ta_invalid_operand_size
	call	ta_operand_64bit
	jmp	ta_instruction_ready

ta_avx_maskmov_w1_instruction:
	or	[ta_rex_prefix],8
ta_avx_maskmov_instruction:
	call	ta_setup_66_0f_38
	mov	[ta_mmx_size],0
	or	[ta_vex_required],2
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_avx_maskmov_mem
	lods	byte [esi]
	call	ta_convert_avx_register
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_avx_register
	mov	[ta_vex_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	jmp	ta_instruction_ready
      ta_avx_maskmov_mem:
	cmp	al,'['
	jne	ta_invalid_operand
	call	ta_get_address
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_avx_register
	mov	[ta_vex_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_avx_register
	mov	[ta_postbyte_register],al
	add	[ta_supplemental_code],2
	jmp	ta_instruction_ready
ta_avx_movmskpd_instruction:
	mov	[ta_opcode_prefix],66h
ta_avx_movmskps_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],50h
	or	[ta_vex_required],2
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_register
	mov	[ta_postbyte_register],al
	cmp	ah,4
	je	ta_avx_movmskps_reg_ok
	cmp	ah,8
	jne	ta_invalid_operand_size
	cmp	[ta_code_type],64
	jne	ta_invalid_operand
      ta_avx_movmskps_reg_ok:
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_avx_register
	mov	bl,al
	jmp	ta_nomem_instruction_ready
ta_avx_maskmovdqu_instruction:
	or	[ta_vex_required],2
	jmp	ta_maskmovdqu_instruction
ta_avx_pmovmskb_instruction:
	or	[ta_vex_required],2
	mov	[ta_opcode_prefix],66h
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_register
	cmp	ah,4
	je	ta_avx_pmovmskb_reg_size_ok
	cmp	[ta_code_type],64
	jne	ta_invalid_operand_size
	cmp	ah,8
	jnz	ta_invalid_operand_size
      ta_avx_pmovmskb_reg_size_ok:
	mov	[ta_postbyte_register],al
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_avx_register
	mov	bl,al
	jmp	ta_nomem_instruction_ready

ta_gather_pd_instruction:
	or	[ta_rex_prefix],8
ta_gather_ps_instruction:
	call	ta_setup_66_0f_38
	or	[ta_vex_required],4
	or	[ta_operand_flags],20h
	call	ta_take_avx_register
	mov	[ta_postbyte_register],al
	call	ta_take_avx512_mask
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	xor	cl,cl
	xchg	cl,[ta_operand_size]
	push	ecx
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_argument
	call	ta_get_address
	pop	eax
	xchg	al,[ta_operand_size]
      ta_gather_mem_size_check:
	mov	ah,4
	test	[ta_rex_prefix],8
	jz	ta_gather_elements_size_ok
	add	ah,ah
      ta_gather_elements_size_ok:
	mov	[ta_mmx_size],ah
	test	al,al
	jz	ta_gather_mem_size_ok
	cmp	al,ah
	jne	ta_invalid_operand_size
      ta_gather_mem_size_ok:
	cmp	byte [esi],','
	je	ta_gather_reg_mem_reg
	test	[ta_vex_required],20h
	jz	ta_invalid_operand
	mov	ah,[ta_operand_size]
	mov	al,80h
	jmp	ta_gather_arguments_ok
      ta_gather_reg_mem_reg:
	or	[ta_vex_required],2
	inc	esi
	call	ta_take_avx_register
      ta_gather_arguments_ok:
	mov	[ta_vex_register],al
	cmp	al,[ta_postbyte_register]
	je	ta_disallowed_combination_of_registers
	mov	al,bl
	and	al,11111b
	cmp	al,[ta_postbyte_register]
	je	ta_disallowed_combination_of_registers
	cmp	al,[ta_vex_register]
	je	ta_disallowed_combination_of_registers
	mov	al,bl
	shr	al,5
	cmp	al,0Ch shr 1
	je	ta_gather_vr128
	mov	ah,32
	cmp	al,6 shr 1
	jne	ta_gather_regular
	add	ah,ah
      ta_gather_regular:
	mov	al,[ta_rex_prefix]
	shr	al,3
	xor	al,[ta_supplemental_code]
	test	al,1
	jz	ta_gather_uniform
	test	[ta_supplemental_code],1
	jz	ta_gather_double
	mov	al,ah
	xchg	al,[ta_operand_size]
	add	al,al
	cmp	al,ah
	jne	ta_invalid_operand_size
	jmp	ta_instruction_ready
      ta_gather_double:
	add	ah,ah
      ta_gather_uniform:
	cmp	ah,[ta_operand_size]
	jne	ta_invalid_operand_size
	jmp	ta_instruction_ready
      ta_gather_vr128:
	cmp	ah,16
	je	ta_instruction_ready
	cmp	ah,32
	jne	ta_invalid_operand_size
	test	[ta_supplemental_code],1
	jnz	ta_invalid_operand_size
	test	[ta_rex_prefix],8
	jz	ta_invalid_operand_size
	jmp	ta_instruction_ready
ta_scatter_pd_instruction:
	or	[ta_rex_prefix],8
ta_scatter_ps_instruction:
	call	ta_setup_66_0f_38
	or	[ta_vex_required],4+8
	or	[ta_operand_flags],20h
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_argument
	call	ta_get_address
	call	ta_take_avx512_mask
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	xor	al,al
	xchg	al,[ta_operand_size]
	push	eax
	call	ta_take_avx_register
	mov	[ta_postbyte_register],al
	pop	eax
	jmp	ta_gather_mem_size_check
ta_gatherpf_qpd_instruction:
	mov	ah,0C7h
	jmp	ta_gatherpf_pd_instruction
ta_gatherpf_dpd_instruction:
	mov	ah,0C6h
      ta_gatherpf_pd_instruction:
	or	[ta_rex_prefix],8
	mov	cl,8
	jmp	ta_gatherpf_instruction
ta_gatherpf_qps_instruction:
	mov	ah,0C7h
	jmp	ta_gatherpf_ps_instruction
ta_gatherpf_dps_instruction:
	mov	ah,0C6h
      ta_gatherpf_ps_instruction:
	mov	cl,4
      ta_gatherpf_instruction:
	mov	[ta_mmx_size],cl
	mov	[ta_postbyte_register],al
	mov	al,ah
	call	ta_setup_66_0f_38
	or	[ta_vex_required],4+8
	or	[ta_operand_flags],20h
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	jne	ta_invalid_argument
	call	ta_get_address
	call	ta_take_avx512_mask
	mov	ah,[ta_mmx_size]
	mov	al,[ta_operand_size]
	or	al,al
	jz	ta_gatherpf_mem_size_ok
	cmp	al,ah
	jne	ta_invalid_operand_size
      ta_gatherpf_mem_size_ok:
	mov	[ta_operand_size],64
	mov	al,6 shr 1
	cmp	ah,4
	je	ta_gatherpf_check_vsib
	cmp	[ta_supplemental_code],0C6h
	jne	ta_gatherpf_check_vsib
	mov	al,0Eh shr 1
      ta_gatherpf_check_vsib:
	mov	ah,bl
	shr	ah,5
	cmp	al,ah
	jne	ta_invalid_operand
	jmp	ta_instruction_ready

ta_bmi_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],38h
	mov	[ta_supplemental_code],0F3h
	mov	[ta_postbyte_register],al
      ta_bmi_reg:
	or	[ta_vex_required],2
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_register
	mov	[ta_vex_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_bmi_reg_reg
	cmp	al,'['
	jne	ta_invalid_argument
	call	ta_get_address
	call	ta_operand_32or64
	jmp	ta_instruction_ready
      ta_bmi_reg_reg:
	lods	byte [esi]
	call	ta_convert_register
	mov	bl,al
	call	ta_operand_32or64
	jmp	ta_nomem_instruction_ready
      ta_operand_32or64:
	mov	al,[ta_operand_size]
	cmp	al,4
	je	ta_operand_32or64_ok
	cmp	al,8
	jne	ta_invalid_operand_size
	cmp	[ta_code_type],64
	jne	ta_invalid_operand
	or	[ta_rex_prefix],8
      ta_operand_32or64_ok:
	ret
ta_pdep_instruction:
	mov	[ta_opcode_prefix],0F2h
	jmp	ta_andn_instruction
ta_pext_instruction:
	mov	[ta_opcode_prefix],0F3h
ta_andn_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],38h
	mov	[ta_supplemental_code],al
	or	[ta_vex_required],2
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_register
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	jmp	ta_bmi_reg
ta_sarx_instruction:
	mov	[ta_opcode_prefix],0F3h
	jmp	ta_bzhi_instruction
ta_shrx_instruction:
	mov	[ta_opcode_prefix],0F2h
	jmp	ta_bzhi_instruction
ta_shlx_instruction:
	mov	[ta_opcode_prefix],66h
ta_bzhi_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],38h
	mov	[ta_supplemental_code],al
	or	[ta_vex_required],2
	call	ta_get_reg_mem
	jc	ta_bzhi_reg_reg
	call	ta_get_vex_source_register
	jc	ta_invalid_operand
	call	ta_operand_32or64
	jmp	ta_instruction_ready
      ta_bzhi_reg_reg:
	call	ta_get_vex_source_register
	jc	ta_invalid_operand
	call	ta_operand_32or64
	jmp	ta_nomem_instruction_ready
      ta_get_vex_source_register:
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_no_vex_source_register
	lods	byte [esi]
	call	ta_convert_register
	mov	[ta_vex_register],al
	clc
	ret
      ta_no_vex_source_register:
	stc
	ret
ta_bextr_instruction:
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],38h
	mov	[ta_supplemental_code],al
	or	[ta_vex_required],2
	call	ta_get_reg_mem
	jc	ta_bextr_reg_reg
	call	ta_get_vex_source_register
	jc	ta_bextr_reg_mem_imm32
	call	ta_operand_32or64
	jmp	ta_instruction_ready
      ta_bextr_reg_reg:
	call	ta_get_vex_source_register
	jc	ta_bextr_reg_reg_imm32
	call	ta_operand_32or64
	jmp	ta_nomem_instruction_ready
      ta_setup_bextr_imm_opcode:
	mov	[ta_xop_opcode_map],0Ah
	mov	[ta_base_code],10h
	call	ta_operand_32or64
	ret
      ta_bextr_reg_mem_imm32:
	call	ta_get_imm32
	call	ta_setup_bextr_imm_opcode
	jmp	ta_store_instruction_with_imm32
      ta_bextr_reg_reg_imm32:
	call	ta_get_imm32
	call	ta_setup_bextr_imm_opcode
      ta_store_nomem_instruction_with_imm32:
	call	ta_store_nomem_instruction
	mov	eax,dword [ta_value]
	call	ta_mark_relocation
	stos	dword [edi]
	jmp	ta_instruction_assembled
      ta_get_imm32:
	cmp	al,'('
	jne	ta_invalid_operand
	push	edx ebx ecx
	call	ta_get_dword_value
	mov	dword [ta_value],eax
	pop	ecx ebx edx
	ret
ta_rorx_instruction:
	mov	[ta_opcode_prefix],0F2h
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],3Ah
	mov	[ta_supplemental_code],al
	or	[ta_vex_required],2
	call	ta_get_reg_mem
	jc	ta_rorx_reg_reg
	call	ta_operand_32or64
	jmp	ta_mmx_imm8
      ta_rorx_reg_reg:
	call	ta_operand_32or64
	jmp	ta_mmx_nomem_imm8

ta_tbm_instruction:
	mov	[ta_xop_opcode_map],9
	mov	ah,al
	shr	ah,4
	and	al,111b
	mov	[ta_base_code],ah
	mov	[ta_postbyte_register],al
	jmp	ta_bmi_reg

ta_llwpcb_instruction:
	or	[ta_vex_required],2
	mov	[ta_xop_opcode_map],9
	mov	[ta_base_code],12h
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_register
	mov	bl,al
	call	ta_operand_32or64
	jmp	ta_nomem_instruction_ready
ta_lwpins_instruction:
	or	[ta_vex_required],2
	mov	[ta_xop_opcode_map],0Ah
	mov	[ta_base_code],12h
	mov	[ta_vex_register],al
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_register
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	xor	cl,cl
	xchg	cl,[ta_operand_size]
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	je	ta_lwpins_reg_reg
	cmp	al,'['
	jne	ta_invalid_argument
	push	ecx
	call	ta_get_address
	pop	eax
	xchg	al,[ta_operand_size]
	test	al,al
	jz	ta_lwpins_reg_mem_size_ok
	cmp	al,4
	jne	ta_invalid_operand_size
      ta_lwpins_reg_mem_size_ok:
	call	ta_prepare_lwpins
	jmp	ta_store_instruction_with_imm32
      ta_lwpins_reg_reg:
	lods	byte [esi]
	call	ta_convert_register
	cmp	ah,4
	jne	ta_invalid_operand_size
	mov	[ta_operand_size],cl
	mov	bl,al
	call	ta_prepare_lwpins
	jmp	ta_store_nomem_instruction_with_imm32
      ta_prepare_lwpins:
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_imm32
	call	ta_operand_32or64
	mov	al,[ta_vex_register]
	xchg	al,[ta_postbyte_register]
	mov	[ta_vex_register],al
	ret

ta_xop_single_source_sd_instruction:
	or	[ta_operand_flags],2
	mov	[ta_mmx_size],8
	jmp	ta_xop_instruction_9
ta_xop_single_source_ss_instruction:
	or	[ta_operand_flags],2
	mov	[ta_mmx_size],4
	jmp	ta_xop_instruction_9
ta_xop_single_source_instruction:
	or	[ta_operand_flags],2
	mov	[ta_mmx_size],0
      ta_xop_instruction_9:
	mov	[ta_base_code],al
	mov	[ta_xop_opcode_map],9
	jmp	ta_avx_xop_common
ta_xop_single_source_128bit_instruction:
	or	[ta_operand_flags],2
	mov	[ta_mmx_size],16
	jmp	ta_xop_instruction_9
ta_xop_triple_source_128bit_instruction:
	mov	[ta_immediate_size],-1
	mov	byte [ta_value],0
	mov	[ta_mmx_size],16
	jmp	ta_xop_instruction_8
ta_xop_128bit_instruction:
	mov	[ta_immediate_size],-2
	mov	byte [ta_value],0
	mov	[ta_mmx_size],16
      ta_xop_instruction_8:
	mov	[ta_base_code],al
	mov	[ta_xop_opcode_map],8
	jmp	ta_avx_xop_common
ta_xop_pcom_b_instruction:
	mov	ah,0CCh
	jmp	ta_xop_pcom_instruction
ta_xop_pcom_d_instruction:
	mov	ah,0CEh
	jmp	ta_xop_pcom_instruction
ta_xop_pcom_q_instruction:
	mov	ah,0CFh
	jmp	ta_xop_pcom_instruction
ta_xop_pcom_w_instruction:
	mov	ah,0CDh
	jmp	ta_xop_pcom_instruction
ta_xop_pcom_ub_instruction:
	mov	ah,0ECh
	jmp	ta_xop_pcom_instruction
ta_xop_pcom_ud_instruction:
	mov	ah,0EEh
	jmp	ta_xop_pcom_instruction
ta_xop_pcom_uq_instruction:
	mov	ah,0EFh
	jmp	ta_xop_pcom_instruction
ta_xop_pcom_uw_instruction:
	mov	ah,0EDh
      ta_xop_pcom_instruction:
	mov	byte [ta_value],al
	mov	[ta_immediate_size],-4
	mov	[ta_mmx_size],16
	mov	[ta_base_code],ah
	mov	[ta_xop_opcode_map],8
	jmp	ta_avx_xop_common
ta_vpcmov_instruction:
	or	[ta_vex_required],2
	mov	[ta_immediate_size],-2
	mov	byte [ta_value],0
	mov	[ta_mmx_size],0
	mov	[ta_base_code],al
	mov	[ta_xop_opcode_map],8
	jmp	ta_avx_xop_common
ta_xop_shift_instruction:
	mov	[ta_base_code],al
	or	[ta_vex_required],2
	mov	[ta_xop_opcode_map],9
	call	ta_take_avx_register
	cmp	ah,16
	jne	ta_invalid_operand
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,'['
	je	ta_xop_shift_reg_mem
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
	call	ta_convert_xmm_register
	mov	[ta_vex_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	push	esi
	xor	cl,cl
	xchg	cl,[ta_operand_size]
	lods	byte [esi]
	call	ta_get_size_operator
	pop	esi
	xchg	cl,[ta_operand_size]
	cmp	al,'['
	je	ta_xop_shift_reg_reg_mem
	cmp	al,10h
	jne	ta_xop_shift_reg_reg_imm
	call	ta_take_avx_register
	mov	bl,al
	xchg	bl,[ta_vex_register]
	jmp	ta_nomem_instruction_ready
      ta_xop_shift_reg_reg_mem:
	or	[ta_rex_prefix],8
	lods	byte [esi]
	call	ta_get_size_operator
	call	ta_get_address
	jmp	ta_instruction_ready
      ta_xop_shift_reg_reg_imm:
	xor	bl,bl
	xchg	bl,[ta_vex_register]
	cmp	[ta_base_code],94h
	jae	ta_invalid_operand
	add	[ta_base_code],30h
	mov	[ta_xop_opcode_map],8
	dec	esi
	jmp	ta_mmx_nomem_imm8
      ta_xop_shift_reg_mem:
	call	ta_get_address
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	push	esi
	xor	cl,cl
	xchg	cl,[ta_operand_size]
	lods	byte [esi]
	call	ta_get_size_operator
	pop	esi
	xchg	cl,[ta_operand_size]
	cmp	al,10h
	jne	ta_xop_shift_reg_mem_imm
	call	ta_take_avx_register
	mov	[ta_vex_register],al
	jmp	ta_instruction_ready
      ta_xop_shift_reg_mem_imm:
	cmp	[ta_base_code],94h
	jae	ta_invalid_operand
	add	[ta_base_code],30h
	mov	[ta_xop_opcode_map],8
	dec	esi
	jmp	ta_mmx_imm8

ta_avx512_4vnniw_instruction:
	mov	[ta_opcode_prefix],0F2h
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],38h
	mov	[ta_supplemental_code],al
	mov	[ta_mmx_size],16
	mov	[ta_broadcast_size],0
	or	[ta_vex_required],8
	call	ta_take_avx_register
	mov	[ta_postbyte_register],al
	call	ta_take_avx512_mask
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_avx_register
	mov	[ta_vex_register],al
	cmp	byte [esi],'+'
	jne	ta_reg4_ok
	inc	esi
	cmp	dword [esi],29030128h
	jne	ta_invalid_operand
	lods	dword [esi]
      ta_reg4_ok:
	cmp	[ta_operand_size],64
	jne	ta_invalid_operand_size
	mov	[ta_operand_size],0
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_avx_rm
	jc	ta_invalid_operand
	mov	[ta_operand_size],64
	jmp	ta_instruction_ready

ta_take_tile_register:
	lods	byte [esi]
	cmp	al,14h
	jne	ta_invalid_operand
	lods	byte [esi]
	mov	ah,al
	shr	ah,4
	cmp	ah,7
	jne	ta_invalid_operand
	and	al,1111b
	ret
ta_take_tile_address:
	or	[ta_operand_flags],80h
	mov	[ta_address_size],0
	jmp	ta_get_sib_address_components
ta_amx_int8_instruction:
	mov	ah,5Eh
	jmp	ta_amx_instruction
ta_amx_bf16_instruction:
	mov	ah,5Ch
ta_amx_instruction:
	mov	[ta_opcode_prefix],al
	or	[ta_vex_required],2
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],38h
	mov	[ta_supplemental_code],ah
	call	ta_take_tile_register
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_tile_register
	mov	bl,al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_tile_register
	mov	[ta_vex_register],al
	jmp	ta_nomem_instruction_ready
ta_tilezero_instruction:
	call	ta_take_tile_register
	mov	[ta_postbyte_register],al
	mov	[ta_opcode_prefix],0F2h
	jmp	ta_tile_instruction
ta_tilerelease_instruction:
	mov	[ta_postbyte_register],0
      ta_tile_instruction:
	or	[ta_vex_required],2
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],38h
	mov	[ta_supplemental_code],49h
	xor	bl,bl
	mov	[ta_vex_register],0
	jmp	ta_nomem_instruction_ready
ta_ldtilecfg_instruction:
	mov	[ta_opcode_prefix],al
	or	[ta_vex_required],2
	mov	[ta_supplemental_code],49h
	mov	ah,38h
	xor	al,al
	mov	cl,64
	jmp	ta_xsave_common
ta_tileloadd_instruction:
	call	ta_tileloadd_setup
	call	ta_take_tile_register
	mov	[ta_postbyte_register],al
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_tile_address
	jmp	ta_instruction_ready
      ta_tileloadd_setup:
	mov	[ta_opcode_prefix],al
	or	[ta_vex_required],2
	mov	[ta_base_code],0Fh
	mov	[ta_extended_code],38h
	mov	[ta_supplemental_code],4Bh
	ret
ta_tilestored_instruction:
	call	ta_tileloadd_setup
	call	ta_take_tile_address
	lods	byte [esi]
	cmp	al,','
	jne	ta_invalid_operand
	call	ta_take_tile_register
	mov	[ta_postbyte_register],al
	jmp	ta_instruction_ready

ta_set_evex_mode:
	mov	[ta_evex_mode],al
	jmp	ta_instruction_assembled

ta_take_avx_register:
	lods	byte [esi]
	call	ta_get_size_operator
	cmp	al,10h
	jne	ta_invalid_operand
	lods	byte [esi]
ta_convert_avx_register:
	mov	ah,al
	and	al,1Fh
	and	ah,0E0h
	sub	ah,60h
	jb	ta_invalid_operand
	jz	ta_avx512_register_size
	sub	ah,60h
	jb	ta_invalid_operand
	jnz	ta_avx_register_size_ok
	mov	ah,16
	jmp	ta_avx_register_size_ok
      ta_avx512_register_size:
	mov	ah,64
      ta_avx_register_size_ok:
	cmp	al,8
	jb	ta_match_register_size
	cmp	[ta_code_type],64
	jne	ta_invalid_operand
	jmp	ta_match_register_size
ta_store_vex_instruction_code:
	test	[ta_rex_prefix],10h
	jnz	ta_invalid_operand
	test	[ta_vex_required],0F8h
	jnz	ta_store_evex_instruction_code
	test	[ta_vex_register],10000b
	jnz	ta_store_evex_instruction_code
	cmp	[ta_operand_size],64
	je	ta_store_evex_instruction_code
	mov	al,[ta_base_code]
	cmp	al,0Fh
	jne	ta_store_xop_instruction_code
	test	[ta_vex_required],2
	jnz	ta_prepare_vex
	cmp	[ta_evex_mode],0
	je	ta_prepare_vex
	cmp	[ta_displacement_compression],1
	jne	ta_prepare_vex
	cmp	edx,80h
	jb	ta_prepare_vex
	cmp	edx,-80h
	jae	ta_prepare_vex
	mov	al,bl
	or	al,bh
	shr	al,4
	cmp	al,2
	je	ta_prepare_vex
	call	ta_compress_displacement
	cmp	[ta_displacement_compression],2
	ja	ta_prepare_evex
	jb	ta_prepare_vex
	dec	[ta_displacement_compression]
	mov	edx,[ta_uncompressed_displacement]
	promote_edx
      ta_prepare_vex:
	mov	ah,[ta_extended_code]
	cmp	ah,38h
	je	ta_store_vex_0f38_instruction_code
	cmp	ah,3Ah
	je	ta_store_vex_0f3a_instruction_code
	test	[ta_rex_prefix],1011b
	jnz	ta_store_vex_0f_instruction_code
	mov	[edi+2],ah
	mov	byte [edi],0C5h
	mov	al,[ta_vex_register]
	not	al
	shl	al,3
	mov	ah,[ta_rex_prefix]
	shl	ah,5
	and	ah,80h
	xor	al,ah
	call	ta_get_vex_lpp_bits
	mov	[edi+1],al
	call	ta_check_vex
	add	edi,3
	ret
      ta_get_vex_lpp_bits:
	cmp	[ta_operand_size],32
	jne	ta_get_vex_pp_bits
	or	al,100b
      ta_get_vex_pp_bits:
	mov	ah,[ta_opcode_prefix]
	cmp	ah,66h
	je	ta_vex_66
	cmp	ah,0F3h
	je	ta_vex_f3
	cmp	ah,0F2h
	je	ta_vex_f2
	test	ah,ah
	jnz	ta_disallowed_combination_of_registers
	ret
      ta_vex_f2:
	or	al,11b
	ret
      ta_vex_f3:
	or	al,10b
	ret
      ta_vex_66:
	or	al,1
	ret
      ta_store_vex_0f38_instruction_code:
	mov	al,11100010b
	mov	ah,[ta_supplemental_code]
	jmp	ta_make_c4_vex
      ta_store_vex_0f3a_instruction_code:
	mov	al,11100011b
	mov	ah,[ta_supplemental_code]
	jmp	ta_make_c4_vex
      ta_store_vex_0f_instruction_code:
	mov	al,11100001b
      ta_make_c4_vex:
	mov	[edi+3],ah
	mov	byte [edi],0C4h
	mov	ah,[ta_rex_prefix]
	shl	ah,5
	xor	al,ah
	mov	[edi+1],al
	call	ta_check_vex
	mov	al,[ta_vex_register]
	xor	al,1111b
	shl	al,3
	mov	ah,[ta_rex_prefix]
	shl	ah,4
	and	ah,80h
	or	al,ah
	call	ta_get_vex_lpp_bits
	mov	[edi+2],al
	add	edi,4
	ret
      ta_check_vex:
	cmp	[ta_code_type],64
	je	ta_vex_ok
	not	al
	test	al,11000000b
	jnz	ta_invalid_operand
	test	[ta_rex_prefix],40h
	jnz	ta_invalid_operand
      ta_vex_ok:
	ret
ta_store_xop_instruction_code:
	mov	[edi+3],al
	mov	byte [edi],8Fh
	mov	al,[ta_xop_opcode_map]
	mov	ah,[ta_rex_prefix]
	test	ah,40h
	jz	ta_xop_ok
	cmp	[ta_code_type],64
	jne	ta_invalid_operand
      ta_xop_ok:
	not	ah
	shl	ah,5
	xor	al,ah
	mov	[edi+1],al
	mov	al,[ta_vex_register]
	xor	al,1111b
	shl	al,3
	mov	ah,[ta_rex_prefix]
	shl	ah,4
	and	ah,80h
	or	al,ah
	call	ta_get_vex_lpp_bits
	mov	[edi+2],al
	add	edi,4
	ret
ta_store_evex_instruction_code:
	test	[ta_vex_required],2
	jnz	ta_invalid_operand
	cmp	[ta_base_code],0Fh
	jne	ta_invalid_operand
	cmp	[ta_displacement_compression],1
	jne	ta_prepare_evex
	call	ta_compress_displacement
      ta_prepare_evex:
	mov	ah,[ta_extended_code]
	cmp	ah,38h
	je	ta_store_evex_0f38_instruction_code
	cmp	ah,3Ah
	je	ta_store_evex_0f3a_instruction_code
	mov	al,11110001b
      ta_make_evex:
	mov	[edi+4],ah
	mov	byte [edi],62h
	mov	ah,[ta_rex_prefix]
	shl	ah,5
	xor	al,ah
	mov	ah,[ta_vex_required]
	and	ah,10h
	xor	al,ah
	mov	[edi+1],al
	call	ta_check_vex
	mov	al,[ta_vex_register]
	not	al
	and	al,1111b
	shl	al,3
	mov	ah,[ta_rex_prefix]
	shl	ah,4
	or	ah,[ta_rex_prefix]
	and	ah,80h
	or	al,ah
	or	al,100b
	call	ta_get_vex_pp_bits
	mov	[edi+2],al
	mov	al,[ta_vex_register]
	not	al
	shr	al,1
	and	al,1000b
	test	[ta_vex_required],80h
	jne	ta_evex_rounding
	mov	ah,[ta_operand_size]
	cmp	ah,16
	jbe	ta_evex_l_ok
	or	al,ah
	jmp	ta_evex_l_ok
      ta_evex_rounding:
	mov	ah,[ta_rounding_mode]
	shl	ah,5
	or	al,ah
      ta_evex_l_ok:
	test	[ta_vex_required],20h
	jz	ta_evex_zaaa_ok
	or	al,[ta_mask_register]
      ta_evex_zaaa_ok:
	test	[ta_vex_required],40h
	jz	ta_evex_b_ok
	or	al,10h
      ta_evex_b_ok:
	mov	[edi+3],al
	add	edi,5
	ret
      ta_store_evex_0f38_instruction_code:
	mov	al,11110010b
	mov	ah,[ta_supplemental_code]
	jmp	ta_make_evex
      ta_store_evex_0f3a_instruction_code:
	mov	al,11110011b
	mov	ah,[ta_supplemental_code]
	jmp	ta_make_evex
ta_compress_displacement:
	mov	ebp,ecx
	mov	[ta_uncompressed_displacement],edx
	or	edx,edx
	jz	ta_displacement_compressed
	xor	ecx,ecx
	mov	cl,[ta_mmx_size]
	test	cl,cl
	jnz	ta_calculate_displacement_scale
	mov	cl,[ta_operand_size]
      ta_calculate_displacement_scale:
	bsf	ecx,ecx
	jz	ta_displacement_compression_ok
	xor	eax,eax
	shrd	eax,edx,cl
	jnz	ta_displacement_not_compressed
	sar	edx,cl
	cmp	edx,80h
	jb	ta_displacement_compressed
	cmp	edx,-80h
	jnb	ta_displacement_compressed
	shl	edx,cl
      ta_displacement_not_compressed:
	inc	[ta_displacement_compression]
	jmp	ta_displacement_compression_ok
      ta_displacement_compressed:
	add	[ta_displacement_compression],2
      ta_displacement_compression_ok:
	mov	ecx,ebp
	ret
