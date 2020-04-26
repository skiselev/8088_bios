;=========================================================================
; at_kbc.inc - AT style keyboard controller support
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
kbc_stat_tout	equ	40h	; receive/transmit timeout
kbc_stat_perr	equ	80h	; parity error
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
kbc_ctr_timeout        equ     30000; XXX 20000
; keyboard and auxiliary device commands
kbd_cmd_leds	equ	0EDh	; keyboard set LEDs
kbd_cmd_rate	equ	0F3h	; keyboard set typematic rate/delay
kbd_cmd_enable	equ	0F4h	; keyboard enable
kbd_cmd_disable	equ	0F5h	; keyboard disable 
kbd_cmd_reset	equ	0FFh	; keyboard reset and self-test
aux_cmd_scale11	equ	0E6h	; auxiliary device set 1:1 scaling factor
aux_cmd_scale21	equ	0E7h	; auxiliary device set 2:1 scaling factor
aux_cmd_resol	equ	0E8h	; auxiliary device set resolution
aux_cmd_status	equ	0E9h	; auxiliary device get status
aux_cmd_dev_id	equ	0F2h	; auxiliary device read device ID
aux_cmd_rate	equ	0F3h	; auxiliary device set sample rate
aux_cmd_enable	equ	0F4h	; auxiliary device enable
aux_cmd_disable	equ	0F5h	; auxiliary device disable
aux_cmd_reset	equ	0FFh	; auxiliary device reset and self-test
; keyboard and auxiliary device responses
kbd_rsp_ack	equ	0FAh	; acknowledge
kbd_rsp_bat_ok	equ	0AAh	; basic assurance test is OK


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
	mov	ah,al			; save command to AH
	mov	cx,3			; try 3 times

.retry:
	push	cx
	cli
; clear the Error, Acknowledge received, and resend received flags
	and	byte [kbd_flags_4],4Fh
	xor	cx,cx

.2:					; wait for KBC to empty input buffer
	in	al,kbc_status_reg
	test	al,kbc_stat_ibf
	loopnz	.2

	mov	al,ah
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
;	CF == 0 - no error
;	CF == 1 - timeout
;-------------------------------------------------------------------------
kbc_send_cmd:
	call	kbc_write_command
	jc	.exit
	xchg	al,ah
	call	kbc_write_data
	xchg	al,ah
.exit:
	ret

;=========================================================================
; kbc_write_command - send command byte to keyboard controller
; Input:
;	AL - command byte
; Output:
;	CF == 0 - no error
;	CF == 1 - timeout
;-------------------------------------------------------------------------
kbc_write_command:
	call	kbc_wait_write
	jc	.timeout		; CF == 1 - timeout
	out	kbc_command_reg,al	; write command

.timeout:
	ret

;=========================================================================
; kbc_write_data - write byte to keyboard controller data register (0x60)
; Input:
;	AL - command byte
; Output:
;	CF == 0 - no error
;	CF == 1 - timeout
;-------------------------------------------------------------------------
kbc_write_data:
	call	kbc_wait_write
	jc	.timeout		; CF == 1 - timeout
	out	kbc_data_reg,al		; write data register

.timeout:
	ret

;=========================================================================
; kbc_wait_write - wait for keyboard controller input buffer to be empty
; Input:
;	none
; Output:
;	CF == 0 - no error, input buffer is empty
;	CF == 1 - timeout
;-------------------------------------------------------------------------
kbc_wait_write:
	push	ax
	push	cx
	mov	cx,kbc_ctr_timeout
					; wait for KBC to empty input buffer
.wait:
	in	al,kbc_status_reg
	test	al,kbc_stat_ibf
	jz	.exit			; note: test instruction sets CF=0
	push	cx
	mov	cx,4
	call	delay_15us		; 45-60us I/O delay
	pop	cx
	loop	.wait
	stc				; CF=1 - timeout
.exit:
	pop	cx
	pop	ax
	ret

;=========================================================================
; NEW: kbc_data_read - wait for data in keyboard controller output buffer
;                  and read it
; Input:
;	none
; Output:
;	AL = data from the keyboard controller
;	AH = keyboard controller status register
;	CF == 0 - no error, data is available
;	CF == 1 - KBC timeout
;-------------------------------------------------------------------------
kbc_data_read:
	push	cx
	mov	cx,kbc_ctr_timeout
					; wait for data
.wait:
	in	al,kbc_status_reg
	mov	ah,al
	test	ah,kbc_stat_obf
	jnz	.read_data		; note: test instruction sets CF=0
	push	cx
	mov	cx,4
	call	delay_15us		; 45-60us I/O delay
	pop	cx
	loop	.wait
	stc				; CF=1 - timeout
	jmp	.exit
.read_data:
	in	al,kbc_data_reg
					; check for receive/transmit timeout
					; and parity errors
	test	ah,kbc_stat_tout | kbc_stat_perr
	jnz	.wait			; note: test instruction sets CF=0
.exit:
	pop	cx
	ret

;=========================================================================
; kbc_wait_read - wait for data in keyboard controller output buffer
; Input:
;	none
; Output:
;	AL = keyboard status register
;	CF == 0 - no error, data is available
;	CF == 1 - timeout
;-------------------------------------------------------------------------
kbc_wait_read:
	push	cx
	mov	cx,kbc_ctr_timeout
					; wait for data
.wait:
	in	al,kbc_status_reg
	test	al,kbc_stat_obf
	jnz	.exit			; note: test instruction sets CF=0
	push	cx
	mov	cx,4
	call	delay_15us		; 45-60us I/O delay
	pop	cx
	loop	.wait
	stc				; CF=1 - timeout
	jmp	.exit
.exit:
	pop	cx
	ret

%ifdef PS2_MOUSE
;=========================================================================
; kbc_aux_read - read data from auxiliary device
; Input:
;	none
; Output:
;	AL = data
;	CF == 0 - no error
;	CF == 1 - timeout
;-------------------------------------------------------------------------
kbc_aux_read:
	call	kbc_wait_aux_read
	jc	.exit			; CF==1 - timeout waiting for data
	push	cx
	mov	cx,1			; FIXME: why this delay is needed?
	call	delay_15us
	pop	cx
	in	al,kbc_data_reg
	clc				; set CF=0 - no error, data is in AL
.exit:
	ret

;=========================================================================
; kbc_aux_send - send command to auxiliary device, wait for acknowledge
; Input:
;	AL = command
; Output:
;	AH - status:
;		00h - success
;		03h - interface error (timeout)
;		04h - resend requested
;	CF == 0 - no error
;	CF == 1 - error
;-------------------------------------------------------------------------
kbc_aux_send:
	push	cx
	mov	ah,al			; store command to AH
	mov	al,kbc_cmd_aux_snd	; write byte to auxiliary device
	call	kbc_send_cmd
	jc	.timeout		; kbc_send_cmd timed out

					; wait for acknowledge
	mov	cx,10			; retry 10 times
.retry:
	call	kbc_wait_aux_read
	jnc	.read_reply		; CF==0 - data is available
	loop	.retry
	jmp	.timeout		; no reply - timeout
.read_reply:
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
	mov	al,00h			; success - ACK received
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
;	CF = 0 - no error
;	CF = 1 - KBC timeout
;-------------------------------------------------------------------------
kbc_aux_enable:
	push	ax
	mov	al,kbc_cmd_wr_ctr	; send "write keyboard controller" cmd
	mov	ah,01001111b
;		   |||||||`-- 1 == enable OBF interrupt for keyboard (IRQ1)
;                  ||||||`-- 1 == enable OBF interrupt for aux port (IRQ12)
;		   |||||`-- 1 == POST complete
;                  ||||`-- 1 == disable the keyboard inhibit
;                  |||`-- 0 == enable keyboard interface
;		   ||`-- 0 == enable mouse interface
;		   |`-- 1 == enable scancode translation
;		   `-- 0 == reserved, must be 0
	call	kbc_send_cmd
	pop	ax
	ret

;=========================================================================
; kbc_aux_disable - disable auxiliary device
; Input:
;	none
; Output:
;	CF = 0 - no error
;	CF = 1 - KBC timeout
;-------------------------------------------------------------------------
kbc_aux_disable:
	push	ax
	mov	al,kbc_cmd_wr_ctr	; send "write keyboard controller" cmd
	mov	ah,01101101b
;		   |||||||`-- 1 == enable OBF interrupt for keyboard (IRQ1)
;                  ||||||`-- 0 == disable OBF interrupt for aux port (IRQ12)
;		   |||||`-- 1 == POST complete
;                  ||||`-- 1 == disable the keyboard inhibit
;                  |||`-- 0 == enable keyboard interface
;		   ||`-- 1 == disable mouse interface
;		   |`-- 1 == enable scancode translation
;		   `-- 0 == reserved, must be 0
	call	kbc_send_cmd
	pop	ax
	ret

;=========================================================================
; kbc_wait_aux_read - wait for data in keyboard controller auxiliary buffer
; Input:
;	none
; Output:
;	AL = keyboard status register
;	CF == 0 - no error, data is available
;	CF == 1 - timeout or no data from an auxiliary device available
;-------------------------------------------------------------------------
kbc_wait_aux_read:
	call	kbc_wait_read
	jc	.exit			; CF==1 - timeout
	test	al,kbc_stat_aobf
	jnz	.exit			; note: test instruction sets CF=0
	stc				; no auxiliary data, set CF=1
.exit:
	ret

%endif ; PS2_MOUSE

;=========================================================================
; kbc_flush - flush all input data from i8042 buffer
; Input:
;	none
; Output:
;	CF = 0 - flushed successfully
;	CF = 1 - can't flush after 20 retries, probably no hardware
;-------------------------------------------------------------------------
kbc_flush:
	push	ax
	push	cx
	mov	cx,20			; maximal KBC buffer size
.flush_next_byte:
	in	al,kbc_status_reg
	test	al,kbc_stat_obf
	jz	.flushed
	push	cx
	mov	cx,4
	call	delay_15us		; 45-60us I/O delay
	pop	cx
	in	al,kbc_data_reg
	loop	.flush_next_byte
	stc				; unable to flush KBC
	jmp	.exit
.flushed:
	clc
.exit:
	pop	cx
	pop	ax
	ret

;=========================================================================
; kbc_init - Initialize keyboard controller
;-------------------------------------------------------------------------
kbc_init:
	push	ax
	push	cx
	push	dx

;-------------------------------------------------------------------------
; flush the data from keyboard controller output buffer

; XXX loop for a number of times, don't hang
.flush:
	call	kbc_data_read
	jnc	.flush			; more data might be available
					; read again

;-------------------------------------------------------------------------
; run keyboard controller self-test

	mov	al,kbc_cmd_test		; send KBC self test command
	call 	kbc_write_command

	call	kbc_data_read		; wait and read the response
	jc	kbc_flush_fail		; XXX: use different error code
	cmp	al,55h			; check for success
	jne	kbc_test_fail

;-------------------------------------------------------------------------
; run keyboard interface test

	mov	al,kbc_cmd_kbd_tst	; send test keyboard interface command
	call	kbc_write_command
	
	call	kbc_data_read		; wait and read test result
	cmp	al,0			; check for success
	jne	kbd_init_fail

;-------------------------------------------------------------------------
; read display type and set equipment bits accordingly

	mov	al,kbc_cmd_rd_in	; send read input port command
	call	kbc_write_command

	call	kbc_data_read		; wait and read input port value
	test	al,kbc_in_display
	jnz	.get_disp_color		; input port bit 6 set => CGA display
	or	word [equipment_list],equip_mono
	jmp	.get_disp_done
.get_disp_color:
	or	word [equipment_list],equip_color_80
.get_disp_done:

;-------------------------------------------------------------------------
; enable keyboard interface and reset the keyboard

	mov	al,kbc_cmd_kbd_ena	; send enable keyboard interface cmd
	call	kbc_write_command
	jc	kbd_init_fail

;XXX: fixme implement kbd_send_cmd_byte, or use DX as the register address?
	mov	al,kbd_cmd_reset
	call	kbc_write_data		; send reset command to the keyboard
	jc	kbd_init_fail

	call	kbc_data_read		; wait for ACK
	jc	kbd_key_fail
	cmp	al,kbd_rsp_ack
	jne	kbd_key_fail		; XXX: print error if no keyboard

	call	kbc_data_read
	jc	kbd_key_fail
	cmp	al,kbd_rsp_bat_ok
	jne	kbd_key_fail		; XXX: print error if BAT fails

;-------------------------------------------------------------------------
; disable keyboard

	mov	al,kbd_cmd_disable
	call	kbc_write_data		; send keyboard disable command
	jc	kbd_key_fail

	call	kbc_data_read		; wait for ACK
	jc	kbd_key_fail
	cmp	al,kbd_rsp_ack
	jne	kbd_key_fail		; XXX: print error if no keyboard

%ifdef PS2_MOUSE
;-------------------------------------------------------------------------
; enable auxiliary port and check for PS/2 mouse presence

	mov	al,kbc_cmd_aux_ena	; send enable auxiliary interface cmd
	call	kbc_write_command

; FIXME - do we need to retry it?
;	mov	cx,10			; try 10 times
;.mouse_reset_retry:
	mov	al,aux_cmd_reset
	call	kbc_aux_send
	jnc	.mouse_reset_ok		; no error - continue
	cmp	ah,03h			; timeout error?
;	loopz	.mouse_reset_retry
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

;-------------------------------------------------------------------------
; set keyboard controller configuration:
; enable keyboard interface, disable auxiliary interface, set POST complete

	mov	al,kbc_cmd_wr_ctr	; send "write keyboard controller" cmd
	mov	ah,01100101b
;		   |||||||`-- 1 == enable OBF interrupt for keyboard (IRQ1)
;		   ||||||`-- 0 == disable OBF interrupt for aux port (IRQ12)
;		   |||||`-- 1 == POST complete
;		   ||||`-- 0 == reserved, must be 0
;		   |||`-- 0 == enable keyboard interface
;		   ||`-- 1 == disable auxiliary interface
;		   |`-- 1 == enable scan code translation to IBM PC scan codes
;		   `-- 0 == reserved, must be 0
	call	kbc_send_cmd
	jc	kbd_init_fail

;-------------------------------------------------------------------------
; enable keyboard

	mov	al,kbd_cmd_enable
	call	kbc_write_data		; send keyboard enable command
	jc	kbd_init_fail

	call	kbc_data_read		; wait for ACK
	jc	kbd_key_fail
	cmp	al,kbd_rsp_ack
	jne	kbd_key_fail		; XXX: print error if no keyboard

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

kbd_init_fail:
	push	ax
	add	al,e_kbd_init_fail
	out	post_reg,al
	pop	ax
	mov	cx,8000h
	call	delay_15us
	out	post_reg,al
	mov	cx,8000h
	call	delay_15us
	jmp	kbd_init_fail

.1:
	hlt
	jmp	.1

kbd_key_fail:
	mov	al,e_kbd_key_fail
	out	post_reg,al
.1:
	hlt
	jmp	.1
