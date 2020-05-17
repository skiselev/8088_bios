;=========================================================================
; misc.inc - Miscellaneous BIOS Services
;       INT 15h, functions:
;       	4Fh	- OS hook keyboard intercept
;		90h	- Device busy loop
;		91h	- Interrupt completed
;		0C0h	- Get system configruation
;		0C2h	- PS/2 mouse services (see ps2aux.inc)
;		
;-------------------------------------------------------------------------
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
;=========================================================================

;-------------------------------------------------------------------------
; offsets for registers on stack
int_15_bp	equ	0
int_15_ip	equ	int_15_bp+2
int_15_cs	equ	int_15_ip+2
int_15_flags	equ	int_15_cs+2

;=========================================================================
; int_15 - Miscellaneous BIOS services
; Input:
;	AH = 4Fh - OS hook keyboard intercept
;		- Does nothing
;	AH = 0C2h - PS/2 mouse services
;		- Implemented in ps2aux.inc
;-------------------------------------------------------------------------
	setloc	0F859h			; INT 15 Entry Point
int_15:
	sti
	cmp	ah,4Fh
	je	int_15_exit		; continue with int 09h ISR
	cmp	ah,0C0h
	je	int_15_fnC0
	cmp	ax,9001h
	je	int_15_os_hook		; diskette - device busy hook
	cmp	ax,90FDh
	je	int_15_os_hook		; diskette - motor start hook
	cmp	ax,9101h
	je	int_15_os_hook		; diskette - interrupt completed

%ifdef PS2_MOUSE
	cmp	ah,0C2h
	jne	.1
	jmp	int_15_fnC2		; jump to PS/2 mouse handler
.1:
%endif

	mov	ah,86h			; no cassete present

int_15_err:
	push	bp
	mov	bp,sp
	or	byte [bp+int_15_flags],1 ; set CF flag
	pop	bp
	iret

int_15_ok:
	push	bp
	mov	bp,sp
	and	byte [bp+int_15_flags],~1 ; clear CF flag
	pop	bp
	iret

int_15_os_hook:
	mov	ah,00h

int_15_exit:
	iret

;=========================================================================
; int_15_fnC0 - Get configuration
; Input:
;	AH = 0C0h - get configuration
; Output:
;	AH = 00h - function supported
;	ES:BX = configuration table (0F000h:0E6F5h)
;	CF = 0 (success)
;-------------------------------------------------------------------------
int_15_fnC0:
	mov	ah,00h
	mov	bx,bioscseg
	mov	es,bx
	mov	bx,config_table
	jmp	int_15_ok
