esp equ +rsp

macro pushD [arg]
{
	common
		local offset,total
		offset = 0
		lea rsp,[rsp-total]
	forward
		offset = offset + 4
		if arg eqtype eax
			mov dword [rsp+total-offset],arg
		else
			mov r8d,dword arg
			mov [rsp+total-offset],r8d
		end if
	common
		total = offset
}

macro popD [arg]
{
	common
		local offset
		offset = 0
	forward
		if arg eqtype [mem]
			mov r8d,[rsp+offset]
			mov dword arg,r8d
		else
			mov arg,dword [rsp+offset]
		end if
		offset = offset + 4
	common
		lea rsp,[rsp+offset]
}

macro add dest,src
{
	if dest eq esp
		add rsp,src
	else
		add dest,src
	end if
}

macro mov dest,src
{
	if src eq esp
		mov dest,ESP
	else
		mov dest,src
	end if
}

macro cmp dest,src
{
	if dest eq esp
		cmp ESP,src
	else
		cmp dest,src
	end if
}

macro use32
{

	macro push args
	\{
		local list,arg,status
		define list
		define arg
		irps sym, args \\{
			define status
			match =dword, sym \\\{
				define status :
			\\\}
			match [any, status arg sym \\\{
				define arg [any
				match [mem], arg \\\\{
					match previous, list \\\\\{ define list previous,[mem] \\\\\}
					match , list \\\\\{ define list [mem] \\\\\}
					define arg
				\\\\}
				define status :
			\\\}
			match [, status arg sym \\\{
				define arg [
				define status :
			\\\}
			match , status \\\{
				match previous, list \\\\{ define list previous,sym \\\\}
				match , list \\\\{ define list sym \\\\}
			\\\}
		\\}
		match arg, list \\{ pushD arg \\}
	\}

	macro pop args
	\{
		local list,arg,status
		define list
		define arg
		irps sym, args \\{
			define status
			match =dword, sym \\\{
				define status :
			\\\}
			match [any, status arg sym \\\{
				define arg [any
				match [mem], arg \\\\{
					match previous, list \\\\\{ define list previous,[mem] \\\\\}
					match , list \\\\\{ define list [mem] \\\\\}
					define arg
				\\\\}
				define status :
			\\\}
			match [, status arg sym \\\{
				define arg [
				define status :
			\\\}
			match , status \\\{
				match previous, list \\\\{ define list previous,sym \\\\}
				match , list \\\\{ define list sym \\\\}
			\\\}
		\\}
		match arg, list \\{ popD arg \\}
	\}

	macro jmp arg
	\{
		if arg eq near eax
			jmp near rax
		else if arg eq near edx
			jmp near rdx
		else if arg eqtype [mem]
			mov r8d,arg
			jmp near r8
		else
			jmp arg
		end if
	\}

	macro call arg
	\{
		if 1
		match =near =dword [mem], arg \\{
			mov r8d,[mem]
			call near r8
		else
		\\}
			call arg
		end if
	\}

	macro salc		; for tinyasm's core it does not need to preserve flags
	\{
		setc al
		neg al
	\}

	macro jcxz target	; for tinyasm's core it does not need to preserve flags
	\{
		test cx,cx
		jz target
	\}

	use64

}

macro use16
{

	purge push,pop,jmp,call,salc,jcxz

	use16

}

; Pointer-promotion helpers: after loading a 32-bit pointer from a dd state field,
; call promote_ptr to eliminate address-size override prefix (0x67) in 64-bit mode.
; mov esi,[dd_field] implicitly zero-extends to rsi, but assembler still emits 0x67
; when [esi] is used as addressing base. Zero-extend explicitly fixes this.
macro promote_esi { mov esi,esi }
macro promote_edi { mov edi,edi }
macro promote_ebx { mov ebx,ebx }
macro promote_ecx { mov ecx,ecx }
macro promote_edx { mov edx,edx }

use32
