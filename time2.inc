;=========================================================================
; time2.int - BIOS Time Services
;       INT 1Ah - BIOS Time Serivces
;		dispatcher
;       INT 08h - IRQ0 interrupt handler (timer interrupt)
;-------------------------------------------------------------------------
;
; Compiles with NASM 2.07, might work with other versions
;
; Copyright (C) 2011 - 2012 Sergey Kiselev.
; Provided for hobbyist use on the Xi 8088 board.
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

;=========================================================================
; int_1A - BIOS Time Services
; Note: see time1.inc for functions implementation
;-------------------------------------------------------------------------
	setloc	0FE6Eh			; INT 1A Entry Point
int_1A:
	push	bx
	push	ds
	mov	bx,biosdseg
	mov	ds,bx
	cmp	ah,.max/2
	jae	int_1A_exit
	mov	bh,0
	mov	bl,ah
	shl	bx,1
    cs	jmp	near [.dispatch+bx]
.dispatch:
	dw	int_1A_fn00
	dw	int_1A_fn01
	dw	int_1A_fn02
	dw	int_1A_fn03
	dw	int_1A_fn04
	dw	int_1A_fn05
	dw	int_1A_fn06
	dw	int_1A_fn07
.max	equ	$-.dispatch
int_1A_exit:
	pop	ds
	pop	bx
	iret
int_1A_exitf:
	pop	ds
	pop	bx
	retf	2

;=========================================================================
; int_08 - IRQ0 ISR, called approximately every 55ms
;-------------------------------------------------------------------------
	setloc	0FEA5h			; INT 08 Entry Point
int_08:
	push	ax
	push	ds
	mov	ax,biosdseg
	mov	ds,ax
	cmp	byte [fdc_motor_tout],0
	jz	.1
	dec	byte [fdc_motor_tout]
	jnz	.1
	and	byte [fdc_motor_state],0F0h ; update fdc_motor_state byte
	mov	al,0Ch			; turn off motors, enable DMA + IRQ
	push	dx
	mov	dx,fdc_dor_reg		; write it to Digital Output register
	out	dx,al
	pop	dx
.1:
	inc	word [ticks_lo]
	jnz	.2
	inc	word [ticks_hi]
.2:
	cmp	word [ticks_hi],18h	; 1573042 ticks in one day
	jnz	.3			; which is 65536 * 24 + 178 or
	cmp	word [ticks_lo],0B2h	; 10000h * 18h + 0B2h
	jnz	.3
	mov	word [ticks_hi],0
	mov	word [ticks_lo],0
	mov	word [new_day],1
.3:
	int	1Ch			; User timer interrupt
	mov	al,20h
	out	pic1_reg0,al
	pop	ds
	pop	ax
	iret
