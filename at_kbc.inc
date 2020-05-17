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
kbc_cmd_rd_ctr	equ	20h	; read controller configuration byte command
kbc_cmd_wr_ctr	equ	60h	; write controller configruation byte command
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
; keyboard controller response timeout in 60us units
kbc_ctr_timeout	equ     20000	; 1.2 seconds
; keyboard and auxiliary device commands
aux_cmd_scale11	equ	0E6h	; auxiliary device set 1:1 scaling factor
aux_cmd_scale21	equ	0E7h	; auxiliary device set 2:1 scaling factor
aux_cmd_resol	equ	0E8h	; auxiliary device set resolution
aux_cmd_status	equ	0E9h	; auxiliary device get status
kbd_cmd_leds	equ	0EDh	; keyboard set LEDs
dev_cmd_dev_id	equ	0F2h	; read device ID
dev_cmd_rate	equ	0F3h	; keyboard set typematic rate/delay
				; auxiliary device set sample rate
dev_cmd_enable	equ	0F4h	; device enable
dev_cmd_disable	equ	0F5h	; device disable
dev_cmd_reset	equ	0FFh	; reset and self-test
; keyboard and auxiliary device responses
dev_rsp_bat_ok	equ	0AAh	; basic assurance test is OK
dev_rsp_ack	equ	0FAh	; acknowledge
dev_rsp_error	equ	0FCh	; error (basic assurance test failed)
dev_rsp_resend	equ	0FEh	; resend (invalid command or argument)

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
; kbc_data_read - wait for data in keyboard controller output buffer
;                 and read it
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

%ifdef PS2_MOUSE
;=========================================================================
; kbc_aux_read - read data from auxiliary device
; Input:
;	none
; Output:
;	AL = data
;	AH = keyboard controller status register
;	CF == 0 - no error
;	CF == 1 - timeout
;-------------------------------------------------------------------------
kbc_aux_read:
	call	kbc_data_read
	jc	.exit			; CF==1 - timeout
	test	ah,kbc_stat_aobf	; data from auxiliary device?
	jnz	.exit			; note: test instruction sets CF=0
	stc				; no auxiliary data, set CF=1
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
	call	kbc_aux_read
	jnc	.check_reply		; CF==0 - data is available
	loop	.retry
	jmp	.timeout		; no reply - timeout
.check_reply:
	cmp	al,dev_rsp_ack
	je	.ok
	cmp	al,dev_rsp_resend
	je	.resend
	cmp	al,dev_rsp_error
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
; Flush the data from keyboard controller output buffer

	call	kbc_flush
	jc	kbc_flush_fail

;-------------------------------------------------------------------------
; Disable keyboard and mouse interfaces so that they don't interfere with tests
; No error handling here. The controller and the keyboard are tested later

	mov	ah,01110000b
;		   |||||||`-- 0 == disable OBF interrupt for keyboard (IRQ1)
;		   ||||||`-- 0 == disable OBF interrupt for aux port (IRQ12)
;		   |||||`-- 0 == power on / POST in progress
;		   ||||`-- 0 == reserved, must be 0
;		   |||`-- 1 == disable keyboard interface
;		   ||`-- 1 == disable auxiliary interface
;		   |`-- 1 == enable scan code translation to IBM PC scan codes
;		   `-- 0 == reserved, must be 0

	mov	al,kbc_cmd_wr_ctr	; write controller configuration byte
	call	kbc_send_cmd

;-------------------------------------------------------------------------
; Run keyboard controller self-test

	mov	dh,e_kbc_test_fail
	mov	dl,e_kbd_tout_fail
	mov	al,kbc_cmd_test		; send KBC self test command
	call 	kbc_write_command
	jc	.kbc_test_error

	mov	dl,e_kbd_resp_fail
	call	kbc_data_read		; wait and read the response
	jc	.kbc_test_error
	mov	dl,e_kbc_test_fail
	cmp	al,55h			; check for success
	je	.kbc_test_ok

.kbc_test_error:
	call	kbd_key_fail		; show the error on the POST display

.kbc_test_ok:

;-------------------------------------------------------------------------
; Run keyboard interface test

	mov	dh,e_kbc_int_fail
	mov	dl,e_kbd_tout_fail
	mov	al,kbc_cmd_kbd_tst	; send test keyboard interface command
	call	kbc_write_command
	jc	.keyboard_interface_error

	mov	dl,e_kbd_resp_fail
	call	kbc_data_read		; wait and read test result
	jc	.keyboard_interface_error
	mov	dl,e_kbd_int_fail
	cmp	al,0			; check for success
	je	.keyboard_interface_ok

.keyboard_interface_error:
	call	kbd_key_fail		; show the error on the POST display

.keyboard_interface_ok:

;-------------------------------------------------------------------------
; Read display type and set equipment bits accordingly
; No error handling here. In the worst case the display settings will be wrong

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
; Enable keyboard interface

	mov	al,kbc_cmd_kbd_ena	; send enable keyboard interface cmd
	call	kbc_write_command
	jc	kbc_cmd_fail

;-------------------------------------------------------------------------
; Check that BAT (Basic Assurance Test) is OK:

; Someone at IBM thought that it is a really clever idea that the keyboard
; would send a BAT (Basic Assurance Test) OK code (0AAh) after the power on
; And yet, not all keyboards do, at least not always...
; Also keyboards do not reset themself on the system reset/reboot...
; Also some keyboard controllers (VIA VT82C42 particularly)
; seem to fake BAT OK code on the power on...

; check for BAT code in the buffer, and if it is not there -
; issue a keyboard reset command

	call	kbc_data_read		; check for BAT code in the buffer
	jc	.keyboard_reset		; timeout - reset the keyboard
	cmp	al,dev_rsp_bat_ok
	je	.keyboard_bat_ok	; all good, do not reset the keyboard

.keyboard_reset:
	mov	dh,e_kbd_bat_fail	; major error numbre
	mov	dl,e_kbd_tout_fail	; minor error number
	mov	al,dev_cmd_reset
	call	kbc_write_data		; send reset command to the keyboard
	jc	.keyboard_reset_error

	mov	dl,e_kbd_resp_fail	; minor error number
	call	kbc_data_read		; wait for ACK
	jc	.keyboard_reset_error
	mov	dl,e_kbd_nack_fail	; minor error number
	cmp	al,dev_rsp_ack
	jne	.keyboard_reset_error

	mov	dl,e_kbd_rsp2_fail	; minor error number
	call	kbc_data_read
	jc	.keyboard_reset_error
	mov	dl,e_kbd_nbat_fail	; minor error number
	cmp	al,dev_rsp_bat_ok
	je	.keyboard_bat_ok	; BAT received, continue

.keyboard_reset_error:
	call	kbd_key_fail		; show the error on the POST display

.keyboard_bat_ok:

;-------------------------------------------------------------------------
; Disable keyboard

	mov	dh,e_kbd_dis_fail	; major error number
	mov	dl,e_kbd_tout_fail	; minor error number
	mov	al,dev_cmd_disable
	call	kbc_write_data		; send keyboard disable command
	jc	.keyboard_disable_error

	mov	dl,e_kbd_resp_fail	; minor error number
	call	kbc_data_read		; wait for ACK
	jc	.keyboard_disable_error
	mov	dl,e_kbd_nack_fail	; minor error number
	cmp	al,dev_rsp_ack
	je	.keyboard_disable_ok	; ACK received, continue

.keyboard_disable_error:
	call	kbd_key_fail		; show the error on the POST display

.keyboard_disable_ok:

%ifdef PS2_MOUSE
;-------------------------------------------------------------------------
; Enable auxiliary port and check for PS/2 mouse presence

	mov	al,kbc_cmd_aux_ena	; send enable auxiliary interface cmd
	call	kbc_write_command

	mov	al,dev_cmd_reset
	call	kbc_aux_send
	cmp	ah,00h			; was the command sent successfully?
	jne	.no_mouse

	call	kbc_aux_read
	jc	.no_mouse
	cmp	al,dev_rsp_bat_ok	; Basic Assurance Test successful?
	jne	.no_mouse
	call	kbc_aux_read		; read and throw away Mouse ID
	jc	.no_mouse
; mouse reset was successful, update equipment word accordingly
	or      word [equipment_list],equip_mouse
.no_mouse:
%endif ; PS2_MOUSE

;-------------------------------------------------------------------------
; Set keyboard controller configuration:
; Enable keyboard interface; Disable auxiliary interface; Set POST complete

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
	jc	kbc_cmd_fail

;-------------------------------------------------------------------------
; Enable keyboard

	mov	dh,e_kbd_ena_fail	; major error number
	mov	dl,e_kbd_tout_fail	; minor error number
	mov	al,dev_cmd_enable
	call	kbc_write_data		; send keyboard enable command
	jc	.keyboard_enable_error

	mov	dl,e_kbd_resp_fail	; minor error number
	call	kbc_data_read		; wait for ACK
	jc	.keyboard_enable_error
	mov	dl,e_kbd_nack_fail	; minor error number
	cmp	al,dev_rsp_ack
	je	.keyboard_enable_ok	; ACK received, continue

.keyboard_enable_error:
	call	kbd_key_fail		; show the error on the POST display

.keyboard_enable_ok:
;-------------------------------------------------------------------------

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

kbc_cmd_fail:
	mov	al,e_kbc_cmd_fail
	out	post_reg,al
.1:
	hlt
	jmp	.1

;=========================================================================
; kbd_key_fail - show keyboard initializaiton error on the POST display
; Input:
;	DH = major error number
;	DL = minor error number
;	AL = data or status code (DEBUG mode only)
; Output:
;	none
;-------------------------------------------------------------------------
kbd_key_fail:
	mov	cx,5			; repeat 5 times
	mov	ah,al			; save AL in AH
.1:
	push	cx
	mov	al,dh			; show DH first (major error number)
	out	post_reg,al
	mov	cx,8000h		; wait approximately 0.5 seconds
	call	delay_15us
	mov	al,dl			; show DL next (minor error number)
	out	post_reg,al
	mov	cx,8000h		; wait approximately 0.5 seconds
	call	delay_15us
	mov	al,ah			; show saved value of AL (data/status)
	out	post_reg,al
	mov	cx,8000h		; wait approximately 0.5 seconds
	call	delay_15us
	pop	cx
	loop	.1
	ret
