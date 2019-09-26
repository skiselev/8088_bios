;=========================================================================
; rtc.inc - RTC/CMOS read and write functions
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

;-------------------------------------------------------------------------
; RTC ports
rtc_addr_reg	equ	70h	; RTC address port
rtc_data_reg	equ	71h	; RTC data port

;-------------------------------------------------------------------------
; locations in RTC and NVRAM
cmos_seconds	equ	00h	; seconds location in RTC
cmos_alarm_secs	equ	01h	; alarm seconds location in RTC
cmos_minutes	equ	02h	; minutes location in RTC
cmos_alarm_mins	equ	03h	; alarm minutes location in RTC
cmos_hours	equ	04h	; hours locaiton in RTC
cmos_alarm_hrs	equ	05h	; alarm hours location in RTC
cmos_day	equ	06h	; day location in RTC
cmos_date	equ	07h	; date location in RTC
cmos_month	equ	08h	; month location in RTC
cmos_year	equ	09h	; year location in RTC
cmos_floppy	equ	10h	; floppy type byte
cmos_equip	equ	14h	; equipment byte
cmos_config_a	equ	2Dh	; BIOS configuration byte A
cmos_sum_hi	equ	2Eh	; checksum of bytes 10h - 20h - high byte
cmos_sum_lo	equ	2Fh	; checksum of bytes 10h - 20h - low byte 
cmos_century	equ	32h	; centry location in RTC (DS12C887 only)

;-------------------------------------------------------------------------
; RTC control register and their bits
cmos_control_a	equ	0Ah	; RTC control A register
cmos_uip	equ	80h	; RTC update in progress bit
cmos_control_b	equ	0Bh	; RTC control B register
cmos_dse	equ	01h	; RTC daylight savings enable bit
cmos_24hours	equ	02h	; RTC 24 hours format (1 = 24 hours, 0 = 12)
cmos_uie	equ	10h	; RTC update ended interrupt enable bit
cmos_aie	equ	20h	; RTC alarm interrupt enable bit
cmos_pie	equ	40h	; RTC periodic interrupt enable bit
cmos_set	equ	80h	; RTC set bit (0 = normal operation, 1 = set)
cmos_control_c	equ	0Ch	; RTC control C register
cmos_uf		equ	20h	; RTC update ended interrupt flag
cmos_af		equ	40h	; RTC alarm interrupt flag
cmos_pf		equ	80h	; RTC periodic interrupt flag
cmos_control_d	equ	0Dh	; RTC control D register
cmos_vrt	equ	80h	; RTC vrt bit (1 = battery is OK)

;=========================================================================
; rtc_read - Read byte from RTC or CMOS memory
; Input:
;	AL - address and NMI enable bit
;		bits 6-0 - address of byte to read
;		bit 7    - 0 = disable NMI, 1 = enable NMI
; Output:
;	AL = byte from RTC
;-------------------------------------------------------------------------
rtc_read:
	cli
	out	rtc_addr_reg,al
	jmp	$+2
	jmp	$+2
	jmp	$+2
	jmp	$+2
	in	al,rtc_data_reg
	sti
	ret

;=========================================================================
; rtc_write - Read byte to RTC or CMOS memory
; Input:
;	AL - address and NMI enable bit
;		bits 6-0 - address of byte to read
;		bit 7    - 0 = disable NMI, 1 = enable NMI
;	AH = byte to write to RTC
;-------------------------------------------------------------------------
rtc_write:
	cli
	out	rtc_addr_reg,al
	jmp	$+2
	jmp	$+2
	jmp	$+2
	jmp	$+2
	xchg	ah,al
	out	rtc_data_reg,al
	xchg	ah,al
	sti
	ret

;=========================================================================
; rtc_init - Initialize RTC
; Notes:
;	- makes sure RTC battery is OK, resets time if not
;	- disables RTC interrupts
;	- validates NVRAM checksum, loads default values if invalid
;-------------------------------------------------------------------------
rtc_init:
	push	ax
	push	bx
	push	cx
	push	dx
	push	si
	push	di
	mov	al,cmos_control_a	; select control A register
	mov	ah,26h			; turn on oscillator and time keeping
					; set SQW frequency to 1.024 KHz
	call	rtc_write		; write control register A

%ifdef AT_RTC_AUTODETECT
	call	rtc_read		; read back control A register
	cmp	al,26h
	jne	.exit			; RTC is not responding, exit
%endif ; AT_RTC_AUTODETECT

	mov	al,cmos_control_b
	call	rtc_read
	mov	ah,al
	and	ah,cmos_dse		; clear all bits except of DSE
	or	ah,cmos_24hours		; set 24 hours bit, keep BCD format and
					; interrupts disabled
	mov	al,cmos_control_b
	call	rtc_write		; write control register B

	mov	al,cmos_control_c
	call	rtc_read		; read control register C - reset
					; interrupt flags

	mov	al,cmos_control_d
	call	rtc_read		; read control register D
	test	al,cmos_vrt
	jnz	.1			; RTC battery is OK
	mov	si,msg_rtc_bad
	call	print
; RTC is bad, set initial time
	mov	ah,03h			; int 1Ah, function 03h - set RTC time
	xor	cx,cx
	xor	dx,dx
	int	1Ah
	mov	ah,05h			; int 1Ah, function 05h - set RTC date
	mov	cx,2010h		; year 2010
	mov	dx,0101h		; January 1st
	int	1Ah

.1:
; set timer variables to RTC time
	mov	ah,02h			; int 1Ah, function 02h - get RTC time
	int	1Ah

; convert time to ticks * 2^11

; ticks = seconds * 37287
	mov	al,dh
	call	bcd_to_binary		; convert seconds to binary

	mov	dx,37287
	mul	dx			; DX:AX = seconds * 37287

	mov	si,ax
	mov	di,dx

; ticks += minutes * 2237216 = minutes * 8992 + minutes * 34 * 2^16
	mov	al,cl
	call	bcd_to_binary		; convert minutes to binary

	mov	bx,ax
	mov	dx,8992
	mul	dx			; DX:AX = minutes * 8992

	add	si,ax
	adc	di,dx

	mov	ax,bx
	mov	dx,34
	mul	dx

	add	di,ax

; ticks += hours * 134232938 = hours * 15210 + hours * 2048 * 2^16
	mov	al,ch
	call	bcd_to_binary		; convert hours to binary

	mov	bx,ax
	mov	dx,15210
	mul	dx			; DX:AX = hours * 15210

	add	si,ax
	adc	di,dx

	mov	ax,bx
	mov	dx,2048
	mul	dx			; AX = hours * 2048

	add	di,ax

; CX:DX = DI:SI / 2048
	mov	cl,11
	shr	si,cl
	mov	dx,di
	mov	cl,5
	shl	dx,cl
	or	dx,si

	mov	cl,11
	shr	di,cl
	mov	cx,di

					; CX = high word of tick count
					; DX = low word of tick count
	
	mov	ah,01h			; int 1Ah, function 01h - set time
	int	1Ah


%ifdef AT_RTC_NVRAM
; compare NVRAM checksum with stored value

	call	nvram_checksum

	mov	al,cmos_sum_hi
	call	rtc_read
	mov	ah,al
	mov	al,cmos_sum_lo
	call	rtc_read
	cmp	bx,ax
	je	.update_equipment

	mov	si,msg_rtc_sum
	call	print

; clear NVRAM

	mov	ax,0010h		; start from 10h, load 00h

.nvram_clear_loop:
	call	rtc_write
	inc	al
	cmp	al,20h			; last address is 20h
	jbe	.nvram_clear_loop

	mov	al,cmos_floppy		; write default floppy type
    cs	mov	ah,byte [default_floppy]
	call	rtc_write

	mov	al,cmos_equip		; write default equipment byte
    cs	mov	ah,byte [default_equip]
	call	rtc_write

; update checksum

	call	nvram_checksum

	mov	al,cmos_sum_hi
	mov	ah,bh
	call	rtc_write

	inc	al			; AL = cmos_sum_lo
	mov	ah,bl
	call	rtc_write

; read equipment byte from NVRAM and set it in BIOS data area

.update_equipment:
	mov	al,cmos_equip
	call	rtc_read
	and	al,~(equip_video|equip_mouse)   ; these are autodetected
	and	byte [equipment_list],equip_video|equip_mouse
	or	byte [equipment_list],al

	mov	al,e_rtc_init_ok	; RTC initialzied successfully
	out	post_reg,al
%endif ; AT_RTC_NVRAM

.exit:
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret

bcd_to_binary:
	push	cx
	mov	ch,al
	and	ch,0Fh
	mov	cl,4
	shr	al,cl
	mov	cl,10
	mul	cl
	add	al,ch
	pop	cx
	ret

%ifdef AT_RTC_NVRAM
;=========================================================================
; nvram_checksum - calculate NVRAM checksum
; Input:
;	none
; Output:
;	BX = NVRAM checksum
;-------------------------------------------------------------------------
nvram_checksum:
	push	ax
	xor	bx,bx
	mov	ah,10h			; start from 10h

.checksum_loop:
	mov	al,ah
	call	rtc_read
	add	bl,al			; BX += AL
	adc	bh,0
	inc	ah
	cmp	ah,20h			; last address is 20h
	jbe	.checksum_loop
	pop	ax
	ret

;=========================================================================
; nvram_save - Save configuration to CMOS memory
; Input:
;	CH - CPU Clock speed
;	     0 - 4.77 Mhz / Normal
;	     1 - 7.16 Mhz / Turbo
;	     2 - 9.55 Mhz (FE2010A only)
;
;	CL - CMOS floppy drive type
;	     7-4 - first floppy drive type
;	     3-0 - second floppy drive type
;-------------------------------------------------------------------------
nvram_save:
	push	ax
	push	bx

	mov	al,cmos_config_a
	mov	ah,ch
	call	rtc_write

	mov	al,cmos_floppy
	mov	ah,cl
	call	rtc_write

	mov	ah,byte [equipment_list]
	and	ah,03Eh			; mask floppy bits

	test	cl,70h
	jz	.second_floppy		; jump if first floppy is not installed
	or	ah,01h			; first floppy is installed

.second_floppy:
	test	cl,07h
	jz	.save_equipment		; jump if second floppy is not installed
	or	ah,41h			; indicate two floppies
					; (even if the first one is missing)

.save_equipment:
	mov	byte [equipment_list],ah
	mov	al,cmos_equip
	call	rtc_write

	call	nvram_checksum

	mov	al,cmos_sum_hi
	mov	ah,bh
	call	rtc_write

	inc	al			; AL = cmos_sum_lo
	mov	ah,bl
	call	rtc_write

	pop	bx
	pop	ax
	ret

;=========================================================================
; get_config_a - Return BIOS configuration byte A from NVRAM
; Input:
;	none
; Output:
;	AL - NVRAM CPU clock frequency configuration setting
;		0 - 4.77 MHz / Normal
;		1 - 7.16 MHz / Turbo
;		2 - 9.55 MHz (FE2010 only)
;-------------------------------------------------------------------------
get_config_a:
	mov	al,cmos_config_a	; read BIOS configuration byte A
	call	rtc_read
	ret
  
;=========================================================================
; get_floppy - Return floppy drive type from NVRAM
; Input:
;	none
; Output:
;	AL = floppy drive type
;-------------------------------------------------------------------------
get_floppy:
	mov     al,cmos_floppy
	call    rtc_read		; read currently configured floppies
	ret

%endif ; AT_RTC_NVRAM

;=========================================================================
; print_rtc - print current RTC date and time
; Input:
;	none
; Output:
;	date and time are printed in YYYY-MM-DD hh:mm:ss format
; Notes:
;	There is a slight probability of getting inconsistent printout.
;	In case the function is called just before midnight, it could
;	print the previos day's date and next day's time.
;-------------------------------------------------------------------------
print_rtc:
	push	ax
	push	bx
	push	cx
	push	dx
	push	si
	mov	si,msg_rtc
	call	print

%ifdef AT_RTC_AUTODETECT
	call	rtc_detect
	jnc	.rtc_present
	mov	si,msg_absent
	call	print
	jmp	.exit
.rtc_present:
%endif ; AT_RTC_AUTODETCT

; print date
	mov	ah,04h
	int	1Ah			; read RTC date
					; CH = BCD century
					; CL = BCD year
					; DH = BCD month
					; DL = BCD date (day of month)
	mov	ax,cx
	call	print_hex		; print 4-digit year
	mov	ax,(0Eh << 8) + '-'
	mov	bx,0007h
	int	10h			; print dash (-)

	mov	al,dh
	call	print_byte		; print 2-digit month
	mov	ax,(0Eh << 8) + '-'
	mov	bx,0007h
	int	10h			; print dash (-)

	mov	al,dl
	call	print_byte		; print 2-digit date (day of month)

	mov	ax,(0Eh << 8) + ' '
	mov	bx,0007h
	int	10h			; print space ( )

; print time
	mov	ah,02h
	int	1Ah			; read RTC time
					; CH = BCD hours
					; CL = BCD minutes
					; DH = BCD seconds
					; DL = daylight saving flag
	mov	al,ch
	call	print_byte		; print 2-digit hours
	
	mov	ax,(0Eh << 8) + ':'
	mov	bx,0007h
	int	10h			; print colon (:)

	mov	al,cl
	call	print_byte		; print 2-digit minutes
	
	mov	ax,(0Eh << 8) + ':'
	mov	bx,0007h
	int	10h			; print colon (:)

	mov	al,dh
	call	print_byte		; print 2-digit seconds

	mov	si,msg_rtc_dse
	call	print
	cmp	dl,0
	je	.dse_disabled
	mov	si,msg_enabled		; DSE enabled
	call	print
	jmp	.dse_done

.dse_disabled:
	mov	si,msg_disabled		; DSE disabled
	call	print

.dse_done:
	mov	si,msg_crlf
	call	print

.exit:
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret

%ifdef AT_RTC_AUTODETECT
;=========================================================================
; rtc_detect - Check RTC is present at 0x70 port
; Input:
;	none
; Output:
;	CF == 0 - RTC is present
;	CF == 1 - RTC is not detected
;-------------------------------------------------------------------------
rtc_detect:
	push ax
	mov	al,cmos_control_a	; select control A register
	call    rtc_read		; check if RTC is present
	cmp     al,26h
	je	.rtc_present
	stc				; no RTC - set CF
	jmp	.exit

.rtc_present:
	clc				; RTC detected - clear CF

.exit:
	pop ax
	ret
%endif ; AT_RTC_AUTODETECT
