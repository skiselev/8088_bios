;=========================================================================
; scancode.inc - Keyboard scancode to ASCII code conversion
;		 and related INT 16h functions:
;	INT 16h - BIOS Keyboard Services
;		- function AH=00h
;		- function AH=01h
;		- function AH=10h
;		- function AH=11h
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

;=========================================================================
; scan_xlat_table - Keyboard scan code to ASCII and
;                   BIOS scan code translation table
; Note 1: Table starts with scancode == 1
; Note 2: The high byte of the 'Normal' field indicates key flags
;	00 - no flags
;	1x - keypad numeric keys, can be used with Alt to enter ASCII codes
;	     x in this case equals digit value
;	20 - key is affected by Caps Log or Num Lock (reverses Shift operation)
;	40 - there is an extended key, so check for 0E0h flag
;	8x - F11, F12 BIOS scan codes
;	     (observation: for all other keys BIOS scan code == kbd scan code)
;-------------------------------------------------------------------------

scan_xlat_table:
;		Normal	Shift	Ctrl	Alt	Key	KeySh	Scan
	dw	001Bh,	011Bh,	011Bh,	01F0h	; Esc		01h
	dw	0031h,	0221h,	0,	7800h	; 1	!	02h
	dw	0032h,	0340h,	0300h,	7900h	; 2	@	03h
	dw	0033h,	0423h,	0,	7A00h	; 3	#	04h
	dw	0034h,	0524h,	0,	7B00h	; 4	$	05h
	dw	0035h,	0625h,	0,	7C00h	; 5	%	06h
	dw	0036h,	075Eh,	071Eh,	7D00h	; 6	^	07h
	dw	0037h,	0826h,	0,	7E00h	; 7	&	08h
	dw	0038h,	092Ah,	0,	7F00h	; 8	*	09h
	dw	0039h,	0A28h,	0,	8000h	; 9	(	0Ah
	dw	0030h,	0B29h,	0,	8100h	; 0	)	0Bh
	dw	002Dh,	0C5Fh,	0C1Fh,	8200h	; -	_	0Ch
	dw	003Dh,	0D2Bh,	0,	8300h	; =	+	0Dh
	dw	0008h,	0E08h,	0E7Fh,	0EF0h	; Back Space	0Eh
	dw	0009h,	0F00h,	9400h,	0A500h	; Tab		0Fh
	dw	2071h,	1051h,	1E11h,	1E00h	; q	Q	10h
;		Normal	Shift	Ctrl	Alt	Key	KeySh	Scan
	dw	2077h,	1157h,	1117h,	1100h	; w	W	11h
	dw	2065h,	1245h,	1205h,	1200h	; e	E	12h
	dw	2072h,	1352h,	1312h,	1300h	; r	R	13h
	dw	2074h,	1454h,	1414h,	1400h	; t	T	14h
	dw	2079h,	1559h,	1519h,	1500h	; y	Y	15h
	dw	2075h,	1655h,	1615h,	1600h	; u	U	16h
	dw	2069h,	1749h,	1709h,	1700h	; i	I	17h
	dw	206Fh,	184Fh,	180Fh,	1800h	; o	O	18h
	dw	2070h,	1950h,	1910h,	1900h	; p	P	19h
	dw	005Bh,	1A7Bh,	1A1Bh,	1AF0h	; [	{	1Ah
	dw	005Dh,	1B7Dh,	1B1Dh,	1BF0h	; ]	}	1Bh
	dw	400Dh,	1C0Dh,	1C0Ah,	1CF0h	; Enter		1Ch
	dw	0,	0,	0,	0,	; Ctrl		1Dh
	dw	2061h,	1E41h,	1E01h,	1E00h	; a	A	1Eh
	dw	2073h,	1F53h,	1F13h,	1F00h	; s	S	1Fh
	dw	2064h,	2044h,	2004h,	2000h	; d	D	20h
;		Normal	Shift	Ctrl	Alt	Key	KeySh	Scan
	dw	2066h,	2146h,	2106h,	2100h	; f	F	21h
	dw	2067h,	2247h,	2207h,	2200h	; g	G	22h
	dw	2068h,	2348h,	2308h,	2300h	; h	H	23h
	dw	206Ah,	244Ah,	240Ah,	2400h	; j	J	24h
	dw	206Bh,	254Bh,	250Bh,	2500h	; k	K	25h
	dw	206Ch,	264Ch,	260Ch,	2600h	; l	L	26h
	dw	003Bh,	273Ah,	0,	27F0h	; ;	:	27h
	dw	0027h,	2822h,	0,	28F0h	; '	"	28h
	dw	0060h,	297Eh,	0,	29F0h	; `	~	29h
	dw	0,	0,	0,	0	; Left Shift	2Ah
	dw	005Ch,	2B7Ch,	2B1Ch,	2BF0h	; \	|	2Bh
	dw	207Ah,	2C5Ah,	2C1Ah,	2C00h	; z	Z	2Ch
	dw	2078h,	2D58h,	2D18h,	2D00h	; x	X	2Dh
	dw	2063h,	2E43h,	2E03h,	2E00h	; c	C	2Eh
	dw	2076h,	2F56h,	2F06h,	2F00h	; v	V	2Fh
	dw	2062h,	3042h,	3006h,	3000h	; b	B	30h
;		Normal	Shift	Ctrl	Alt	Key	KeySh	Scan
	dw	206Eh,	314Eh,	310Eh,	3100h	; n	N	31h
	dw	206Dh,	324Dh,	320Dh,	3200h	; m	M	32h
	dw	002Ch,	333Ch,	0,	33F0h	; ,	<	33h
	dw	002Eh,	343Eh,	0,	34F0h	; .	>	34h
	dw	602Fh,	353Fh,	0,	35F0h	; /	?	35h
	dw	0,	0,	0,	0	; Right Shift	36h
	dw	602Ah,	0,	9600h,	37F0h	; *	PrtSc	37h
	dw	0,	0,	0,	0	; Alt		38h
	dw	0020h,	3920h,	3920h,	3920h	; Space		39h
	dw	0,	0,	0,	0	; Caps Lock	3Ah
	dw	0000h,	5400h,	5E00h,	6800h	; F1		3Bh
	dw	0000h,	5500h,	5F00h,	6900h	; F2		3Ch
	dw	0000h,	5600h,	6000h,	6A00h	; F3		3Dh
	dw	0000h,	5700h,	6100h,	6B00h	; F4		3Eh
	dw	0000h,	5800h,	6200h,	6C00h	; F5		3Fh
	dw	0000h,	5900h,	6300h,	6D00h	; F6		40h
;		Normal	Shift	Ctrl	Alt	Key	KeySh	Scan
	dw	0000h,	5A00h,	6400h,	6E00h	; F7		41h
	dw	0000h,	5B00h,	6500h,	6F00h	; F8		42h
	dw	0000h,	5C00h,	6600h,	7000h	; F9		43h
	dw	0000h,	5D00h,	6700h,	7100h	; F10		44h
	dw	0,	0,	0,	0	; Num Lock	45h
	dw	0,	0,	0,	0	; Scroll Lock	46h
	dw	7700h,	4737h,	7700h,	9700h	; Home	7	47h
	dw	7800h,	4838h,	8D00h,	9800h	; Up 	8	48h
	dw	7900h,	4939h,	8400h,	9900h	; PgUp	9	49h
	dw	002Dh,	4A2Dh,	8E00h,	4AF0h	; Gray -	4Ah
	dw	7400h,	4B34h,	7300h,	9B00h	; Left	4	4Bh
	dw	75F0h,	4C35h,	8F00h,	4CF0h	; Center5	4Ch
	dw	7600h,	4D36h,	7400h,	9D00h	; Right	6 	4Dh
	dw	002Bh,	4E2Bh,	0,	4EF0h	; Gray +	4Eh
	dw	7100h,	4F31h,	7500h,	9F00h	; End	1	4Fh
	dw	7200h,	5032h,	9100h,	0A000h	; Down	2	50h
;		Normal	Shift	Ctrl	Alt	Key		Scan
	dw	7300h,	5133h,	7600h,	0A100h	; PgDn	3	51h
	dw	7000h,	5230h,	9200h,	0A200h	; Ins	0	52h
	dw	6000h,	532Eh,	9300h,	0A300h	; Del	.	53h
	dw	0,	0,	0,	0	; SysRq		54h
	dw	0,	0,	0,	0	; NoKey		55h
	dw	0,	0,	0,	0	; NoKey		56h
	dw	8500h,	8700h,	8900h,	8B00h	; F11		57h
	dw	8600h,	8800h,	8A00h,	8C00h	; F12		58h
.end:
scan_xlat_table_size equ (.end-scan_xlat_table)/8

;=========================================================================
; scan_xlat - Translate scan code to ASCII character / scan code pair
; Input:
;	AL - scan code
; Output:
;	AX - ASCII character / scan code pair
;-------------------------------------------------------------------------
scan_xlat:
	push	bx
	dec	al
	cmp	al,scan_xlat_table_size
	jb	.1
	xor	ax,ax
	jmp	.exit

.1:
	mov	bh,00h				; calculate offset of the key
	mov	bl,al				; in scan_xlat_table
	shl	bx,1				; BX = AL * 8
	shl	bx,1
	shl	bx,1

	test	byte [kbd_flags_1],kbd_alt_bit	; is the Alt key pressed?
	jz	.check_ctrl			; Alt key is not pressed
    cs	mov	ax,word [scan_xlat_table+6+bx]	; Alt codes are in 4th column
	; check for numeric Keypad keys
    cs  test	byte [scan_xlat_table+1+bx],10h ; keypad?
	jz	.check_extended
	test	byte [kbd_flags_3],10b		; previous scan code was 0E0h?
	jnz	.check_extended			; ... yes, it is a 0E0h
						; and so it is not a keypad key

    cs	mov	al,byte [scan_xlat_table+1+bx]
	and	al,0Fh				; get the numeric value
	mov	ah,byte [kbd_alt_keypad]	; load value from work area
	aad	0Ah				; AL = AH * 10 + AL
	mov	byte [kbd_alt_keypad],al	; store it back to the work area

	xor	ax,ax				; return nothing
	jmp	.exit

.check_ctrl:
	test	byte [kbd_flags_1],kbd_ctrl_bit	; is the Ctrl key pressed?
	jz	.check_locks			; Ctrl key is not pressed
    cs	mov	ax,word [scan_xlat_table+4+bx]	; Ctrl codes are in 3rd column
	jmp	.check_extended

.check_locks:
    cs	test	byte [scan_xlat_table+1+bx],20h	; is this key affected by Locks?
	jz	.check_shift			; not affected by Locks

	cmp	al,46h				; Num Lock affects scancodes
	jae	.check_num_lock			; above or equal 47h

	test	byte [kbd_flags_1],kbd_caps_bit	; is Caps Lock active?
	jz	.check_shift			; Caps Lock is not active
	jmp	.lock_active

.check_num_lock:
	test    byte [kbd_flags_1],kbd_num_bit	; is Num Lock active?
	jz	.check_shift			; Num Lock is not active
.lock_active:
	; are any of Shift keys active?
	test	byte [kbd_flags_1],kbd_lshft_bit|kbd_rshft_bit
	jz	.shift				; Lock active, Shift inactive
	jmp	.no_shift			; Lock active, Shift active

.check_shift:
	; are any of Shift keys active?
	test	byte [kbd_flags_1],kbd_lshft_bit|kbd_rshft_bit
	jz	.no_shift
.shift:
    cs	mov	ax,word [scan_xlat_table+2+bx]	; Shift codes are in 2nd column
	jmp	.check_extended

.no_shift:
	mov	ah,al				; Move scan code to AH
	inc	ah
    cs	mov	al,byte [scan_xlat_table+bx]	; Normal codes are in 1st column
    cs	test	byte [scan_xlat_table+1+bx],80h	; F11 or F12?
	jz	.check_extended			; not F11 or F12
    cs  mov	ah,byte [scan_xlat_table+bx+1]	; Load scan code for F11 or F12
	jmp	.exit

.check_extended:
   cs	test	byte [scan_xlat_table+1+bx],40h	; have an extended key?
	jz	.exit				; no extended key
	test	byte [kbd_flags_3],10b		; previous scan code was 0E0h?
	jz	.exit				; ... not a 0E0h
	cmp	al,00h
	jnz	.ascii_code			; key has an ASCII code
	mov	al,0E0h				; indicate extended key
	jmp	.exit

.ascii_code:
	mov	ah,0E0h				; indicate extended key for a
						; key with valid ASCII code
						; (Keypad Enter, *, and /)

.exit:
	pop	bx
	ret

;=========================================================================
; int_16_fn00 - get keystroke
; Input:
;	AH = 00h
; Output:
;	AH = BIOS scan code
;	AL = ASCII character
; Notes:
;	- Waits for the keystroke if keyboard buffer is empty
;	- Converts extended keystrokes (101+ key keyboards) to standard
;	  keystrokes, discards extended keystrokes without standard analogs
;	- Use function AH=10h for extended keystrokes handling
;-------------------------------------------------------------------------
int_16_fn00:
	sti
.1:
	mov	bx,word [kbd_buffer_head]
	cmp	bx,word [kbd_buffer_tail] ; keystroke available?
	jz	.1			; wait or keystroke
	mov	ax,word [bx]		; get the keystroke
	add	bx,2			; move kbd_buffer_head to the next
					;   location
	cmp	bx,word [kbd_buffer_end]
	jne	.2
	mov	bx,word [kbd_buffer_start]
.2:
	mov	word [kbd_buffer_head],bx
	cmp	al,0E0h			; a convertible (to standard)
					;   extended key?
	jne	.3			; not a convertible key
	mov	al,00h			; convert to a standard key
	jmp	.exit
.3:
	cmp	al,0F0h			; an ignored extended key?
	je	.1			; an ignored extended key, get next key
	cmp	ah,084h			; an ignored extended key?
					;   (scancode above 84h)
	ja	.1			; an ignored extended key, get next key
	cmp	ah,0E0h			; a convertible ASCII extended key?
	jne	.exit
	cmp	al,0Ah			; LF (Ctrl + Keypad Enter)
	jne	.4
	mov	ah,1Ch			; standard Enter scancode
	jmp	.exit
.4:
	cmp	al,0Dh			; CR (Keypad Enter)
	jne	.5
	mov	ah,1Ch			; standard Enter scancode
	jmp	.exit
.5:
	cmp	al,2Fh			; Keypad /
	jne	.6
	mov	ah,35h			; standard / scancode
	jmp	.exit
.6:
	cmp	al,2Ah			; Keypad *
	jne	.exit
	mov	ah,37h			; standard * scancode
.exit:
	jmp	int_16_exit

;=========================================================================
; int_16_fn01 - check for keystroke
; Input:
;	AH = 01h
; Output:
;	ZF = 1 - no keystroke available
;	ZF = 0 = keystroke is available and put in AX
;		AH = BIOS scan code
;		AL = ASCII character
; Notes:
;	- Does not remove keystroke from keyboard buffer
;	- Converts extended keystrokes (101+ key keyboards) to standard
;	  keystrokes, discards extended keystrokes without standard analogs
;	- Use function AH=11h for extended keystrokes handling
;-------------------------------------------------------------------------
int_16_fn01:
	mov	bx,word [kbd_buffer_head]
	mov	ax,word [bx]
	cmp	bx,word [kbd_buffer_tail]
	jz	.exit			; no keystroke, exit
	cmp	al,0E0h			; a convertible (to standard)
					;   extended key?
	jne	.1			; not a convertible key
	mov	al,00h			; convert to a standard key
	jmp	.clear_zf
.1:
	cmp	al,0F0h			; an ignored extended key?
	je	.discard_key		; an ignored extended key, discard
	cmp	ah,084h			; an ignored extended key?
					;   (scancode above 84h)
	ja	.discard_key		; an ignored extended key, discard
	cmp	ah,0E0h			; a convertible ASCII extended key?
	jne	.clear_zf
	cmp	al,0Ah			; LF (Ctrl + Keypad Enter)
	jne	.2
	mov	ah,1Ch			; standard Enter scancode
	jmp	.clear_zf
.2:
	cmp	al,0Dh			; CR (Keypad Enter)
	jne	.3
	mov	ah,1Ch			; standard Enter scancode
	jmp	.clear_zf
.3:
	cmp	al,2Fh			; Keypad /
	jne	.4
	mov	ah,35h			; standard / scancode
	jmp	.clear_zf
.4:
	cmp	al,2Ah			; Keypad *
	jne	.clear_zf
	mov	ah,37h			; standard * scancode

.clear_zf:
	or	ax,ax			; clear ZF (AX is always non-zero)
.exit:
	jmp	int_16_exitf

.discard_key:
	add	bx,2			; move kbd_buffer_head to the next
					;   location
	cmp	bx,word [kbd_buffer_end]
	jne	.5
	mov	bx,word [kbd_buffer_start]
.5:
	mov	word [kbd_buffer_head],bx
	jmp	int_16_fn01		; check for the next key
	

;=========================================================================
; int_16_fn10 - get extended keystroke
; Input:
;	AH = 10h
; Output:
;	AH = BIOS scan code
;	AL = ASCII character
; Note:
;	- Waits for the keystroke if keyboard buffer is empty
;-------------------------------------------------------------------------
int_16_fn10:
	sti
.1:
	mov	bx,word [kbd_buffer_head]
	cmp	bx,word [kbd_buffer_tail] ; keystroke available?
	jz	.1			; wait or keystroke
	mov	ax,word [bx]		; get the keystroke
	add	bx,2			; move kbd_buffer_head to the next
					;   location
	cmp	bx,word [kbd_buffer_end]
	jne	.2
	mov	bx,word [kbd_buffer_start]
.2:
	mov	word [kbd_buffer_head],bx
	cmp	al,0F0h			; an extended key?
	jne	.exit			; not an extended key
	cmp	ah,00h			; is it Alt + 240 (F0 00)?
	je	.exit			; yes it is Alt + 240, return ASCII
	mov	al,00h			; set ASCII character to zero

.exit:
	jmp	int_16_exit

;=========================================================================
; int_16_fn11 - check for extended keystroke
; Input:
;	AH = 01h
; Output:
;	ZF = 1 - no keystroke available
;	ZF = 0 = keystroke is available and put in AX
;		AH = BIOS scan code
;		AL = ASCII character
; Note:
;	- Does not remove keystroke from keyboard buffer
;-------------------------------------------------------------------------
int_16_fn11:
	mov	bx,word [kbd_buffer_head]
	mov	ax,word [bx]
	cmp	bx,word [kbd_buffer_tail]
	jz	.exit			; no keystroke, exit
	cmp	al,0F0h			; an extended key?
	jne	.clear_zf		; not a extended key
	mov	al,00h			; set scancode to zero
.clear_zf:
	or	ax,ax			; clear ZF (AX is always non-zero)
.exit:
	jmp	int_16_exitf
