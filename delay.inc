;=========================================================================
; delay.inc - Delay functions
;-------------------------------------------------------------------------
;
; Compiles with NASM 2.11.08, might work with other versions
;
; Copyright (C) 2011 - 2017 Sergey Kiselev.
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
;=========================================================================

%ifdef AT_DELAY

;=========================================================================
; delay_15us - delay for multiplies of 15 microseconds
; Input:
;	CX = time to delay (in 15 microsecond units)
; Notes:
;	1.  Actual delay will be between (CX - 1) * 15us and CX * 15us
;	2.  This relies on the "refresh" bit of port 61h and therefore on
;	    timer channel 1. Will not function properly if timer gets
;	    reprogrammed by an application or if it was not initialized yet
;-------------------------------------------------------------------------
delay_15us:
	push	ax
	push	cx
.zero:
	in	al,ppi_pb_reg
	test	al,refresh_flag
	jz	.zero
	dec	cx
	jz	.exit
.one:
	in	al,ppi_pb_reg
	test	al,refresh_flag
	jnz	.one
	dec	cx
	jnz	.zero
.exit:
	pop	cx
	pop	ax
	ret

%else ; AT_DELAY

;=========================================================================
; delay_15us - delay for multiplies of approximately 15 microseconds
; Input:
;	CX = time to delay (in 15 microsecond units)
; Notes:
;	This implementation does not provide precise timing
;	The actual delay depends on the CPU clock frequency
;-------------------------------------------------------------------------
delay_15us:
	push	ax
	push	cx
.1:
	mov	al,4
.2:
	dec	al
	jnz	.2
	loop	.1
	pop	cx
	pop	ax
	ret

%endif ; AT_DELAY


%if 0
;=========================================================================
; divide_32 - divide 64-bit argument by 32-bit, return 64-bit result
; Input:
;	DX:AX	- dividend
;	CX	- divisor
; Output:
;	DX:AX	- quotient
;-------------------------------------------------------------------------
divide_32:
	or	dx,dx
	jnz	.1
	div	cx
	xor	dx,dx
	ret
.1:
	push	bx
	mov	bx,ax
	mov	ax,dx
	xor	dx,dx
	div	cx
	xchg	bx,ax
	div	cx
	mov	dx,bx
	pop	bx
	ret
%endif ; 0
