;=========================================================================
; delay.inc - Delay functions
;-------------------------------------------------------------------------
;
; Compiles with NASM 2.13.02, might work with other versions
;
; Copyright (C) 2010 - 2023 Sergey Kiselev.
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
%ifdef PIT_DELAY

;=========================================================================
; delay_15us - delay for multiplies of 15 microseconds using PIT
; Input:
;	CX = time to delay (in 15 microsecond units)
;
; - Calculate the total number of PIT ticks necessary
;	1,193,182 / 1000 ms / 1000 us / * 15 us * 2 = ~ 36 ticks/us
; - Latch the PIT and draw down the countdown total on each read.
; - Exit when countdown underflows.
;
; Note: Mode 3 (Square Wave) decements the readable counter by 2, so the
; effective frequency of the counter is actually 2,386,360 Hz.
;
; Contributed by @640-KB (under GPL-3.0 license)
; Based on contribution by @Raffzahn (under CC BY-SA 4.0):
; https://retrocomputing.stackexchange.com/a/24874/21323
;-------------------------------------------------------------------------
delay_15us:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,2*15*pic_freq/1000/1000+1	; ~ 36 ticks/us	
	mul	cx		; dx:ax = countdown of pit ticks to wait
	xchg	ax,bx		; dx:bx = countdown ticks
	call	io_wait_latch	; ax = start read
	mov	cx,ax		; cx = last read
.tick_loop:
	call	io_wait_latch	; ax = current counter reading
	sub	cx,ax		; cx = # of ticks elapsed since last reading
	sub	bx,cx		; subtract change in ticks from countdown
	mov	cx,ax		; cx = save the last read
	sbb	dx,0		; borrow out of high word (if necessary)
	jae	.tick_loop	; loop while countdown >= 0
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret

;=========================================================================
; Latch PIT 0 and read counter
; Output:
;	AX = current counter
;-------------------------------------------------------------------------
io_wait_latch:
	mov	al,0		; counter 0, latch (00b)
	pushf			; save current IF
	cli			; disable interrupts
	out	pit_ctl_reg,al	; write command to ctc
	in	al,pit_ch0_reg	; read low byte of counter 0 latch
	mov	ah,al		; save it
	in	al,pit_ch0_reg	; read high byte of counter 0 latch
	popf			; restore IF state
	xchg	al,ah		; convert endian
	ret

%else ; LOOP_DELAY

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

%endif ; PIT_DELAY
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
