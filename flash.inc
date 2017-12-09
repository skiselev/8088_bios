;=========================================================================
; flash.inc - Flash ROM programming functions
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

config_segment	equ	0000h
config_space	equ	8000h		; offset of the configuration
flash_floppy	equ	config_space
temp_segment	equ	07C0h		; segment for flash writing code
flash_write	equ	0		; offset for flash writing code

;=========================================================================
; flash_get_floppy - Returns floppy drive type from Flash ROM configuration
;                    area
; Input:
;	none
; Output:
;	AL = floppy drive type
;-------------------------------------------------------------------------
flash_get_floppy:
    cs	mov	al,byte [flash_floppy]
	cmp	al,0FFh			; config area erased in flash?
	jne	.exit
    cs  mov	al,byte [default_floppy] ; load the default configuration
.exit:
	ret

;=========================================================================
; flash_save - Save configuration to Flash ROM
; Input:
;	CL - CMOS floppy drive type
;	     7-4 - first floppy drive type
;	     3-0 - second floppy drive type
;-------------------------------------------------------------------------
flash_save:
	push	ax
	push	dx
	push	si
	push	di
	push	ds
	push	es
	push	cx

	mov	si,msg_saving
	call	print

	mov	ax,cs			; ROM segment
	mov	ds,ax
	mov	ax,temp_segment		; copy flash_write_code here
	mov	es,ax

	mov	si,flash_write_code
	mov	di,flash_write
	mov	cx,flash_write_size

	cld
    rep movsb				; copy the flash_write_code

	mov	ax,config_segment	; copy current flash content here
	mov	es,ax

	mov	si,config_space
	mov	di,si			; use the same offset
	mov	cx,16384		; maximal flash segment size (Am29F010)

	cld
    rep movsb

	pop	cx			; load the original CX value
	push	cx
    es  mov	byte [flash_floppy],cl	; set new floppy drive type
	call	temp_segment:flash_write

	jc	.error

	mov	si,msg_save_ok
	call	print
	jmp	.exit

.error:
	mov	si,msg_save_error
	call	print

.exit:
	pop	cx
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	dx
	pop	ax
	ret


;=========================================================================
; flash_write - Write configuration to Flash ROM
;		Must be copied to RAM and run from it
; Input:
;	Data to write stored at ES:config_space
; Output:
;	CF clear on success
;	CF set on error
;	AX,CX,DX,DS,SI,DI trashed
;-------------------------------------------------------------------------
flash_write_code:
	mov	ax,bioscseg
	mov	ds,ax

	cli				; no interrupts while writing flash
	mov	al,nmi_disable
	out	nmi_mask_reg,al		; disable NMIs too

	mov	ax,word [0]		; get data from first two bytes of flash

; try to identify Flash ROM using 5555h and 0AAAAh addresses for commands
	mov	si,5555h
	mov	di,0AAAAh
	mov	byte [si],0AAh
	mov	byte [di],55h
	mov	byte [si],90h		; enter ID mode command
	mov	cx,1000h
.delay1:
	loop	.delay1			; some ICs need 10ms delay
	mov	cx,word [0]		; get device ID
	cmp	cx,ax			; is it really an ID?
	jne	.select_flash		; yes, try to select the flash type

; try to identify Flash ROM using 0555h and 0AAAh addresses for commands
	mov	si,0555h
	mov	di,0AAAh
	mov	byte [si],0AAh
	mov	byte [di],55h
	mov	byte [si],90h		; enter ID mode command
	mov	cx,1000h
.delay2:
	loop	.delay2			; some ICs need 10ms delay
	mov	cx,word [0]		; get device ID
	cmp	cx,ax			; is it really an ID?
	jne	.select_flash		; yes, try to select the flash type

; unable to get Flash ROM ID, return with error

.error:
	stc
	mov	al,nmi_enable
	out	nmi_mask_reg,al		; enable NMIs
	sti
	retf

.select_flash:
	mov	byte[si],0AAh
	mov	byte[di],55h
	mov	byte[si],0F0h		; exit ID mode command

.try_at29c010:
	cmp	cx,0D51Fh		; 1F = Atmel, D5 = AT29C010A
	jne	.try_sst29ee010
	mov	cx,128			; sector size is 128 bytes
	jmp	short .write_auto_sector

.try_sst29ee010:
	cmp	cx,07BFh		; BF = SST, 07 = SST29EE010/GLS29EE010
	jne	.try_w29ee011
	mov	cx,128			; sector size is 128 bytes
	jmp	short .write_auto_sector

.try_w29ee011:
	cmp	cx,0C1DAh		; DA = Winbond, C1 = W29EE011
	jne	.try_sst39sf010
	mov	cx,128			; sector size is 128 bytes
	jmp	short .write_auto_sector

.try_sst39sf010:
	cmp	cx,0B5BFh		; BF = SST, B5 = SST39SF010
	jne	.try_am29f010
	mov	cx,4096			; sector size is 4 KiB
	jmp	short .write_erase_byte

.try_am29f010:
	cmp	cx,2001h		; 01 = AMD, 20 = Am29F010
	jne	.error
	mov	cx,16384		; sector size is 16 KiB

; write Flash that requires block erase prior to write, and writen byte by byte
	
.write_erase_byte:
	mov	bx,config_space
	mov	byte [si],0AAh
	mov	byte [di],55h
	mov	byte [si],80h		; erase command
	mov	byte [si],0AAh
	mov	byte [di],055h
	mov	byte [bx],30h		; erase sector command
	mov	dx,cx			; save sector size to DX
	xor	cx,cx

.write_erase_byte1:
	mov	al,20h			; additional delay
					; Am29F010 is slow to erase...
.write_erase_byte2:
	dec	al
	jnz	.write_erase_byte2
	cmp	byte [bx],0FFh		; is it erased yet?
	loopne	.write_erase_byte1
	or	cx,cx			; CX ?= 0
	jz	.error			; erase timed out

.write_erase_byte3:
	mov	byte [si],0AAh
	mov	byte [di],55h
	mov	byte [si],0A0h		; enter write mode

    es	mov	al,byte [bx]		; read byte to write
	mov	byte [bx],al		; write it to flash
	xor	cx,cx

.write_erase_byte4:
	cmp	al,byte [bx]		; wait for write operation to complete
	loopne	.write_erase_byte4
	or	cx,cx			; CX ?= 0
	jz	.error			; write timed out

	inc	bx
	dec	dx
	jnz	.write_erase_byte3
	jmp	short .exit

; write Flash that auto erases itself and programmed sector by sector

.write_auto_sector:
	mov	bx,config_space
	mov	byte [si],0AAh
	mov	byte [di],55h
	mov	byte [si],0A0h		; enter write mode

.write_auto_sector1:
    es	mov	al,byte [bx]		; read byte to write
	mov	byte [bx],al		; write it to flash
	inc	bx
	loop	.write_auto_sector1	; write next byte

	xor	cx,cx
	dec	bx

.write_auto_sector2:
	cmp	al,byte [bx]		; wait for write operation to complete
	loopne	.write_auto_sector2
	or	cx,cx			; CX ?= 0
	jz	.error			; write timed out

.exit:
	clc
	mov	al,nmi_enable
	out	nmi_mask_reg,al		; enable NMIs
	sti
	retf

flash_write_size	equ	($-flash_write_code)
%if flash_write_size > ((config_segment << 4) + config_space - (temp_segment << 4))
%error "flash_write code is too big - try increasing changing config_segment"
%endif
