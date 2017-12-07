;=========================================================================
; setup.inc - NVRAM setup functions
; Works either with AT-style RTC/NVRAM or with Flash ROM
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

default_floppy	db	cmos_1440 << 4	; one 1.44 MB floppy drive
default_equip	db	21h		; CGA 80x25, one floppy drive

;=========================================================================
; nvram_setup - NVRAM configuration utility
; Input:
;	none
; Output:
;	none
;-------------------------------------------------------------------------
nvram_setup:
	push	ax
	push	bx
	push	cx
	push	dx
	push	si

%ifdef AT_RTC
	mov	al,cmos_floppy
	call	rtc_read		; read currently configured floppies
%else ; AT_RTC
	call	flash_get_floppy	; read floppies from configuration
					; section of Flash ROM
%endif ; AT_RTC
	mov	cl,al			; save it to CL

	mov	si,msg_set_welcome
	call	print

.menu_loop:
	mov	si,msg_set_prompt
	call	print

.menu_input:
	mov	ah,00h
	int	16h
	or	al,20h			; convert to lower case
	mov	dl,0			; floppy 0
	cmp	al,'f'
	je	.set_floppy
	inc	dl			; floppy 1
	cmp	al,'g'
	je	.set_floppy
	cmp	al,'p'
	je	.print_settings
%ifdef AT_RTC
	cmp	al,'t'
	je	.set_time
	cmp	al,'d'
	je	.set_date
%endif ; AT_RTC
	cmp	al,'w'
	je	.save
	cmp	al,'q'
	je	.no_save
	cmp	al,'h'
	je	.help
	jmp	.menu_input

.help:
	mov	ah,0Eh			; echo
	mov	bx,0007h
	int	10h
	mov	si,msg_set_help
	call	print
	jmp	.menu_loop

.set_floppy:
	mov	ah,0Eh			; echo
	mov	bx,0007h
	int	10h

.set_floppy_prompt:
	mov	si,msg_set_fd_prmt
	call	print

.floppy_input:
	mov	ah,00h
	int	16h
	or	al,20h			; convert to lower case
	cmp	al,'r'
	je	.floppy_exit
	cmp	al,'h'
	je	.floppy_help
	cmp	al,'0'
	jb	.floppy_input
	cmp	al,'6'
	ja	.floppy_input
	cmp	al,'5'
	je	.floppy_input
	mov	ah,0Eh			; echo
	mov	bx,0007h
	int	10h
	sub	al,'0'
	mov	dh,0F0h
	or	dl,dl			; drive	0?
	jnz	.drive_1
	shl	al,1
	shl	al,1
	shl	al,1
	shl	al,1
	mov	dh,0Fh
.drive_1:
	and	cl,dh
	or	cl,al
	jmp	.menu_loop

.floppy_exit:
	mov	ah,0Eh			; echo
	mov	bx,0007h
	int	10h
	jmp	.menu_loop

.floppy_help:
	mov	ah,0Eh			; echo
	mov	bx,0007h
	int	10h
	mov	si,msg_set_fd_help
	call	print
	jmp	.set_floppy_prompt

.print_settings:
	mov	ah,0Eh			; echo
	mov	bx,0007h
	int	10h
	mov	si,msg_crlf
	call	print
%ifdef AT_RTC
	call	print_rtc		; print current RTC time
%endif ; AT_RTC
	mov	al,cl			; floppy bytes to AL
	call	print_floppy
	jmp	.menu_loop

%ifdef AT_RTC
.set_time:
	push	cx
	push	di
	push	es
	mov	ah,0Eh			; echo
	mov	bx,0007h
	int	10h
	mov	si,msg_crlf
	call	print

	mov	si,msg_set_time
	call	print

	mov	ah,02h
	int	1Ah			; get current time

	mov 	ax,07C0h		; use 07C0:0000 as a buffer
	mov	es,ax
	xor	di,di
	mov	ax,8			; length of the time string

	call	get_line

	or	ax,ax			; empty string?
	jz	.set_time_exit

	call	atoi

	cmp	ax,23			; hours are greater than 23?
	ja	.set_time_invalid

	call	bin_to_bcd		; convert to BCD

	mov	ch,al			; CH = hours

    es	cmp	byte [di],':'		; ':' delimiter?
	jne	.set_time_invalid

	inc	di

	call	atoi

	cmp	ax,59			; minutes are greater than 59?
	ja	.set_time_invalid

	call	bin_to_bcd

	mov	cl,al			; CL = minutes

    es	cmp	byte [di],':'		; ':' delimiter?
	jne	.set_time_invalid

	inc	di

	call	atoi

	cmp	ax,59			; seconds are greater than 59?
	ja	.set_time_invalid

	call	bin_to_bcd

	mov	dh,al			; DH = seconds

    es	cmp	byte [di],00h		; end of line?
	jne	.set_time_invalid

	mov	ah,03h			; set RTC time
	int	1Ah

.set_time_exit:
	pop	es
	pop	di
	pop	cx
	jmp	.menu_loop

.set_time_invalid:
	mov	si,msg_time_inval
	call	print
	jmp	.set_time_exit

.set_date:
	push	cx
	push	di
	push	es
	mov	ah,0Eh			; echo
	mov	bx,0007h
	int	10h
	mov	si,msg_crlf
	call	print
	mov	si,msg_set_date
	call	print

	mov 	ax,07C0h		; use 07C0:0000 as a buffer
	mov	es,ax
	xor	di,di
	mov	ax,10			; length of the date string

	call	get_line

	or	ax,ax			; empty string?
	jz	.set_date_exit

	call	atoi

	cmp	ax,9999			; year is greater than 9999?
	ja	.set_date_invalid

	mov	bx,ax			; store binary year in BX

	call	bin_to_bcd		; convert to BCD

	mov	cx,ax			; CH = century, CL = year
	
    es	cmp	byte [di],'-'		; '-' delimiter?
	jne	.set_date_invalid

	inc	di

	call	atoi

	cmp	ax,1			; month is lower than 1?
	jb	.set_date_invalid

	cmp	ax,12			; month is greater than 12?
	ja	.set_date_invalid

	call	bin_to_bcd

	mov	dh,al			; DH = month

    es	cmp	byte [di],'-'		; '-' delimiter?
	jne	.set_date_invalid

	inc	di

	call	atoi

	cmp	ax,1			; day is lower than 1?
	jb	.set_date_invalid

	cmp	dh,02h			; February
	je	.set_date_february

	cmp	dh,04h			; April
	je	.set_date_30_days

	cmp	dh,06h			; July
	je	.set_date_30_days

	cmp	dh,09h			; September
	je	.set_date_30_days

	cmp	dh,11h			; November
	je	.set_date_30_days

.set_date_31_days:
	cmp	ax,31			; day of month is greater than 31?
	ja	.set_date_invalid
	jmp	.set_date_set_day

.set_date_30_days:
	cmp	ax,30			; day of month is greater than 30?
	ja	.set_date_invalid
	jmp	.set_date_set_day

.set_date_29_days:
	cmp	ax,29			; day of month is greater than 28?
	ja	.set_date_invalid
	jmp	.set_date_set_day

.set_date_28_days:
	cmp	ax,28			; day of month is greater than 28?
	ja	.set_date_invalid
	jmp	.set_date_set_day

.set_date_february:
	test	bx,0003h		; check if years divides by 4
	jnz	.set_date_28_days	; not a leap year

	cmp	cl,00h			; year doesn't end with 00
	jne	.set_date_29_days	; a leap year

	test	ch,03h			; check if century divides by 4
	jz	.set_date_29_days	; year divides by 400
	jmp	.set_date_28_days	; year divides by 100 but not by 400

.set_date_set_day:
	call	bin_to_bcd

	mov	dl,al			; DL = date (day of month)

    es	cmp	byte [di],00h		; end of line?
	jne	.set_date_invalid

	mov	ah,05h			; set RTC date
	int	1Ah

.set_date_exit:
	pop	es
	pop	di
	pop	cx
	jmp	.menu_loop

.set_date_invalid:
	mov	si,msg_date_inval
	call	print
	jmp	.set_date_exit

%endif ; AT_RTC

.save:
	mov	ah,0Eh			; echo
	mov	bx,0007h
	int	10h

%ifdef AT_RTC
	call	rtc_save
%else ; AT_RTC
	call	flash_save
%endif ; AT_RTC
	jmp	.exit

.no_save:
	mov	ah,0Eh			; echo
	mov	bx,0007h
	int	10h

.exit:
	mov	si,msg_crlf
	call	print
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
