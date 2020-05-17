;========================================================================
; inttrace.asm -- Interrupt tracing
;------------------------------------------------------------------------
;
; Compiles with NASM 2.11.08, might work with other versions
;
; Copyright (C) 2010 - 2020 Sergey Kiselev.
; Provided for hobbyist use on the Xi 8088 and Micro 8088 boards.
;  
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <http://www.gnu.org/licenses/>.
;
; TODO:
;========================================================================

offset_BP	equ	0
offset_ES	equ	offset_BP+2
offset_DS	equ	offset_ES+2
offset_SI	equ	offset_DS+2
offset_DX	equ	offset_SI+2
offset_CX	equ	offset_DX+2
offset_BX	equ	offset_CX+2
offset_AX	equ	offset_BX+2
offset_SFLAGS	equ	offset_AX+2
offset_RET	equ	offset_SFLAGS+2
offset_IP	equ	offset_RET+2
offset_CS	equ	offset_IP+2
offset_FLAGS	equ	offset_CS+2
original_SP	equ	offset_FLAGS+2

;========================================================================
; int_trace - print registers at interrupt service routine
;========================================================================
	global	int_trace
int_trace:
	pushf
	push	ax
	push	bx
	push	cx
	push	dx
	push	si
	push	ds
	push	es
	push	bp
	mov	bp,sp

	xor	ax,ax			; AX = 0
	lea	si,[offset_IP+bp]	; SI = interrupt return CS:IP

.test_int:
    ss	lds	bx,[si]			; DS:BX = caller's CS:IP
	cmp	byte [bx-2], 0CDh	; int opcode
	jne	.test_call_ptr		; not an int opcode
	mov	al,byte[bx-1]		; interrupt vector
	jmp	.print_regs

.test_call_ptr:
	cmp	word [bx-4], 1EFFh	; interupt emulation - call dword ptr
	jne	.print_regs
	inc	ah

.print_regs:
	mov	si,msg_regs
	call	print
	call	print_hex		; interrupt vector
	call	print_space
	mov	ax,word [offset_AX+bp]
	call	print_hex		; AX
	call	print_space
	mov	ax,word [offset_BX+bp]
	call	print_hex		; BX
	call	print_space
	mov	ax,cx
	call	print_hex		; CX
	call	print_space
	mov	ax,dx
	call	print_hex		; DX
	call	print_space
	mov	ax,word [offset_BP+bp]
	call	print_hex		; BP
	call	print_space
	mov	ax,word [offset_SI+bp]
	call	print_hex		; SI
	call	print_space
	mov	ax,di
	call	print_hex		; DI
	call	print_space
	mov	ax,word [offset_DS+bp]
	call	print_hex		; DS
	call	print_space
	mov	ax,es
	call	print_hex		; ES
	call	print_space
	mov	ax,word [offset_CS+bp]
	call	print_hex		; CS
	call	print_space
	mov	ax,word [offset_IP+bp]
	call	print_hex		; IP
	call	print_space
	mov	ax,ss
	call	print_hex		; SS
	call	print_space
	mov	ax,bp
	add	ax,original_SP
	call	print_hex
	call	print_space
	mov	ax,word [offset_FLAGS+bp]
	call	print_hex		; FLAGS
	mov	ax,0E0Dh
	mov	bl,07h
	int	10h
	mov	ax,0E0Ah
	mov	bl,07h
	int	10h

	pop	bp
	pop	es
	pop	ds
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	popf
	ret

print_space:
	push	ax
	push	bx
	mov	ax,0E20h
	mov	bl,0Fh
	int	10h
	pop	bx
	pop	ax
	ret


;=========================================================================

msg_regs:
	db	'INT  AX   BX   CX   DX   BP   SI   DI   DS   ES   CS   IP   SS   SP   Flags', 0Dh, 0Ah, 00h
