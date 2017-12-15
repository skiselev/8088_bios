;=========================================================================
; at_kbc.inc - AT style keyboard controller support
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

;=========================================================================
; Keyboard controller defines - ports, commands, and flags
;-------------------------------------------------------------------------
kbc_data_reg	equ	60h
; keyboard controller return codes
kbc_ret_test	equ	0AAh	; keyboard controller test passed
; keyboard controller status register and its bits
kbc_status_reg	equ	64h
kbc_stat_obf	equ	01h	; output buffer full flag
kbc_stat_ibf	equ	02h	; input buffer full flag
kbc_stat_aobf	equ	20h	; auxiliary output buffer full flag
; keyboard contoller command register and commands
kbc_command_reg	equ	64h
kbc_cmd_wr_ctr	equ	60h	; write control register command
kbc_cmd_aux_dis	equ	0A7h	; disable auxiliary interface command
kbc_cmd_aux_ena	equ	0A8h	; enable auxiliary interface command
kbc_cmd_aux_tst	equ	0A9h	; test auxiliary interface command
kbc_cmd_test	equ	0AAh	; keyboard controller self-test command
kbc_cmd_kbd_tst	equ	0ABh	; test keyboard interface command
kbc_cmd_kbd_dis	equ	0ADh	; disable keyboard interface command
kbc_cmd_kbd_ena	equ	0AEh	; enable keyboard interface command
kbc_cmd_rd_in	equ	0C0h	; read keyboard input port
kbc_cmd_aux_snd	equ	0D4h	; send command byte to auxiliary device command
; keyboard controller control register bits
kbc_ctr_kbd_int	equ	01h	; enable keyboard OBF interrupt
kbc_ctr_aux_int	equ	02h	; enable auxiliary OBF interrupt
kbc_ctr_no_lock	equ	08h	; ignore keyboard inhibit (keyboard lock)
kbc_ctr_kbd_dis	equ	10h	; disable keyboard interface
kbc_ctr_aux_dis equ	20h	; disable auxiliary interface
kbc_ctr_xlat	equ	40h	; enable keyboard scancode translation
; keyboard controller input port bits
kbc_in_display	equ	40h	; input port bit 6: 0 = MDA, 1 = CGA
; timeout - number of retries
kbc_ctr_timeout	equ	10000

;=========================================================================
; kbc_kb_send - send command to keyboard, wait for acknowledge
; Input:
;	AL = command
; Output:
;	none
;-------------------------------------------------------------------------
kbc_kb_send:
	push	ax
	push	cx
	mov	cx,3			; try 3 times

.retry:
	push	cx
	cli
; clear the Error, Acknowledge received, and resend received flags
	and	byte [kbd_flags_4],4Fh
	call	kbc_wait_write		; wait for KBC to empty input buffer
	out	kbc_data_reg,al		; send command to the keyboard
	sti
	xor	cx,cx
.wait_for_ack:				; wait for acknowledge (set by IRQ1 ISR)
	test	byte [kbd_flags_4], 10h	; acknowledge bit set?
	loopz	.wait_for_ack
	pop	cx
	jnz	.exit
	loop	.retry			; try again
; if the operation failed after 3 retries, set the error bit and quit
	or	byte [kbd_flags_4], 80h
.exit:
	pop	cx
	pop	ax
	ret

;=========================================================================
; kbc_send_cmd - send command + argument to keyboard controller
; Input:
;	AL - command byte
;	AH = argument
; Output:
;	ZF == 1 - success
;	ZF == 0 - time out
;-------------------------------------------------------------------------
kbc_send_cmd:
	cli
	call	kbc_send_cmd_byte	; send the command byte
	jnz	.exit			; ZF == 0 - time out
	call	kbc_wait_write
	jnz	.exit			; ZF == 0 - time out
	xchg	al,ah
	out	kbc_data_reg,al		; send the argument
	xchg	al,ah			; restore AX

.exit:
	ret

;=========================================================================
; kbc_send_cmd_byte - send command byte to keyboard controller
; Input:
;	AL - command byte
; Output:
;	ZF == 1 - success
;	ZF == 0 - time out
;-------------------------------------------------------------------------
kbc_send_cmd_byte:
	call	kbc_wait_write
	jnz	.exit			; ZF == 0 - time out
	out	kbc_command_reg,al	; write command

.exit:
	ret

;=========================================================================
; kbc_wait_write - wait for keyboard controller input buffer to be empty
; Input:
;	none
; Output:
;	ZF == 1 - success
;	ZF == 0 - time out
;-------------------------------------------------------------------------
kbc_wait_write:
	push	ax
	push	cx
	mov	cx,kbc_ctr_timeout
					; wait for KBC to empty input buffer
.wait:
	push	cx
	mov	cx,4
	call	delay_15us		; 45-60us I/O delay
	pop	cx
	in	al,kbc_status_reg
	test	al,kbc_stat_ibf
	loopnz	.wait
	pop	cx
	pop	ax
	ret

;=========================================================================
; kbc_wait_read - wait for data in keyboard controller output buffer
; Input:
;	none
; Output:
;	AL = keyboard status register
;	ZF == 0 - data is available
;	ZF == 1 - time out
;-------------------------------------------------------------------------
kbc_wait_read:
	push	cx
	mov	cx,kbc_ctr_timeout
					; wait for data
.wait:
	push	cx
	mov	cx,4
	call	delay_15us		; 45-60us I/O delay
	pop	cx
	in	al,kbc_status_reg
	test	al,kbc_stat_obf
	loopz	.wait
	pop	cx
	ret

%ifdef PS2_MOUSE
;=========================================================================
; kbc_aux_read - read data from auxiliary device
; Input:
;	none
; Output:
;	AL = data
;	CF == 0 - data is available
;	CF == 1 - time out
;-------------------------------------------------------------------------
kbc_aux_read:
	call	kbc_wait_aux_read
	jz	.timeout		; if ZF=1 - timeout waiting for data
	push	cx
	mov	cx,1			; FIXME: is this delay needed?
	call	delay_15us
	pop	cx
	in	al,kbc_data_reg
	clc
	jmp	.exit
.timeout:
	stc
.exit:
	ret

;=========================================================================
; kbc_aux_send - send command to auxiliary device, wait for acknowledge
; Input:
;	AL = command
; Output:
;	AH - status:
;		00h - success
;		03h - interface error (time out)
;		04h - resend requested
;	CF == 0 - no error
;	CF == 1 - error
;-------------------------------------------------------------------------
kbc_aux_send:
	push	cx
	mov	ah,al			; store command to AH
	mov	al,kbc_cmd_aux_snd	; write byte to auxiliary device
	call	kbc_send_cmd
	jnz	.timeout		; kbc_send_cmd timed out

					; wait for acknowledge
	mov	cx,10			; retry 10 times
.retry:
	call	kbc_wait_aux_read
	jz	.timeout		; if ZF=1 - timeout waiting for data
	in	al,kbc_data_reg
	cmp	al,0FAh			; ACK?
	je	.ok
	cmp	al,0FEh			; resend?
	je	.resend
	cmp	al,0FCh			; error?
	je	.timeout		; treat as timeout/interface error
	loop	.retry
	jmp	.timeout

.ok:
	xor	al,al			; success - ACK received
	clc
	jmp	.exit

.timeout:
	mov	al,03h			; interface error
	jmp	.error

.resend:
	mov	al,04h			; resend

.error:
	stc

.exit:
	xchg	ah,al			; status to AH, original command to AL
	pop	cx
	ret

;=========================================================================
; kbc_aux_enable - enable auxiliary device
; Input:
;	none
; Output:
;	ZF = 1 - success
;	ZF = 0 - error
;-------------------------------------------------------------------------
kbc_aux_enable:
	push	ax
	mov	al,kbc_cmd_wr_ctr	; write controller command byte
	mov	ah,01000111b		; pc compatible, enable aux
					; enable keyboard, enable aux obf
					; interrupt, enable obf interrupt
	call	kbc_send_cmd
	pop	ax
	ret

;=========================================================================
; kbc_aux_disable - disable auxiliary device
; Input:
;	none
; Output:
;	ZF = 1 - success
;	ZF = 0 - error
;-------------------------------------------------------------------------
kbc_aux_disable:
	push	ax
	mov	al,kbc_cmd_wr_ctr	; write controller command byte
	mov	ah,01100101b		; pc compatible, disable aux
					; enable keyboard, disable aux obf
					; interrupt, enable obf interrupt
	call	kbc_send_cmd
	pop	ax
	ret

;=========================================================================
; kbc_wait_aux_read - wait for data in keyboard controller auxiliary buffer
; Input:
;	none
; Output:
;	AL = keyboard status register
;	ZF == 0 - data is available
;	ZF == 1 - time out
;-------------------------------------------------------------------------
kbc_wait_aux_read:
	call	kbc_wait_read
	jz	.error
	test	al,kbc_stat_aobf
.error:
	ret

%endif ; PS2_MOUSE

;=========================================================================
; kbc_init - Initialize keyboard controller
;-------------------------------------------------------------------------
kbc_init:
	push	ax
	push	cx
	push	dx

;-------------------------------------------------------------------------
; flush the data from keyboard controller output buffer

	mov	cx,kbc_ctr_timeout
.kbc_flush_retry:			; flush the data from KBC output buffer
	push	cx
	mov	cx,4
	call	delay_15us		; 45-60us I/O delay
	pop	cx
	in	al,kbc_status_reg
	mov	ah,al
	test	ah,kbc_stat_obf		; KBC output buffer full?
	jz	.kbc_flush_obf
	in	al,kbc_data_reg		; flush a byte from KBC output buffer
.kbc_flush_obf:
	test	ah,kbc_stat_ibf		; KBC input buffer full?
	loopnz	.kbc_flush_retry	; retry if input buffer is still full
	jnz	kbc_flush_fail
	
;-------------------------------------------------------------------------
; run keyboard controller self-test

	mov	al,kbc_cmd_test		; send KBC self test command
	call 	kbc_send_cmd_byte
; FIXME: need to wait for IBF empty? (meaning KBC had accepted the command)
	call	kbc_wait_read		; wait for response
	in	al,kbc_data_reg
	cmp	al,55h			; check for success
	jne	kbc_test_fail

;-------------------------------------------------------------------------
; run keyboard interface test

.kbc_test_retry:
	mov	al,kbc_cmd_kbd_tst	; send test keyboard interface command
	call	kbc_send_cmd_byte
	call	kbc_wait_read		; wait for test result
	in	al,kbc_data_reg
	cmp	al,0			; check for success
	jne	kbd_int_fail

;-------------------------------------------------------------------------
; read display type and set equipment bits accordingly

	mov	al,kbc_cmd_rd_in	; send read input port command
	call	kbc_send_cmd_byte
	call	kbc_wait_read

	in	al,kbc_data_reg
	test	al,kbc_in_display
	jnz	.get_disp_color		; input port bit 6 set => CGA display
	or	word [equipment_list],equip_mono
	jmp	.get_disp_done
.get_disp_color:
	or	word [equipment_list],equip_color
.get_disp_done:

;-------------------------------------------------------------------------
; initialize keyboard controller

	mov	al,kbc_cmd_kbd_ena	; send enable keyboard interface cmd
	call	kbc_send_cmd_byte

	mov	al,kbc_cmd_aux_ena	; send enable auxiliary interface cmd
	call	kbc_send_cmd_byte

	mov	al,kbc_cmd_wr_ctr	; send "write keyboard controller" cmd
	mov	ah,01101001B		; Bit 7 = 0 - reserved
					; Bit 6 = 1 - IBM PC scancodes
					; Bit 5 = 1 - IBM PC / no party check
					; Bit 4 = 0 - Enable keyboard
					; Bit 3 = 1 - Disable inhibit
					; Bit 2 = 0 - system flag = 0
					; Bit 1 = 0 - reserved
					; Bit 0 = 1 - enable OBF interrupt
	call	kbc_send_cmd

	mov	al,0FFh			; send keyboard reset command
	call	kbc_send_cmd_byte

	mov	cx,1000h
	call	delay_15us

	call	kbc_wait_read		; wait for response
	in	al,kbc_data_reg		; clear the output buffer


%ifdef PS2_MOUSE
;-------------------------------------------------------------------------
; check for PS/2 mouse presence

	mov	cx,10			; try 10 times
.mouse_reset_retry:
	mov	al,0FFh
	call	kbc_aux_send
	jnc	.mouse_reset_ok		; no error - continue
	cmp	ah,03h			; timeout error?
	loopz	.mouse_reset_retry
	jmp	.no_mouse
.mouse_reset_ok:
	call	kbc_aux_read
	jc	.no_mouse
	cmp	al,0AAh			; Basic Assurance Test successful?
	jne	.no_mouse
	call	kbc_aux_read
	jc	.no_mouse
; mouse reset successful, update equipment word accordingly
	or      word [equipment_list],equip_mouse
.no_mouse:
%endif ; PS2_MOUSE

	mov	al,e_kbd_ok
	out	post_reg,al
	pop	dx
	pop	cx
	pop	ax
	ret

kbc_flush_fail:
	mov	al,e_kbc_flsh_fail
	out	post_reg,al
.1:
	hlt
	jmp	.1

kbc_test_fail:
	mov	al,e_kbc_test_fail
	out	post_reg,al
.1:
	hlt
	jmp	.1

kbd_int_fail:
	and	al,0Fh
	add	al,e_kbd_int_fail
	out	post_reg,al
.1:
	hlt
	jmp	.1

kbd_key_fail:
	mov	al,e_kbd_key_fail
	out	post_reg,al
.1:
	hlt
	jmp	.1
