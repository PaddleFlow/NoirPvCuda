.code

extern PvDummyPrinter:proc

gpr_stack_size equ 80h

; Macro for pushing all GPRs to stack.
pushaq_fast macro rsp_offset
	
	mov qword ptr [rsp+rsp_offset+00h],rax
	mov qword ptr [rsp+rsp_offset+08h],rcx
	mov qword ptr [rsp+rsp_offset+10h],rdx
	mov qword ptr [rsp+rsp_offset+18h],rbx
	mov qword ptr [rsp+rsp_offset+28h],rbp
	mov qword ptr [rsp+rsp_offset+30h],rsi
	mov qword ptr [rsp+rsp_offset+38h],rdi
	mov qword ptr [rsp+rsp_offset+40h],r8
	mov qword ptr [rsp+rsp_offset+48h],r9
	mov qword ptr [rsp+rsp_offset+50h],r10
	mov qword ptr [rsp+rsp_offset+58h],r11
	mov qword ptr [rsp+rsp_offset+60h],r12
	mov qword ptr [rsp+rsp_offset+68h],r13
	mov qword ptr [rsp+rsp_offset+70h],r14
	mov qword ptr [rsp+rsp_offset+78h],r15
	
endm

pushaq macro
	
	sub rsp,gpr_stack_size
	pushaq_fast 0h
	
endm

; Macro for popping all GPRs from stack.
popaq_fast macro rsp_offset

	mov rax,qword ptr [rsp+rsp_offset]
	mov rcx,qword ptr [rsp+rsp_offset+8]
	mov rdx,qword ptr [rsp+rsp_offset+10h]
	mov rbx,qword ptr [rsp+rsp_offset+18h]
	mov rbp,qword ptr [rsp+rsp_offset+28h]
	mov rsi,qword ptr [rsp+rsp_offset+30h]
	mov rdi,qword ptr [rsp+rsp_offset+38h]
	mov r8, qword ptr [rsp+rsp_offset+40h]
	mov r9, qword ptr [rsp+rsp_offset+48h]
	mov r10,qword ptr [rsp+rsp_offset+50h]
	mov r11,qword ptr [rsp+rsp_offset+58h]
	mov r12,qword ptr [rsp+rsp_offset+60h]
	mov r13,qword ptr [rsp+rsp_offset+68h]
	mov r14,qword ptr [rsp+rsp_offset+70h]
	mov r15,qword ptr [rsp+rsp_offset+78h]

endm


popaq macro

	popaq_fast 0h
	add rsp,gpr_stack_size

endm

PvDummyInterrupt proc

	; Print a string in order to respond to this dummy interrupt.
	; Save all volatile registers.
	pushaq
	call PvDummyPrinter
	popaq
	iretq

PvDummyInterrupt endp

end