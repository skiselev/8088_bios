;=========================================================================
; ps2aux.inc - PS/2 mouse support functions:
;	INT 15h, function AH=0C2h
;	INT 74h - IRQ12 interrupt handler
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
int_15_fnC2_bp	equ	0
int_15_fnC2_ds	equ	int_15_fnC2_bp+2
int_15_fnC2_dl	equ	int_15_fnC2_ds+2
int_15_fnC2_dh	equ	int_15_fnC2_dl+1
int_15_fnC2_cl	equ	int_15_fnC2_dh+1
int_15_fnC2_ch	equ	int_15_fnC2_cl+1
int_15_fnC2_bl	equ	int_15_fnC2_ch+1
int_15_fnC2_bh	equ	int_15_fnC2_bl+1
int_15_fnC2_al	equ	int_15_fnC2_bh+1
int_15_fnC2_ah	equ	int_15_fnC2_al+1
int_15_fnC2_ip	equ	int_15_fnC2_ah+1
int_15_fnC2_cs	equ	int_15_fnC2_ip+2
int_15_fnC2_flags equ	int_15_fnC2_cs+2

;=========================================================================
; int_15_fnC2 - mouse functions
; Input:
;	AH = 0C2h - mouse functions
;	AL - function:
;		00h - enable / disable PS/2 mouse
;		01h - reset PS/2 mouse
;		02h - set sample rate
;		03h - set resolution
;		04h - read device type
;		05h - initialize PS/2 mouse
;		06h - set scaling or get status
;		07h - set PS/2 mouse driver address
; Output:
;	AH - exit status:
;		00h - no error
;		01h - invalid function call
;		02h - invalid input value
;		03h - interface error
;		04h - request for resend received from 8042
;		05h - no driver installed (function 0C207h has not been called)
;	CF = 1 - function is not supported, CF = 0 function is supported
;-------------------------------------------------------------------------
int_15_fnC2:
	push	ax
	push	bx
	push	cx
	push	dx
	push	ds
	push	bp
	mov	bp,sp			; establish stack addressing
	mov	bx,biosdseg
	mov	ds,bx
	mov	bx,word [ebda_segment]
	mov	ds,bx			; load EBDA segment to DS
	cmp	al,.num_func
	jae	int_15_fnC2_err1	; return error 1 - invalid function
	call	kbc_aux_disable		; disable auxiliary device
	mov	bl,al			; set to index into dispatch table
	mov	bh,0
	shl	bx,1			; address words
    cs	jmp	near [.dispatch+bx]

.dispatch:
	dw	int_15_fnC200
	dw	int_15_fnC201
	dw	int_15_fnC202
	dw	int_15_fnC203
	dw	int_15_fnC204
	dw	int_15_fnC205
	dw	int_15_fnC206
	dw	int_15_fnC207
.num_func	equ ($-.dispatch)/2

int_15_fnC2_ok:
	mov	ah,00h			; no error
	and	byte [bp+int_15_fnC2_flags],~1 ; clear CF
	jmp	int_15_fnC2_exit

int_15_fnC2_err1:
	mov	ah,01h			; invalid function call
	jmp	int_15_fnC2_err

int_15_fnC2_err2:
	mov	ah,02h			; invalid input value
	jmp	int_15_fnC2_err

int_15_fnC2_err3:
	mov	ah,03h			; interface error
	jmp	int_15_fnC2_err

int_15_fnC2_err5:
	mov	ah,05h			; no driver installed
	jmp	int_15_fnC2_err

int_15_fnC2_err:
	or	byte [bp+int_15_fnC2_flags],1 ; set CF

int_15_fnC2_exit:
	mov	byte [bp+int_15_fnC2_ah],ah ; save return status
	call	kbc_aux_enable		; enable auxiliary device
	pop	bp
	pop	ds
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	iret

;=========================================================================
; int_15_fnC200 - enable / disable PS/2 mouse
; Input:
;	AX = 0C200h
;	BH - sub-function:
;		00h - disable
;		01h - enable
;-------------------------------------------------------------------------
int_15_fnC200:
	mov	bh,byte [bp+int_15_fnC2_bh]
	cmp	bh,01h
	ja	int_15_fnC2_err1	; invalid sub-function

	test	byte [mouse_flags_2],80h ; driver installed?
	jz	int_15_fnC2_err5	; no driver installed

	cmp	bh,00h
	je	.disable

	mov	al,dev_cmd_enable	; device enable command
	jmp	.send_it
.disable:
	mov	al,dev_cmd_disable	; device disable command

.send_it:
	call	kbc_aux_send
	jc	int_15_fnC2_err		; error
	jmp	int_15_fnC2_ok

;=========================================================================
; int_15_fnC201 - reset PS/2 mouse
; Input:
;	AX = 0C201h
; Output:
;	BL - Basic Assurance Test (BAT) completion status
;	     0AAh - BAT successful
;	     0FCh - BAT error
;	BH = device ID
;-------------------------------------------------------------------------
int_15_fnC201:
	mov	cx,10			; try the reset 10 times
.1:
	mov	al,dev_cmd_reset	; reset auxiliary device, set defaults
	call	kbc_aux_send
	jnc	.2			; no error - continue
	cmp	ah,03h			; timeout error?
	loopz	.1			; try again
	jmp	int_15_fnC2_err		; error
.2:
	call	kbc_aux_read
	jc	int_15_fnC2_err3	; interface error
	mov	byte [bp+int_15_fnC2_bl],al
	call	kbc_aux_read
	jc	int_15_fnC2_err3	; interface error
	mov	byte [bp+int_15_fnC2_bh],al
	jmp	int_15_fnC2_ok

;=========================================================================
; int_15_fnC202 - set sample rate
; Input:
;	AX = 0C202h
;	BH - sample rate:
;		00h - 10 samples per second
;		01h - 20 samples per second
;		02h - 40 samples per second
;		03h - 60 samples per second
;		04h - 80 samples per second
;		05h - 100 samples per second
;		06h - 200 samples per second
;-------------------------------------------------------------------------
int_15_fnC202:
	mov	bh,byte [bp+int_15_fnC2_bh]
	cmp	bh,06h
	ja	int_15_fnC2_err2	; invalid input value
	mov	bl,bh
	mov	bh,00h			; rate index in BX

	mov	al,dev_cmd_rate		; set sample rate
	call	kbc_aux_send
	jc	int_15_fnC2_err		; error

    cs	mov	al,byte [.rate_table+bx]
	call	kbc_aux_send
	jc	int_15_fnC2_err		; error
	jmp	int_15_fnC2_ok

.rate_table	db	10, 20, 40, 60, 80, 100, 200
	
;=========================================================================
; int_15_fnC203 - set resolution
; Input:
;	AX = 0C203h
;	BH - resolution value:
;		00h - 1 count per millimeter
;		01h - 2 counts per millimeter
;		02h - 4 counts per millimeter
;		03h - 8 counts per millimeter
;-------------------------------------------------------------------------
int_15_fnC203:
	mov	bh,byte [bp+int_15_fnC2_bh]
	cmp	bh,03h
	ja	int_15_fnC2_err2	; invalid input value

	mov	al,aux_cmd_resol	; set resolution
	call	kbc_aux_send
	jc	int_15_fnC2_err		; error

	mov	al,bh
	call	kbc_aux_send
	jc	int_15_fnC2_err		; error
	jmp	int_15_fnC2_ok

;=========================================================================
; int_15_fnC204 - read device type
; Input:
;	AX = 0C204h
; Output:
;	BH = device type
;-------------------------------------------------------------------------
int_15_fnC204:
	mov	al,dev_cmd_dev_id	; read device type
	call	kbc_aux_send
	jc	int_15_fnC2_err		; error
	call	kbc_aux_read
	jc	int_15_fnC2_err3
	mov	byte [bp+int_15_fnC2_bh],al
	jmp	int_15_fnC2_ok

;=========================================================================
; int_15_fnC205 - initialize PS/2 mouse
; Input:
;	AX = 0C205h
;	BH = data package size in bytes (03h or 04h)
; Output:
;	none
;-------------------------------------------------------------------------
int_15_fnC205:
	mov	bh,byte [bp+int_15_fnC2_bh]
	cmp	bh,03h
	jb	int_15_fnC2_err2	; invalid input value
	cmp	bh,04h
	ja	int_15_fnC2_err2	; invalid input value
	dec	bh

	mov	al,byte [mouse_flags_2]
	and	al,0F8h			; mask out package size bits
	or	al,bh			; add the new package size
	mov	byte [mouse_flags_2],al

	mov	cx,5			; try the reset 5 times
.1:
	mov	al,dev_cmd_reset	; reset auxiliary device, set defaults
	call	kbc_aux_send
	jnc	.2			; no error - continue
	cmp	ah,03h			; timeout error?
	loopz	.1			; try again
	jmp	int_15_fnC2_err		; error
.2:
	call	kbc_aux_read
	jc	int_15_fnC2_err3	; interface error
	call	kbc_aux_read
	jc	int_15_fnC2_err3	; interface error
	jmp	int_15_fnC2_ok

;=========================================================================
; int_15_fnC206 - set scaling or get status
; Input:
;	AX = 0C206h
;	BH - sub-function:
;		00h - return status
;		01h - set scaling factor to 1:1
;		02h - set scaling factor to 2:1
; Output:
;	if BH = 00h on entry:
;		BL - status byte 1:
;			bit 0 = 1 - right button pressed
;			bit 1 = 0 - reserved
;			bit 2 = 1 - left button pressed
;			bit 3 = 0 - reserved
;			bit 4 = 0 - 1:1 scaling, 1 - 2:1 scaling
;			bit 5 = 0 - disable, 1 - enable
;			bit 6 = 0 - stream mode, 1 - remote mode
;			bit 7 = 0 - reserved
;		CL - status byte 2:
;			00h - 1 count per millimeter
;			01h - 2 counts per millimeter
;			02h - 4 counts per millimeter
;			03h - 8 counts per millimeter
;		DL - status byte 3:
;			0Ah - 10 samples per second
;			14h - 20 samples per second
;			3Ch - 40 samples per second
;			3Ch - 60 samples per second
;			50h - 80 samples per second
;			64h - 100 samples per second
;			0C8h - 200 samples per second
;-------------------------------------------------------------------------
int_15_fnC206:
	mov	bh,byte [bp+int_15_fnC2_bh]
	cmp	bh,02h
	ja	int_15_fnC2_err1	; invalid sub-function
	je	.scale_2x1
	cmp	bh,01h
	je	.scale_1x1
					; BH == 0, return status
	mov	al,aux_cmd_status	; request status command
	call	kbc_aux_send
	jc	int_15_fnC2_err		; error
	call	kbc_aux_read
	jc	int_15_fnC2_err3	; interface error
	mov	byte [bp+int_15_fnC2_bl],al
	call	kbc_aux_read
	jc	int_15_fnC2_err3	; interface error
	mov	byte [bp+int_15_fnC2_cl],al
	call	kbc_aux_read
	jc	int_15_fnC2_err3	; interface error
	mov	byte [bp+int_15_fnC2_dl],al
	jmp	int_15_fnC2_ok

.scale_2x1:
	mov	al,aux_cmd_scale21	; set 2:1 scaling factor command
	jmp	.send_it
.scale_1x1:
	mov	ah,aux_cmd_scale11	; set 1:1 scaling factor command
.send_it:
	call	kbc_aux_send
	jc	int_15_fnC2_err		; error
	jmp	int_15_fnC2_ok

;=========================================================================
; int_15_fnC207 - set PS/2 mouse driver
; Input:
;	AX = 0C207h
;	ES:BX - pointer to mouse driver
;-------------------------------------------------------------------------
int_15_fnC207:
	mov	bx,word [bp+int_15_fnC2_bl]
	mov	word [mouse_driver],bx
	mov	ax,es
	mov	word [mouse_driver+2],ax
	or	ax,ax
	jnz	.set_handler
	or	bx,bx
	jnz	.set_handler
; remove handler
	and	byte [mouse_flags_2],~80h
	jmp	int_15_fnC2_ok

.set_handler:
	or	byte [mouse_flags_2],80h
	jmp	int_15_fnC2_ok

;=========================================================================
; int_74 - PS/2 mouse hardware interrupt service routine
;-------------------------------------------------------------------------
int_74:
	sti
	push	ax
	push	bx
	push	ds
	mov	ax,biosdseg
	mov	ds,ax
	mov	ax,word [ebda_segment]
	mov	ds,ax

	in	al,kbc_status_reg
	and	al,kbc_stat_obf | kbc_stat_aobf
	cmp	al,kbc_stat_obf | kbc_stat_aobf
	jne	.exit			; no mouse data in the buffer

	in	al,kbc_data_reg
	mov	ah,al			; save to AH

	mov	al,byte [mouse_flags_2]
	test	al,80h
	jz	.exit			; no mouse driver installed

	mov	al,byte [mouse_flags_1]
	and	al,07h			; bits 2-0 are index in buffer
	mov	bx,mouse_data
	add	bl,al			; note: we shouldn't have an overflow...
	mov	byte [bx],ah		; save data to the buffer

	mov	ah,byte [mouse_flags_2]
	and	ah,07h			; bits 2-0 are package count-1

	cmp	al,ah			; enough bytes in the buffer?
	jae	.call_driver

	inc	byte [mouse_flags_1]	; increment the index
	jmp	.exit

.call_driver:

;-------------------------------------------------------------------------
; This BIOS supports 3 bytes (standard PS/2) and 4 bytes (MS Intellimouse)
; package sizes.
; Stack layout (bX refers to byte position X in mouse_data buffer):
; 	- for 3 bytes package: 00 00 b2 00 b1 00 b0 00
; 	- for 4 bytes package: 00 00 b3 00 b2 00 b0 b1
;-------------------------------------------------------------------------

	cmp	ah,03h			; check 4 bytes package size?
	jz	.four_bytes
					; put data in stack for 3 bytes format
	xor	ax,ax
	mov	al,byte [mouse_data]
	push	ax
	mov	al,byte [mouse_data+1]
	push	ax
	mov	al,byte [mouse_data+2]
	push	ax
	mov	al,00h
	push	ax
	jmp	.do_call

.four_bytes:				; put data in stack for 4 bytes format
	mov	ah,byte [mouse_data+1]
	mov	al,byte [mouse_data]
	push	ax
	mov	ah,00h
	mov	al,byte [mouse_data+2]
	push	ax
	mov	al,byte [mouse_data+3]
	push	ax
	mov	al,00h
	push	ax

.do_call:
	call	far [mouse_driver]
	add	sp,0008h		; remove parameters from the stack
	and	byte [mouse_flags_1],0F8h ; reset mouse data index

.exit:
	mov	al,20h
	out	pic2_reg0,al		; signal EOI to the slave PIC
	out	pic1_reg0,al		; signal EOI to the master PIC
	pop	ds
	pop	bx
	pop	ax
	iret
