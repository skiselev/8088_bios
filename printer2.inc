;=========================================================================
; printer2.inc - Parallel printer support (part 2 of 2)
;       INT 17h, function AH=00h
;	INT 17h, function AH=02h
;	- see printer1.inc for other INT 17h functions
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

;=========================================================================
; int_17  - BIOS Printer Services
;-------------------------------------------------------------------------
	setloc	0EFD2h			; INT 17 Entry Point
int_17:
	sti
	push	bx
	push	dx
	push	si
	push	ds
	mov	bx,biosdseg
	mov	ds,bx
	cmp	dx,num_parallel
	jae	int_17_error		; invalid port number specified
	mov	si,printer_timeout	; printer port timeout setting in BDA
	add	si,dx			; [SI] = timeout for the selected port
	mov	bx,dx
	shl	bx,1
	mov	dx,word [equip_parallel+bx] ; DX = serial port address
	or	dx,dx
	jz	int_17_error		; specified port is not installed

	mov	bl,al			; save AL to BL

	or	ah,ah
	jz	int_17_fn00		; AH = 00h
	dec	ah
	jz	int_17_fn01		; AH = 01h
	dec	ah
	jz	int_17_fn02		; AH = 02h

int_17_exit:
	xor	ah,prn_stat_invert	; invert bits 3 and 6 of the status
	mov	al,bl			; restore AL

int_17_error:
	pop	ds
	pop	si
	pop	dx
	pop	bx
	iret

;=========================================================================
; int_17_fn00 - Print character
; Input:
;	AH = 0 - function 00h - print character
;	AL = character to print
;	DX = printer port number (0-2)
; Output:
;	AH = printer status:
;		bit 0	= 1	- timeout error
;		bit 1,2		- reserved
;		bit 3	= 1	- I/O error
;		bit 4	= 1	- printed selected
;		bit 5	= 1	- out of paper
;		bit 6	= 1	- acknowledge from printer
;		bit 7	= 1	- printer not busy
;-------------------------------------------------------------------------
int_17_fn00:
	out	dx,al			; output the character to the data port
	inc	dx			; DX = prn_stat_reg
					; OPTIMIZATION:
					; prn_stat_reg = prn_data_reg + 1
	in	al,dx			; read status
	mov	ah,al			; AH = printer status
	test	ah,prn_stat_busy	; check if printer is busy
        jnz     .not_busy

	mov	ax,90FEh		; printer busy
	int	15h			; call OS hook

	mov	bh,prn_stat_busy
	call	wait_for_port		; wait for printer to be ready
	jnz	.timeout		; timeout had occured

.not_busy:
	inc	dx			; DX = prn_ctrl_reg
					; OPTIMIZATION:
					; prn_ctrl_reg = prn_stat_reg + 1
	mov	al,(prn_ctrl_sel | prn_ctrl_init | prn_ctrl_strobe)
	out	dx,al			; activate strobe
	out	unused_reg,al		; I/O delay
	mov	al,(prn_ctrl_sel | prn_ctrl_init)
	out	dx,al			; de-activate strobe
	and	ah,prn_stat_bits	; leave only valid status bits
	jmp	int_17_exit

.timeout:
	and	ah,prn_stat_bits
	or	ah,prn_stat_tmout	; set timeout bit
	jmp	int_17_exit

;=========================================================================
; int_17_fn02 - Return printer status
; Input:
;	AH = 2 - function 02h - return printer status
;	DX = printer port number (0-2)
; Output:
;	AH = printer status (see int_17_fn00 for complete description)
;-------------------------------------------------------------------------
int_17_fn02:
	inc	dx			; DX = prn_stat_reg
					; OPTIMIZATION:
					; prn_stat_reg = prn_data_reg + 1
int_17_status:
	in	al,dx			; read the status
	mov	ah,al			; move status to AH
	and	ah,prn_stat_bits	; discard reserved bits
	jmp	int_17_exit
