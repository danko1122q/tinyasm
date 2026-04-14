; tinyasm platform32.inc
; Compatibility macros for pure 32-bit ELF build.
; Unlike platform.inc (which runs core 32-bit code on x64 ELF64),
; this file targets a native 32-bit ELF binary.
; No use32/use64 bridging is needed: the whole binary IS 32-bit.

; No esp alias needed - we're genuinely 32-bit here
; No pushD/popD macros needed - push/pop work natively on 32-bit dwords

; The 'use32' macro below installs the same push/pop/jmp/call/salc/jcxz
; overrides that core code expects (they are identity mappings in real 32-bit mode)
macro use32
{
	; In a real 32-bit binary, push/pop/jmp/call/salc/jcxz work natively.
	; We define them as pass-throughs so the core source compiles unchanged.
	macro push args
	\{
		push args
	\}

	macro pop args
	\{
		pop args
	\}

	macro jmp arg
	\{
		jmp arg
	\}

	macro call arg
	\{
		call arg
	\}

	macro salc
	\{
		salc
	\}

	macro jcxz target
	\{
		jcxz target
	\}

	use32

}

macro use16
{
	purge push,pop,jmp,call,salc,jcxz
	use16
}

use32
