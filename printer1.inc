;========================================================================
; printer2.inc - Parallel printer support (part 1 of 2)
;	INT 17h, function AH=01h 
;	detect_parallel
;	- see printer2.inc for other INT 17h functions
;-------------------------------------------------------------------------
;
; Compiles with NASM 2.11.08, might work with other versions
;
; Copyright (C) 2010 - 2019 Sergey Kiselev.
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

num_parallel	equ	3		; number of parallel ports

prn_data_reg	equ	0		; printer data register (out)

prn_stat_reg	equ	1		; printer status register (in)
prn_stat_bits	equ	0F8h		; used bits in status register
prn_stat_invert	equ	048h		; bits 6 and 3 need to be inverted
prn_stat_busy	equ	80h		; 0 = printer is busy (1 = ready)
prn_stat_tmout	equ	01h		; 1 = timeout (software error)

prn_ctrl_reg	equ	2		; printer control register (out)
prn_ctrl_sel	equ	08h		; 1 = select printer
prn_ctrl_init	equ	04h		; 0 = initialize printer (1 = normal)
prn_ctrl_autolf	equ	02h		; 1 = auto linefeed
prn_ctrl_strobe	equ	01h		; 1 = strobe active

prn_init_delay	equ	2485		; printer initialization delay, 75 ms

;=========================================================================
; int_17_fn01 - Initialize printer port
; Input:
;	AH = 1 - function 01h - initialize printer port
;	DX = printer port number (0-2)
; Output:
;	AH = printer status (see int_17_fn00 for complete description)
;-------------------------------------------------------------------------
int_17_fn01:
	inc	dx
	inc	dx			; DX = prn_ctrl_reg
					; OPTIMIZATION:
					; prn_ctrl_reg = prn_data_reg + 2
	mov	al,prn_ctrl_sel		; init bit = 0, initialize printer
	out	dx,al
	push	cx
	mov	cx,prn_init_delay
	call	delay_15us
	pop	cx
	mov	al,(prn_ctrl_sel | prn_ctrl_init)
	out	dx,al			; deactivate init bit
	dec	dx			; OPTIMIZATION:
					; prn_stat_reg = prn_ctrl_reg - 1
	jmp	int_17_status		; exit returning status

;=========================================================================
; detect_parallel - detect and print parallel ports
; Input:
;	none
; Output:
;	none (updates BIOS data area)
; Notes:
; 	- If parallel port is found at 3BCh the following parallel port
;	  enumeration is used:
;	  LPT1 - 3BCh, LPT2 - 378h, LPT3 - 278h
;	- If parallel port is not found at 3BCh, the enumeration is:
;	  LPT1 - 378h, LPT2 - 278h, LPT3 - none
;-------------------------------------------------------------------------
detect_parallel:
	push	ax
	push	bx
	push	dx
	push	si
	mov	al,e_parallel_scan
	out	post_reg,al

	mov	bx,1414h		; default timeout is 20 seconds
	mov	word [printer_timeout],bx   ; initialize timeout values
	mov	word [printer_timeout+2],bx ; for all 4 ports
	xor	bx,bx			; BX = 0
	xor	di,di			; DI = 0
.detect_loop:
    cs	mov	dx,word [parallel_base+di] ; DX = parallel port address
	mov	al,05Ah			; just some test value
	out	dx,al			; write it to the printer data register
	mov	al,0FFh
	out	unused_reg,al		; set ISA data bus floating signals
	in	al,dx
	cmp	al,05Ah			; compare with the test value
	jne	.no_port		; no port at this address
	mov	word [equip_parallel+bx],dx ; store port address to the BIOS
					; data area
	add	word [equipment_list],4000h ; increment number of printer ports
	jmp	.next

.no_port:
	mov	word [equip_parallel+bx],0 ; no port
	or	di,di			; no printer port at 3BCh?
	jz	.next_skip_3BC		; don't increment LPTx number
.next:
	inc	bx
	inc	bx

.next_skip_3BC:
	inc	di
	inc	di
	cmp	di,(num_parallel << 1)	; all parallel ports have been scanned?
	je	.print			; done with detection, print findings
	jmp	.detect_loop

.print:
	mov	si,msg_parallel
	call	print
	xor	bx,bx

.print_loop:
	mov	si,msg_parallel_lpt
	call	print
	mov	al,bl
	shr	al,1
	inc	al			; AL = LPT port number (1..3)
	call	print_digit
	mov	si,msg_colon
	call	print
	mov	ax,word [equip_parallel+bx] ; get parallel port address
	or	ax,ax			; parallel interface installed?
	jz	.print_no_port
	call	print_hex
	jmp	.print_next

.print_no_port:
	mov	si,msg_none
	call	print

.print_next:
	inc	bx
	inc	bx
	cmp	bx,(num_parallel << 1)	; all parallel ports have been printed?
	je	.done
	mov	si,msg_semicolon
	call	print
	jmp	.print_loop

.done:
	mov	si,msg_crlf
	call	print

	pop	si
	pop	dx
	pop	bx
	pop	ax
	ret

;-------------------------------------------------------------------------
; parallel port base addresses - used for port detection
parallel_base	dw	3BCh, 378h, 278h
