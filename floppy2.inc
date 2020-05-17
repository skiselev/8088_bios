;=========================================================================
; floppy2.inc - BIOS floppy disk services (part 2 of 2)
;	INT 13h, function AH=00h
;       INT 13h, function AH=02h
;       INT 13h, function AH=03h
;       INT 13h, function AH=04h
;       INT 13h, function AH=05h
;	- see floppy1.inc for other INT 13h functions
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
int_13_bp	equ	0
int_13_ds	equ	int_13_bp+2
int_13_di	equ	int_13_ds+2
int_13_si	equ	int_13_di+2
int_13_dx	equ	int_13_si+2
int_13_dl	equ	int_13_dx
int_13_dh	equ	int_13_dx+1
int_13_cx	equ	int_13_dx+2
int_13_cl	equ	int_13_cx
int_13_ch	equ	int_13_cx+1
int_13_bx	equ	int_13_cx+2
int_13_bl	equ	int_13_bx
int_13_bh	equ	int_13_bx+1
int_13_ax	equ	int_13_bx+2
int_13_al	equ	int_13_ax
int_13_ah	equ	int_13_ax+1
int_13_ip	equ	int_13_ax+2
int_13_cs	equ	int_13_ip+2
int_13_flags	equ	int_13_cs+2
int_13_flags_l	equ	int_13_flags

;=========================================================================
; int_13 - BIOS floppy disk services
; Input:
;	AH = function
;		00h - Reset disk system
;		01h - Get status of last operation
;		02h - Read disk sectors
;		03h - Write disk sectors
;		04h - Verify disk sectors
;		05h - Format track
;		08h - Get drive parameters
;		15h - Get disk type
;		16h - Detect disk change
;		17h - Set disk type for format
;		18h - Set media type for format
; Output:
;	- depends on function
;	- for most functions:
;		CF clear if successful
;			AH = 00h - successful completion
;		CF set on error
;			AH = error code
;-------------------------------------------------------------------------
	setloc	0EC59h			; INT 13 (Floppy) Entry Point
int_13:
	sti
	push	ax
	push	bx
	push	cx
	push	dx
	push	si
	push	di
	push	ds
	push	bp
	mov	bp,sp
	cmp	ah,.num_func
	ja	.invalid_function
	mov	al,ah
	cbw
	mov	di,ax
	shl	di,1
	mov	ax,biosdseg
	mov	ds,ax
    cs	jmp	near [.dispatch+di]

.dispatch:
	dw	int_13_fn00		; Reset disk system
	dw	int_13_fn01		; Get status of last operation
	dw	int_13_fn02		; Read disk sectors
	dw	int_13_fn03		; Write disk sectors
	dw	int_13_fn04		; Verify disk sectors
	dw	int_13_fn05		; Format track
	dw	.invalid_function	; AH = 06h
	dw	.invalid_function	; AH = 07h
	dw	int_13_fn08		; Get drive parameters
	dw	.invalid_function	; AH = 09h
	dw	.invalid_function	; AH = 0Ah
	dw	.invalid_function	; AH = 0Bh
	dw	.invalid_function	; AH = 0Ch
	dw	.invalid_function	; AH = 0Dh
	dw	.invalid_function	; AH = 0Eh
	dw	.invalid_function	; AH = 0Fh
	dw	.invalid_function	; AH = 10h
	dw	.invalid_function	; AH = 11h
	dw	.invalid_function	; AH = 12h
	dw	.invalid_function	; AH = 13h
	dw	.invalid_function	; AH = 14h
	dw	int_13_fn15		; Get disk type
	dw	int_13_fn16		; Detect disk change
	dw	int_13_fn17		; Set disk type for format
	dw	int_13_fn18		; Set media type for format
.num_func	equ ($-.dispatch)/2

.invalid_function:
	mov	ah,fdc_e_invalid
	mov	byte [fdc_last_error],ah
	stc				; error condition
	jmp	int_13_exit

;-------------------------------------------------------------------------
; int_13_upd_exit - Update diskette change flag and exit

int_13_upd_exit:
	pushf
	push	ax
	mov	al,byte [bp+int_13_dl]	; get drive number
	cmp	al,1
	ja	.no_update		; skip if invalid drive number
	mov	bx,fdc_media_state
	add	bl,al			; BX -> drive media state
	test	byte [bx],fdc_m_established
	jz	.no_update		; skip if media type not established
	mov	ah,04h			; set bit 2 (drive 0 type determined)
	or	al,al
	jz	.drive_0
	mov	ah,40h			; set bit 6 (drive 1 type determined)

.drive_0:
	or	byte [fdc_info],ah	; set media detected bit

.no_update:
	pop	ax
	popf

int_13_exit:
	mov	byte [bp+int_13_ah],ah	; pass AH to the caller
	mov	ax,201h			; set IF and CF
	jc	.set_error		; there is an error
	and	byte [bp+int_13_flags_l],0FEh ; no errors - clear CF
	dec	ax			; clear CF in AX too

.set_error:
	or	word [bp+int_13_flags],ax
	pop	bp
	pop	ds
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	iret

;=========================================================================
; int_13_fn00: Reset disk system
; Input:
;	AH = 00h
;	DL = drive number (bit 7 not set - floppy drive)
; Output:
;	CF clear if successful
;		AH = 00h - successful completion
;	CF set on error
;		AH = 20h - controller failure
;		AH = 80h - timeout / device not ready
; Note:
;	This function doesn't check drive number, but HDD BIOS (if installed)
;	normally will call this functions if bit 7 of DL is not set
;-------------------------------------------------------------------------
int_13_fn00:
	call	fdc_init
	mov	[fdc_last_error],ah	; save the error code
	jmp	int_13_exit

;=========================================================================
; fdc_init - Initialize floppy disk controller
; Input:
;	none
; Output:
;	CF clear on success
;		AH = 00h - successful completion
;	CF set if error
;		AH = 20h - controller failure
;		AH = 80h - timeout / device not ready
;-------------------------------------------------------------------------
fdc_init:
	and	byte [fdc_calib_state],0F0h ; require recalibration
	call	fdc_reset		; reset FDC
	jc	fdc_init_error
	mov	dx,fdc_status_reg
	in	al,dx
	test	al,80h
	jz	.try_again		; try again if FDC not ready
	test	al,40h
	jz	.fdc_ready		; FDC ready to recieve data
.try_again:
	call	fdc_reset
	mov	dx,fdc_status_reg
	in	al,dx
	test	al,80h
	jz	fdc_init_error
	test	al,40h
	jnz	fdc_init_error
.fdc_ready:
	mov	al,08h			; FDC Sense Interrupt Status command
	call	fdc_write		; send the command
	jc	fdc_init_error
	call	fdc_read		; read ST0
	jc	fdc_init_error
	mov	byte [fdc_ctrl_status],al	; save ST0
	mov	ah,al			; save AL to AH
	call	fdc_read		; read current cylinder
	jc	fdc_init_error
	mov	byte [fdc_ctrl_status+1],al	; save
	and	ah,0C0h
	cmp	ah,0C0h			; abnormal termination?
	jne	fdc_init_error	; not an abnormal termination

; fall through to fdc_send_specify

;=========================================================================
; fdc_send_specify - Send specify command to FDC
; Input:
;	none
; Output:
;	CF clear on success
;		AH = 00 - successful completion
;	CF set if error
;		AH = 20h - controller failure
;	AX,CL,SI trashed
; Note:
;	Specify command parameters are obtained from table at INT 1Eh vector
;-------------------------------------------------------------------------
fdc_send_specify:

	push	ds
	xor	ax,ax
	mov	ds,ax
	lds	si,[1Eh*4]		; DS:SI -> INT 1Eh
	mov	al,3		 	; FDC Specify command
	mov	ah,byte [si]		; specify command - byte 0
	mov	si,word [si+1]		; specify command - byte 1
	mov	cl,3			; command length 3 bytes (AX, SI - low)
	pop	ds
	call	fdc_send_cmd		; send specify command to FDC
	jc	fdc_init_error
	mov	ah,fdc_e_success	; successful completion
	ret

fdc_init_error:
	mov	ah,fdc_e_failure
	stc
	ret

;=========================================================================
; int_13_fn02 - Read disk sectors
; int_13_fn03 - Write disk sectors
; int_13_fn04 - Verify disk sectors
; Input:
;	AH = function
;		02h - read
;		03h - write
;		04h - verify
;	AL = number of sectors to read / write / verify (must be nonzero)
;	CH = cylinder number
;	CL = sector number
;	DH = head number (0 or 1)
;	DL = drive number (0 or 1)
;	ES:BX -> data buffer
; Output:
;	CF clear if successful
;		AH = 00h - successful completion
;		AL = number of sectors transferred or verified
;	CF set on error
;		AH = error code
;-------------------------------------------------------------------------
int_13_fn02:
int_13_fn03:
int_13_fn04:
	cmp	dl,1
	ja	.invalid_drive
	mov	si,fdc_media_state
	push	dx
	mov	dh,00h
	add	si,dx			; SI -> drive media state
	pop	dx

	call	read_cmos_type		; get drive type in AL
	jc	.invalid_drive

	mov	di,fdc_motor_state
	and	byte [di],~fdc_write_flag ; read / verify operation
	cmp	ah,03h			; write function
	jne	.motor_on		; jump if not write function
	or	byte [di],fdc_write_flag ; write / format operation

.motor_on:
	call	fdc_motor_on

	call	fdc_disk_change
	jc	.error_end_io		; jump if disk change check failed

	test	byte [si],fdc_m_established
	jnz	.select_rate		; jump if media type is established

	call	fdc_detect_media	; establish media type
	jc	.error_end_io

.select_rate:
	call	fdc_select_rate		; select transfer rate
	
; send FDC specify command
	mov	ax,0BF03h		; FDC Specify command +
					; 0BFh - specify byte 0 for 1.44M
	cmp	byte [si],fdc_m_1440	; 1.44M?
	je	.fdc_send_specify_cmd	; jump if 1.44M
	mov	ah,0DFh			; 0DFh - specify byte 0 for 360/1.2/720
	cmp	byte [si],fdc_m_2880	; 2.88M?
	jne	.fdc_send_specify_cmd	; jump if not 2.88M
	mov	ah,0AFh			; 0AFh - specify byte 0 for 2.88M

.fdc_send_specify_cmd:
	mov	si,2			; specify byte 1
	mov	cl,3			; 3 bytes command
	call	fdc_send_cmd
	jc	.error_end_io		; jump if failed to send command
	push	ds
	xor	si,si
	mov	ds,si
	lds	si,[1Eh*4]		; DS:SI -> INT 1Eh
	mov	cl,byte [bp+int_13_al]	; number of sectors to transfer
	mov	ah,00h
	mov	al,cl			; AX =  number of sectors to transfer
	add	cl,byte [bp+int_13_cl]	; number of the first sector to access
	dec	cl			; minus 1 - last sector to access
	cmp	cl,byte [si+4]		; compare with sectors per track
	jbe	.calc_dma_count		; jump if last sector <= sectors/track
	mov	byte [si+4],cl		; update it in disk parameter table

.calc_dma_count:
	mov	cl,byte [si+3]		; bytes per sector (02h = 512 bytes...)
	add	cl,7			; CL = log2 (bytes per sector)
	shl	ax,cl			; AX = AL * 2 ^ CL = AX * bytes/sector
	pop	ds
	dec	ax			; minus one byte
	mov	cx,ax			; ...count for DMA

	mov	ah,byte [bp+int_13_ah]	; AH = function
	mov	al,46h			; DMA mode byte for read
	cmp	ah,02h			; read function?
	je	.configure_dma		; jump if read function
	mov	al,4Ah			; DMA mode byte for write
	cmp	ah,03h			; write function?
	je	.configure_dma		; jump if write function
	mov	al,42h			; DMA mode byte for verification

.configure_dma:
	call	fdc_configure_dma
	mov	cx,word [bp+int_13_cx]	; restore parameters
	jc	.error_end_io		; jump if DMA boundry crossed

	call 	fdc_seek		; seek drive DL to cylinder CH, head DH
	jc	.error_fdc_get_result	; jump if seek failed

	push	ds
	xor	ax,ax
	mov	ds,ax
	lds	si,[1Eh*4]		; DS:SI -> INT 1Eh
	mov	ah,byte [si+3]		; AH = sector size (02h = 512)
	mov	al,cl			; AL = first sector number
	mov	di,ax			; DI = first sector, sector size
	mov	bl,byte [si+4]		; BL = sectors per track
	mov	ch,byte [si+6]		; CH = special sector size
	pop	ds

	mov	si,fdc_media_state
	mov	dh,00h
	add	si,dx
	mov	dl,byte [si]		; drive media state
	mov	bh,1Bh			; read / write gap for 1.2M/1.44M/2.88M
	cmp	dl,fdc_m_1440		; 1.44M media?
	je	.gap_set		; jump if 1.44M media
	cmp	dl,fdc_m_1200in1200	; 1.2M in 1.2M drive?
	je	.gap_set		; jump if 1.2M media in 1.2M drive
	cmp	dl,fdc_m_2880		; 2.88 media?
	je	.gap_set		; jump if 2.88M media
	mov	bh,23h			; read / write gap for 360K in 1.2M drv
	cmp	dl,fdc_m_360in1200	; 360K in 1.2M drive
	je	.gap_set		; jump if 360K media in 1.2M drive
	mov	bh,2Ah			; read / write gap for 360K and 720K drv
	
.gap_set:
	mov	dx,word [bp+int_13_dx]	; restore dx
	mov	al,byte [bp+int_13_ch]	; cylinder
	mov	ah,dh			; head
	mov	si,ax			; head / cylinder (bytes 2-3)
	shl	ah,1
	shl	ah,1
	or	ah,dl			; FDC command byte 1
	mov	al,0E6h			; FDC Read command
	cmp	byte [bp+int_13_ah],3	; write function?
	jne	.send_command		; jump if not write 
	mov	al,0C5h			; FDC Write command

.send_command:
	mov	cl,9			; 9 bytes command
	and	byte [fdc_calib_state],~fdc_irq_flag ; wait for IRQ6
	call	fdc_send_cmd
	jc	.error_fdc_get_result	; jump if failed to send command
	call	fdc_wait_irq
	jc	.error_fdc_get_result
	mov	cl,7
	call	fdc_get_result		; read result bytes
	jc	.error_end_io

	call	fdc_get_error

.exit_end_io:
	mov	ch,byte [bp+int_13_ch]	; CH = cylinder
;	mov	dh,byte [bp+int_13_dl]	; DH = head
	call	fdc_end_io		; return number of last sector in BL
	or	al,al
	jz	.zero_sectors		; jump if zero sectors were transferred
	mov	al,bl			; AL = last transferred sector
	sub	al,byte [bp+int_13_cl]	; minus the first sector

.zero_sectors:
	or	ah,ah
	jz	.exit			; jump if no errors

.error:
	stc				; indicate error

.exit:
	mov	byte [fdc_last_error],ah
	mov	byte [bp+int_13_al],al	; number of transferred sectors
	jmp	int_13_upd_exit

.error_fdc_get_result:
	mov	cl,7
	push	ax
	call	fdc_get_result		; read result bytes
	pop	ax

.error_end_io:
	mov	al,00h			; failure, no sectors transferred
	jmp	.exit_end_io

.invalid_drive:
	mov	ax,fdc_e_invalid << 8	; AH = 01h, AL = 00h (0 sectors trans)
	jmp	.error


;=========================================================================
; int_13_fn05 - Format track
; Input:
;	AH = 05h
;	CH = track number
;	DH = head number (0 or 1)
;	DL = drive number (0 or 1)
;	ES:BX -> address field buffer
; Output:
;	CF clear if successful
;		AH = 00h - successful completion
;	CF set on error
;		AH = error code 
;-------------------------------------------------------------------------
int_13_fn05:
	cmp	dl,1
	ja	.invalid_drive		; jump if invalid drive number
	call	read_cmos_type		; get drive type in AL
	jc	.invalid_drive
	mov	si,fdc_media_state
	push	dx
	mov	dh,00h
	add	si,dx			; SI -> drive media state
	pop	dx
	cmp	byte [si],0
	jz	.timeout		; jump if media type is not established
	or	byte [fdc_motor_state],fdc_write_flag ; write operation
	call	fdc_motor_on
	call	fdc_disk_change		; read disk change line
	jc	.error_end_io		; jump if disk change check failed
	call	fdc_select_rate		; select transfer rate
	call	fdc_send_specify	; send FDC Specify command
	push	ds
	xor	si,si
	mov	ds,si
	lds	si,[1Eh*4]		; DS:SI -> INT 1Eh
	mov	cl,[si+4]		; sectors per track
	mov	di,word [si+7]		; format gap, sectors per track
	mov	si,word [si+3]		; bytes per sector, sectors per track
	pop	ds
	mov	ch,00h			; CX = sectors per track
	shl	cx,1			; CX = CX * 4 (address field is 4 bytes)
	shl	cx,1
	dec	cx	 		; minus one byte...
	mov	al,4Ah			; DMA mode byte for write
	call	fdc_configure_dma
	jc	.error_end_io		; DMA boundry crossed
	mov	ch,byte [bp+int_13_ch]	; cylinder
	push	si
	call 	fdc_seek		; seek drive DL to cylinder CH, head DH
	pop	si
	jc	.error_fdc_get_result	; jump if seek failed

	mov	ah,dh
	shl	ah,1
	shl	ah,1
	or	ah,dl
	mov	al,4Dh			; FDC Format command

	mov	cl,6			; 6 byte command
	and	byte [fdc_calib_state],~fdc_irq_flag ; wait for IRQ6
	call	fdc_send_cmd
	jc	.exit_end_io		; jump if failed to send command
	call	fdc_wait_irq
	jc	.exit_end_io
	mov	cl,7
	call	fdc_get_result		; read result bytes
	jc	.exit_end_io
	call	fdc_get_error		; get error code

.exit_end_io:
	call	fdc_end_io
	or	ah,ah
	jz	.exit			; jump if no errors

.error:
	stc				; indicate error

.exit:
	mov	byte [fdc_last_error],ah
	jmp	int_13_upd_exit

.error_fdc_get_result:
	mov	cl,7
	push	ax
	call	fdc_get_result		; read result bytes
	pop	ax
	jmp	.exit_end_io

.error_end_io:
	call	fdc_end_io
	jmp	.error

.timeout:
	mov	ah,fdc_e_timeout
	jmp	.error

.invalid_drive:
	mov	ah,fdc_e_invalid
	jmp	.error

;=========================================================================
; int_0E - IRQ6 (FDC) service routine
; Sets bit 7 in drive recalibration status byte (fdc_calib_state)
;-------------------------------------------------------------------------
	setloc	0EF57h

int_0E:
	push	ax
	push	ds
	mov	ax,biosdseg
	mov	ds,ax
	or	byte [fdc_calib_state],fdc_irq_flag ; set IRQ flag
	pop	ds
	mov	al,20h			; send EOI to PIC
	out	pic1_reg0,al
	mov	ax,9101h		; call "interrupt completed" OS hook
	int	15h
	pop	ax
	iret

;=========================================================================
; Disk parameter tables in INT 1Eh format
; Byte 0: First byte for specify command:
;	bits 7-4: step rate ([32 ms - value * 2], e.g. 0D0h is 32-13*2 = 6 ms)
;	bits 3-0: head unload time (0Fh = 240 ms)
; Byte 1: Second byte for specify command:
;	bits 7-1: head load time (1 = 4 ms)
;	bit 0: non-DMA mode (always 0)
; Byte 2: motor off timeout in clock ticks (25h - approximately 2 seconds)
; Byte 3: sector size (00h - 128, 01h - 256, 02h - 512, 03h - 1024)
; Byte 4: sectors per track
; Byte 5: gap length for read/write (2Ah for 5.25", 1Bh for 3.5")
; Byte 6: special sector size (0FFh - not used)
; Byte 7: gap length for formating (50h for 5.25", 6Ch for 3.5")
; Byte 8: format filler byte (default 0F6h)
; Byte 9: head settle time in milliseconds
; Byte A: motor start time, in 1/8 seconds
; Byte B: number of cylinders minus one
; Byte C: transfer rate (in bits 7 - 6)
;-------------------------------------------------------------------------
media_360_in_360:
	db	0DFh, 02h, 25h, 02h, 09h, 2Ah, 0FFh, 50h, 0F6h, 0Fh, 08h
	db	27h, 80h
media_1200:
	db	0DFh, 02h, 25h, 02h, 0Fh, 1Bh, 0FFh, 54h, 0F6h, 0Fh, 08h
	db	4Fh, 00h
media_720:
	db	0DFh, 02h, 25h, 02h, 09h, 2Ah, 0FFh, 50h, 0F6h, 0Fh, 08h
	db	4Fh, 80h
media_1440:
	db	0BFh, 02h, 25h, 02h, 12h, 1Bh, 0FFh, 6Ch, 0F6h, 0Fh, 08h
	db	4Fh, 00h
media_360_in_1200:
	db	0DFh, 02h, 25h, 02h, 09h, 23h, 0FFh, 50h, 0F6h, 0Fh, 08h
	db	27h, 40h
media_2880:
	db	0AFh, 02h, 25h, 02h, 24h, 1Bh, 0FFh, 50h, 0F6h, 0Fh, 08h
	db	4Fh, 0C0h

;=========================================================================
; Disk parameter table for IBM compatibility
; Using 2.88M disk as it has maximal number of cylinders and sectors
;-------------------------------------------------------------------------
	setloc	0EFC7h
int_1E:
	db	0AFh, 02h, 25h, 02h, 24h, 1Bh, 0FFh, 50h, 0F6h, 0Fh, 08h
