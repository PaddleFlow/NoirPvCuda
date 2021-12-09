.code

RtlCaptureContext proc

	pushfq
	; Save Segment Selectors.
	mov word ptr [rcx+38h],cs
	mov word ptr [rcx+3Ah],ds
	mov word ptr [rcx+3Ch],es
	mov word ptr [rcx+3Eh],fs
	mov word ptr [rcx+40h],gs
	mov word ptr [rcx+42h],ss
	; Save General-Purpose Registers.
	mov qword ptr [rcx+78h],rax
	mov qword ptr [rcx+80h],rcx
	mov qword ptr [rcx+88h],rdx
	mov qword ptr [rcx+90h],rbx
	lea rax,[rsp+10h]		; rsp needs special treatments.
	mov qword ptr [rcx+98h],rax
	mov qword ptr [rcx+0A0h],rbp
	mov qword ptr [rcx+0A8h],rsi
	mov qword ptr [rcx+0B0h],rdi
	mov qword ptr [rcx+0B8h],r8
	mov qword ptr [rcx+0C0h],r9
	mov qword ptr [rcx+0C8h],r10
	mov qword ptr [rcx+0D0h],r11
	mov qword ptr [rcx+0D8h],r12
	mov qword ptr [rcx+0E0h],r13
	mov qword ptr [rcx+0E8h],r14
	mov qword ptr [rcx+0F0h],r15
	; Save x87 Floating-Point State.
	fxsave [rcx+100h]
	; Save Streaming SIMD Extension State.
	movaps xmmword ptr [rcx+1A0h],xmm0
	movaps xmmword ptr [rcx+1B0h],xmm1
	movaps xmmword ptr [rcx+1C0h],xmm2
	movaps xmmword ptr [rcx+1D0h],xmm3
	movaps xmmword ptr [rcx+1E0h],xmm4
	movaps xmmword ptr [rcx+1F0h],xmm5
	movaps xmmword ptr [rcx+200h],xmm6
	movaps xmmword ptr [rcx+210h],xmm7
	movaps xmmword ptr [rcx+220h],xmm8
	movaps xmmword ptr [rcx+230h],xmm9
	movaps xmmword ptr [rcx+240h],xmm10
	movaps xmmword ptr [rcx+250h],xmm11
	movaps xmmword ptr [rcx+260h],xmm12
	movaps xmmword ptr [rcx+270h],xmm13
	movaps xmmword ptr [rcx+280h],xmm14
	movaps xmmword ptr [rcx+290h],xmm15
	stmxcsr dword ptr [rcx+34h]
	; Save the Instruction Pointer.
	mov rax,[rsp+8]
	mov [rcx+0F8h],rax
	; Save the Flags Register.
	pop rax
	mov dword ptr [rcx+44h],eax
	; Return
	ret

RtlCaptureContext endp

RtlRestoreContext proc

	; Setup iret instruction stack.
	sub rsp,28h
	mov rax,qword ptr [rcx+0F8h]
	mov qword ptr[rsp],rax			; Restore RIP
	mov ax,word ptr [rcx+38h]
	mov word ptr [rsp+8],ax			; Restore CS
	mov eax,dword ptr [rcx+44h]
	mov qword ptr [rsp+10h],rax		; Restore RFlags
	mov rax,qword ptr [rcx+98h]
	mov qword ptr [rsp+18h],rax		; Restore RSP
	mov ax,word ptr [rcx+42h]
	mov word ptr[rsp+20h],ax		; Restore SS
	; Restore Segment Selectors.
	mov ds,word ptr [rcx+3Ah]
	mov es,word ptr [rcx+3Ch]
	mov fs,word ptr [rcx+3Eh]
	mov gs,word ptr [rcx+40h]
	; The cs and ss selectors are restored on iret instruction.
	; Restore General-Purpose Registers, except rcx and rip.
	mov rax,qword ptr [rcx+78h]
	mov rdx,qword ptr [rcx+88h]
	mov rbx,qword ptr [rcx+90h]
	mov rbp,qword ptr [rcx+0A0h]
	mov rsi,qword ptr [rcx+0A8h]
	mov rdi,qword ptr [rcx+0B0h]
	mov r8,qword ptr [rcx+0B8h]
	mov r9,qword ptr [rcx+0C0h]
	mov r10,qword ptr [rcx+0C8h]
	mov r11,qword ptr [rcx+0D0h]
	mov r12,qword ptr [rcx+0D8h]
	mov r13,qword ptr [rcx+0E0h]
	mov r14,qword ptr [rcx+0E8h]
	mov r15,qword ptr [rcx+0F0h]
	; Restore x87 Floating-Point State.
	fxrstor [rcx+100h]
	; Restore Streaming SIMD Extension State.
	movaps xmm0,xmmword ptr [rcx+1A0h]
	movaps xmm1,xmmword ptr [rcx+1B0h]
	movaps xmm2,xmmword ptr [rcx+1C0h]
	movaps xmm3,xmmword ptr [rcx+1D0h]
	movaps xmm4,xmmword ptr [rcx+1E0h]
	movaps xmm5,xmmword ptr [rcx+1F0h]
	movaps xmm6,xmmword ptr [rcx+200h]
	movaps xmm7,xmmword ptr [rcx+210h]
	movaps xmm8,xmmword ptr [rcx+220h]
	movaps xmm9,xmmword ptr [rcx+230h]
	movaps xmm10,xmmword ptr [rcx+240h]
	movaps xmm11,xmmword ptr [rcx+250h]
	movaps xmm12,xmmword ptr [rcx+260h]
	movaps xmm13,xmmword ptr [rcx+270h]
	movaps xmm14,xmmword ptr [rcx+280h]
	movaps xmm15,xmmword ptr [rcx+290h]
	ldmxcsr dword ptr [rcx+34h]
	; Last but not least, restore the rcx register
	mov rcx,qword ptr [rcx+80h]
	; Finally, execute iret to restore cs:rip, ss:rsp and rflags.
	iret

RtlRestoreContext endp

end