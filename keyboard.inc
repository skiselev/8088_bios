;=========================================================================
; keyboard.inc - Keyboard support
;       INT 16h - BIOS Keyboard Services
;		- function AH=02h
;		- function AH=03h
;		- function AH=05h
;		- function AH=12h
;		- see scancode.inc for other (scancode related)
;                 INT 16h functions
;       INT 09h - IRQ1 interrupt handler
;-------------------------------------------------------------------------
;
; Compiles with NASM 2.11.08, might work with other versions
;
; Copyright (C) 2011 - 2019 Sergey Kiselev.
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
; Scan codes and keyboard flags
;-------------------------------------------------------------------------

; scancodes for special keys

kbd_num_code	equ	45h
kbd_scroll_code	equ	46h
kbd_caps_code	equ	3ah
kbd_ctrl_code	equ	1dh
kbd_alt_code	equ	38h
kbd_rshift_code	equ	36h
kbd_prtsc_code	equ	37h
kbd_lshift_code	equ	2ah
kbd_ins_code	equ	52h
kbd_del_code	equ	53h

; Bits for the various modifier keys

kbd_rshft_bit	equ	1
kbd_lshft_bit	equ	2
kbd_ctrl_bit	equ	4
kbd_alt_bit	equ	8
kbd_scroll_bit	equ	10h
kbd_num_bit	equ	20h
kbd_caps_bit	equ	40h
kbd_ins_bit	equ	80h

;=========================================================================
; int_16 - BIOS keyboard functions
; XXX: critical regions - it appears that INT 16h modifies buffer head,
;	while INT 09h modifies buffer tail, so we shouldn't have any race
;	conditions
;-------------------------------------------------------------------------
	setloc	0E82Eh
int_16:
;	call	int_trace		; DEBUG
	push	bx
	push	ds
	mov	bx,biosdseg
	mov	ds,bx
	cmp	ah,.num_func
	jae	int_16_exit
	mov	bh,0
	mov	bl,ah
	shl	bx,1
    cs	jmp	near [.dispatch+bx]

.dispatch:
	dw	int_16_fn00		; read char from buffer, wait if empty
	dw	int_16_fn01		; check buffer, do not clear
	dw	int_16_fn02		; get shift status
%ifdef AT_KEYBOARD
	dw	int_16_fn03		; set delays
%else ; AT_KEYBOARD
	dw	int_16_exit		; not implemented on XT
%endif ; AT_KEYBOARD
	dw	int_16_exit		; keyclick
	dw	int_16_fn05		; write to keyboard buffer
	dw	int_16_exit		; 06
	dw	int_16_exit		; 07
	dw	int_16_exit		; 08
	dw	int_16_exit		; 09
	dw	int_16_exit		; 0A
	dw	int_16_exit		; 0B
	dw	int_16_exit		; 0C
	dw	int_16_exit		; 0D
	dw	int_16_exit		; 0E
	dw	int_16_exit		; 0F
	dw	int_16_fn10		; get enhanced keystroke
	dw	int_16_fn11		; check enhanced keystroke
	dw	int_16_fn12		; get enhanced shift flags
.num_func	equ	($-.dispatch)/2

int_16_exit:
	pop	ds
	pop	bx
	iret

int_16_exitf:
	pop	ds
	pop	bx
	sti
	retf	2

;=========================================================================
; int_16_fn02 - get shift flags
; Input:
;	AH = 02h
; Output:
;	AL - shift flags
;            AL bits:
;		7 - Insert active
;		6 - Caps Lock active
;		5 - Num Lock active
;		4 - Scroll Lock active
;		3 - Alt key pressed (either Alt on 101/102-key keyboards)
;		2 - Ctrl key pressed (either Ctrl on 101/102-key keyboards)
;		1 - left shift key pressed
;		0 - right shift key pressed
;-------------------------------------------------------------------------
int_16_fn02:
	mov	al,byte [kbd_flags_1]
	jmp	int_16_exit

%ifdef AT_KEYBOARD

;=========================================================================
; int_16_fn03 - set keyboard typematic rate
; Input:
;	AH = 03
;	AL - subfunction
;	     00 - set typematic rate to default
;	     01 - increase initial delay
;	     02 - slow typematic rate by 1/2
;	     04 - turn off typematic chars
;	     05 - set typematic rate/delay
;	BH - repeat delay (AL=5)
;		0 - 250ms	2 - 750ms
;		1 - 500ms	3 - 1000ms
;	BL - typematic rate, one of the following  (AL=5)
;		00 - 30.0	01 - 26.7	02 - 24.0	03 - 21.8
;		04 - 20.0	05 - 18.5	06 - 17.1	07 - 16.0
;		08 - 15.0	09 - 13.3	0A - 12.0	0B - 10.9
;		0C - 10.0	0D - 9.2	0E - 8.6	0F - 8.0
;		10 - 7.5	11 - 6.7	12 - 6.0	13 - 5.5
;		14 - 5.0	15 - 4.6	16 - 4.3	17 - 4.0
;		18 - 3.7	19 - 3.3	1A - 3.0	1B - 2.7
;		1C - 2.5	1D - 2.3	1E - 2.1	1F - 2.0
; Output:
;	none
;-------------------------------------------------------------------------
int_16_fn03:
	cmp	al,05
	jne	.exit			; only AL = 5 implemented
	push	bx
	push	cx
	mov	al,kbc_cmd_kbd_dis	; disable keyboard interface
	call	kbc_send_cmd_byte
	and	bh,3			; legal BH values from 0 to 3
	mov	cl,5
	shl	bh,cl
	and	bl,1Fh			; legal BL values from 0 to 1F
	mov	al,0F3h
	call	kbc_kb_send
	mov	al,bh
	call	kbc_kb_send
	mov	al,kbc_cmd_kbd_ena	; enable keyboard interface
	call	kbc_send_cmd_byte
	mov	al,04h
	call	kbc_kb_send
	pop	cx
	pop	bx
.exit:
	jmp	int_16_exit

%endif ; AT_KEYBOARD

;=========================================================================
; int_16_fn05 - store keystroke in keyboard buffer
; Input:
;	AH = 05h
;	CH = BIOS scan code
;	CL = ACII character
; Output:
;	AL - status
;	     00h - success
;	     01h - keyboard buffer full
;-------------------------------------------------------------------------
int_16_fn05:
	push	si
	mov	al,1			; assume no space
	cli				; critical section
	mov	bx,word [kbd_buffer_tail]
	mov	si,bx
	add	bx,2
	cmp	bx,word [kbd_buffer_end]
	jne	.1
	mov	bx,word [kbd_buffer_start]
.1:
	cmp	bx,word [kbd_buffer_head]
	je	.no_space		; no space in buffer
	mov	word [si],cx
	mov	word [kbd_buffer_tail],bx
	mov	al,0			; stored successfully
.no_space:
	sti
	pop	si
	jmp	int_16_exit

;=========================================================================
; int_16_fn12 - get extended shift flags
; Input:
;	AH = 12h
; Output:
;	AL - shift flags 1 (same as returned by function AH=02)
;	AH - shift flags 2
;            AH bits:
;		7 - SysRq key pressed
;		6 - Caps Lock pressed
;		5 - Num Lock pressed
;		4 - Scroll Lock pressed
;		3 - right Alt key pressed
;		2 - right Ctrl key pressed
;		1 - left Alt key pressed
;		0 - left Ctrl key pressed
;-------------------------------------------------------------------------
int_16_fn12:
	mov	ah,byte [kbd_flags_2]
	and	ah,7Fh			; clear final SysRq bit
	test	ah,00000100b		; test current SysRq bit
	je	.1
	or	ah,80h			; set final SysRq bit
.1:
	and	ah,11110011b		; clear right Alt/Ctrl bits
	mov	al,byte [kbd_flags_3]
	and	al,00001100b		; get right Alt/Ctrl bits
	or	ah,al			; copy to AH
	mov	al,byte [kbd_flags_1]
	jmp	int_16_exit

;=========================================================================
; int_09 - Keyboard hardware interrupt (IRQ1) handler
;-------------------------------------------------------------------------
	setloc	0E987h			; INT 09 Entry Point
int_09:
	push	ax
	push	bx
	push	ds

	mov	ax,biosdseg
	mov	ds,ax

%ifdef AT_KEYBOARD
%ifndef DISABLE_KBD_DURING_INTERRUPTS
	mov	al,kbc_cmd_kbd_dis	; disable keyboard interface
	call    kbc_send_cmd_byte
%endif
%endif ; AT_KEYBOARD
	in	al,ppi_pa_reg		; get keyboard data / scancode
	mov	ah,al			; save it in AH
	in	al,ppi_pb_reg		; read PPI port B, control
	or	al,10000000b		; set keyboard clear bit
	out	ppi_pb_reg,al		; write it back to PPI port B
	and	al,01111111b		; unset keyboard clear bit
	out	ppi_pb_reg,al		; write it back to PPI port B
	mov	al,ah			; restore AL / scancode
	sti
%ifdef AT_KEYBOARD
	mov	ah,4Fh
	stc
	int	15h			; keyboard intercept
	jnc	keyboard_int_exit	; CF = 0, ignore scan code
%else ; AT_KEYBOARD
	cmp	al,0FFh			; check for overrun
	jz	keyboard_int_exit
%endif ; AT_KEYBOARD
; check for KBC response codes

	cmp	al,0EEh			; echo response?
	je      keyboard_int_exit
	cmp	al,0FAh			; acknowledge?
	jne     .2
	or	byte [kbd_flags_4],10h	; set ack bit
	jmp	keyboard_int_exit
.2:
	cmp	al,0FEh			; resend command?
	jne	.3
	or	byte [kbd_flags_4],20h	; set resend bit
	jmp	keyboard_int_exit

; Note: other KBC response codes have their MSB bit set and will be ignored

.3:

; check for 0E0h and 0E1h scancodes, set flags in kbd_flags_3

	cmp	al,0E0h
	jne	.4
	or	byte [kbd_flags_3],10b	; set E0 flag
	and	byte [kbd_flags_3],0FEh	; clear E1 flag
	jmp	keyboard_int_exit
.4:
	cmp	al,0E1h
	jne	.5
	or	byte [kbd_flags_3],1	; set E1 flag
	and	byte [kbd_flags_3],0FDh	; clear E0 Flag
	jmp	keyboard_int_exit

; check for various <Ctrl>+<Alt>+<Key> combinations:

.5:
	mov	bl,byte [kbd_flags_1]
	and	bl,kbd_alt_bit|kbd_ctrl_bit
	cmp	bl,kbd_alt_bit|kbd_ctrl_bit
	jne	.6
	cmp	al,kbd_del_code		; <Ctrl>+<Alt>+<Del>?
	jne	.check_minus
	mov	word [warm_boot],1234h ; warm boot flag
	jmp	0F000h:warm_start	; reboot

; handle turbo mode switch combinations

%ifdef MACHINE_XI8088
.check_minus:
	cmp	al,4Ah			; gray - scancode
	jne	.check_plus

.turbo_off:
	in	al,ppi_pb_reg
	and	al,0FBh			; clear bit 2 (turbo enable bit)
	out	ppi_pb_reg,al
	jmp	keyboard_int_exit

.check_plus:
	cmp	al,4Eh			; gray + scancode
	jne	.6

.turbo_on:
	in	al,ppi_pb_reg
	or	al,04h			; set bit 2 (turbo enable bit)
	out	ppi_pb_reg,al
	jmp	keyboard_int_exit
%endif ; MACHINE_XI8088

%ifdef MACHINE_FE2010A
.check_minus:
	cmp	al,4Ah			; gray - scancode
	jne	.check_asterisk

.turbo_off:
	call	set_cpu_clk_4_77mhz
	jmp	keyboard_int_exit

.check_asterisk:
	cmp	al,37h			; * scancode
	jne	.check_plus

.turbo_7_16mhz:
	call	set_cpu_clk_7_16mhz
	jmp	keyboard_int_exit

.check_plus:
	cmp	al,4Eh			; gray + scancode
	jne	.6

.turbo_9_55mhz:
	call	set_cpu_clk_9_55mhz
	jmp	keyboard_int_exit

%endif ; MACHINE_FE2010A

%ifdef MACHINE_XT
.check_minus:
					; do nothing for an XT
%endif ; MACHINE_XT

; Check for the INS key here. This one needs to toggle the ins bit
; in the keyboard flags variables.

.6:
	cmp	al,kbd_ins_code
	jne	.7
	or	byte [kbd_flags_2],kbd_ins_bit ; set INS bit
	jmp	.translate	   	; pass on INS key

.7:
	cmp	al,kbd_ins_code+80h	; INS "break" scan code
	jne     .8
	and	byte [kbd_flags_2],~kbd_ins_bit ; clear INS bit
	xor     byte [kbd_flags_1],kbd_ins_bit ; toggle INS bit
	jmp     .no_buffer

; handle the left and right Shift keys

.8:
	cmp	al,kbd_lshift_code
	jne	.9
	or	byte [kbd_flags_1],kbd_lshft_bit ; set left Shift bit
	jmp	.no_buffer
.9:
	cmp	al,kbd_lshift_code+80h
	jne     .10
	and     byte [kbd_flags_1],~kbd_lshft_bit ; clear left Shift bit
	jmp     .no_buffer
.10:
	cmp	al,kbd_rshift_code
	jne	.11
	or	byte [kbd_flags_1],kbd_rshft_bit ; set right Shift bit
	jmp	.no_buffer
.11:
	cmp	al,kbd_rshift_code+80h
	jne     .12
	and	byte [kbd_flags_1],~kbd_rshft_bit ; clear right Shift bit
	jmp	.no_buffer

; handle the Alt key

.12:
	cmp	al,kbd_alt_code
	jne	.13
	or	byte [kbd_flags_1],kbd_alt_bit ; set Alt bit
	jmp	.no_buffer
.13:
	cmp	al,kbd_alt_code+80h
	jne	.14
	and	byte [kbd_flags_1],~kbd_alt_bit ; clear Alt bit
	cmp	byte [kbd_alt_keypad],0		; anything in Alt + Keypad area
	je	.no_buffer				; nothing there - exit
	mov	al,byte [kbd_alt_keypad]	; put it in the buffer
	mov	byte [kbd_alt_keypad],0		; zero the buffer
	mov	ah,0
	jmp	.put_in_buffer

; handle the Ctrl key

.14:
	cmp	al,kbd_ctrl_code
	jne	.15
	or	byte [kbd_flags_1],kbd_ctrl_bit ; set Ctrl bit
	jmp	.no_buffer
.15:
	cmp	al,kbd_ctrl_code+80h
	jne	.16
	and	byte [kbd_flags_1],~kbd_ctrl_bit ; clear Ctrl bit
	jmp	.no_buffer

; handle the Caps Lock key

.16:
	cmp	al,kbd_caps_code
	jne	.17
	or	byte [kbd_flags_2],kbd_caps_bit ; set Caps Lock bit
	xor	byte [kbd_flags_1],kbd_caps_bit ; toggle Caps Lock bit
	jmp	.no_buffer
.17:
	cmp	al,kbd_caps_code+80h
	jne	.18
	and	byte [kbd_flags_2],~kbd_caps_bit ; clear Caps Lock bit
	jmp	set_LEDs

; check if print screen key was pressed

.18:
	cmp	al,kbd_prtsc_code
	jne	.test_scroll_loc
	test	byte [kbd_flags_3],10b	; test E0 flag
	jz	.test_scroll_loc	; jump if no 0E0h flag (Keypad *)
	jmp	print_screen

; handle the Scroll Lock (046h) key and Ctrl-Break combination (0E0h, 046h)

.test_scroll_loc:
	cmp	al,kbd_scroll_code
	jne	.19
	test	byte [kbd_flags_3],10b	; test E0 flag
	jz	.scroll_lock		; jump if no 0E0h flag (Scroll Lock)

; handle Ctrl-Break

	mov	bx,word [kbd_buffer_start] ; empty keyboard buffer
	mov	word [kbd_buffer_head],bx
	mov	word [kbd_buffer_tail],bx
	or	byte [break_flag],80h	; set Ctrl-Break flag
	int	1Bh			; call INT 1Bh - Ctrl-Break handler
	jmp	.no_buffer

; handle Scroll Lock

.scroll_lock:
	or	byte [kbd_flags_2],kbd_scroll_bit ; set Scroll Lock bit
	xor	byte [kbd_flags_1],kbd_scroll_bit ; toggle Scroll Lock bit
	jmp	.no_buffer
.19:
	cmp	al,kbd_scroll_code+80h
	jne	.20
	and	byte [kbd_flags_2],~kbd_scroll_bit ; clear Scroll Lock bit
	jmp	set_LEDs

; handle the Num Lock

.20:
	cmp	al,kbd_num_code
	jne	.21
	test	byte [kbd_flags_1],kbd_ctrl_bit	; Ctrl+Num Lock (Pause)?
	jnz	pause			; jump if pause
	or	byte [kbd_flags_2],kbd_num_bit ; set Num Lock bit
	xor	byte [kbd_flags_1],kbd_num_bit ; toggle Num Lock bit
	jmp	.no_buffer
.21:
	cmp	al,kbd_num_code+80h
	jne	.translate
	and	byte [kbd_flags_2],~kbd_num_bit ; clear Num Lock bit
	jmp	set_LEDs

; translate keyboard scan code to ASCII and BIOS scan code

.translate:
	test	byte [kbd_flags_2],08h	; check pause flag
	jz	.check_release
	and	byte [kbd_flags_2],~08h	; clear pause flag
	jmp	.no_buffer		; don't put key in buffer
.check_release:
	test	al,80h			; ignore key release and KBC responses
	jnz	.no_buffer

	call	scan_xlat
	test	ax,ax		 	; check for bad code
	je	.no_buffer

.put_in_buffer:
	push	si			; XXX - use Int 16h/05?
	mov	bx,word [kbd_buffer_tail]
	mov	si,bx
	add	bx,2
	cmp	bx,word [kbd_buffer_end]
	jne	.1
	mov	bx,word [kbd_buffer_start]
.1:
	cmp	bx,word [kbd_buffer_head]
	je	.no_space		; no space in buffer
	mov	word [si],ax
	mov	word [kbd_buffer_tail],bx

.no_space:
	pop	si

.no_buffer:
	and     byte [kbd_flags_3],0FCh	; E0, E1 not last code

keyboard_int_exit:
%ifdef AT_KEYBOARD
%ifndef DISABLE_KBD_DURING_INTERRUPTS
	mov	al,kbc_cmd_kbd_ena	; enable keyboard interface
	call	kbc_send_cmd_byte
%endif
%endif ; AT_KEYBOARD
	mov	al,20h			; send EOI (end of interrupt)
	out	pic1_reg0,al		; to the 8259A PIC.

keyboard_int_exit1:
	pop	ds
	pop	bx
	pop	ax
	iret

pause:
	xor	byte [kbd_flags_2],08h	; toggle pause flag
	jz	keyboard_int_exit	; jump if pause flag is unset now

	mov	al,20h			; send EOI (end of interrupt)
	out	pic1_reg0,al		; to the 8259A PIC

.pause_loop:
	test	byte [kbd_flags_2],08h
	jnz	.pause_loop		; wait if pause flag is set
	jmp	keyboard_int_exit1

print_screen:
	mov	al,20h			; send EOI (end of interrupt)
	out	pic1_reg0,al		; to the 8259A PIC
	int	5h			; INT 5 - print screen
	and     byte [kbd_flags_3],0FDh	; clear E0 flag
	jmp	keyboard_int_exit1

;=========================================================================
; set_LEDs - Update the kbd_flags_4 LED bits from the kbd_flags_1
; 	     variable and update LEDs on the keyboard
;-------------------------------------------------------------------------
set_LEDs:
	mov	al,20h			; send EOI (end of interrupt)
	out	pic1_reg0,al		; to the 8259A PIC

	mov	al,byte [kbd_flags_1]
	shr	al,1
	shr	al,1
	shr	al,1
	shr	al,1
	and	al,111b
	and	byte [kbd_flags_4],0F8h	; clear LED bits.
	or	byte [kbd_flags_4],al	; mask in new bits.
%ifdef AT_KEYBOARD
%ifndef DISABLE_KBD_DURING_INTERRUPTS
	mov	al,kbc_cmd_kbd_ena	; enable keyboard interface
	call	kbc_send_cmd_byte
%endif
	mov	ah,al			; save LED bits

	mov	al,kbc_cmd_kbd_dis	; disable keyboard interface
	call	kbc_send_cmd_byte

	mov	al,0EDh			; set LEDs cmd
	call	kbc_kb_send		; send the command to the keyboard
	mov	al,ah			; get parameter byte
	call	kbc_kb_send		; send parameter to the keyboard

	mov	al,kbc_cmd_kbd_ena	; enable keyboard interface
	call	kbc_send_cmd_byte
	mov	al,0F4h			; restart keyboard scanning
	call	kbc_kb_send
%endif ; AT_KEYBOARD
	jmp	keyboard_int_exit1

;=========================================================================
; kbd_buffer_init - Setup keyboard buffer
;-------------------------------------------------------------------------
kbd_buffer_init:
	mov	ax,kbd_buffer		; setup keyboard buffer
	mov	word [kbd_buffer_start],ax
	mov	word [kbd_buffer_head],ax
	mov	word [kbd_buffer_tail],ax
	add	ax,20h			; size of the keyboard buffer
	mov	word [kbd_buffer_end],ax
	xor	ax,ax			; clear keyboard flags
	mov	word [kbd_flags_1],ax
	mov	word [kbd_flags_2],ax
	mov	word [kbd_flags_3],ax
	mov	word [kbd_flags_4],ax
	ret
