;=========================================================================
; floppy1.inc - BIOS floppy disk services (part 1 of 2)
;       INT 13h, function AH=01h
;       INT 13h, function AH=08h
;       INT 13h, function AH=15h
;       INT 13h, function AH=16h
;       INT 13h, function AH=17h
;       INT 13h, function AH=18h
;       INT 13h, function AH=08h
;	- see floppy2.inc for other INT 13h functions
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
; FDC registers

fdc_dor_reg	equ	3F2h		; FDC digital output register (W)
fdc_status_reg	equ	3F4h		; FDC main status register (R)
fdc_stat_ready	equ	80h		; 1 = FDC is ready to send / receive
fdc_stat_dir	equ	40h		; 0 = CPU -> FDC; 1 = FDC -> CPU
fdc_stat_dma	equ	20h		; 1 = DMA and IRQ enabled
fdc_stat_busy	equ	10h		; 1 = FDC is busy
fdc_data_reg	equ	3F5h		; FDC data register (R/W)
fdc_dir_reg	equ	3F7h		; FDC digital input register (W)
fdc_ccr_reg	equ	3F7h		; FDC control configuration register (R)

;-------------------------------------------------------------------------
; DMAC registers

dmac_ch2_addr_reg	equ	04h	; DMAC channel 2 base addres (W)
dmac_ch2_count_reg	equ	05h	; DMAC channel 2 word count (W)
dmac_mask_reg		equ	0Ah	; DMAC single mask bit register (W)
dmac_mode_reg		equ	0Bh	; DMAC mode register (R/W)
dmac_ff_reg		equ	0Ch	; DMAC set (R) or clear (W)
					; first / last flip-flop
dmapage_ch2_reg		equ	81h	; DMA page channel 2 register

;-------------------------------------------------------------------------
; Floppy disk services - error codes

fdc_e_success	equ	00h		; successful completion
fdc_e_invalid	equ	01h		; invalid function or parameter
fdc_e_address	equ	02h		; address mark not found
fdc_e_wprotect	equ	03h		; disk write-protected
fdc_e_notfound	equ	04h		; sector not found
fdc_e_changed	equ	06h		; disk changed
fdc_e_dma	equ	08h		; DMA overrun
fdc_e_boundary	equ	09h		; attempted DMA across 64K boundary
fdc_e_format	equ	0Ch		; not supported or drive type unknown
fdc_e_crc	equ	10h		; uncorrectable CRC error on read
fdc_e_failure	equ	20h		; controller failure
fdc_e_seek	equ	40h		; seek failed
fdc_e_timeout	equ	80h		; timeout / device not ready

;-------------------------------------------------------------------------
; Drive media state for fdc_media_state

fdc_m_rate_bits		equ	0C0h	; bits 7-6: data transfer rate
					; 00b = 500 Kbit/sec
					; 01b = 300 Kbit/sec
					; 10b = 250 Kbit/sec
					; 11b = 1 Mbit/sec
fdc_m_double_step	equ	20h	; bit 5:    1 = double stepping required
fdc_m_established	equ	10h	; bit 4:    1 = media/drive established
fdc_m_state_bits	equ	07h	; bits 2-0: media state bits
					; 000b = trying 360K in 360K drive
					; 001b = trying 360K in 1.2M drive
					; 010b = trying 1.2M in 1.2M drive
					; 011b = known 360K in 360K drive
					; 100b = known 360K in 1.2M drive
					; 101b = known 1.2M in 1.2M drive
					; 110b = not used
					; 111b = 720K in 720K or 1.44M in 1.44M
fdc_m_360in360		equ	93h	; 250 Kbps, established, 360K in 360K
fdc_m_720		equ	97h	; 250 Kbps, established, other drive
fdc_m_try_360in1200	equ	61h	; 300 Kbps, not established, 360K in 1.2
fdc_m_360in1200		equ	74h	; 300 Kbps, established, 360K in 1.2M
fdc_m_try_1200in1200	equ	02h	; 500 Kbps, not established, 1.2M in 1.2
fdc_m_1200in1200	equ	15h	; 500 Kbps, established, 1.2M in 1.2M
fdc_m_try_1440		equ	07h	; 500 Kbps, not established, other drive
fdc_m_1440		equ	17h	; 500 Kbps, established, other drive
fdc_m_try_2880		equ	0C7h	; 1 Mbps, not established, other drive
fdc_m_2880		equ	0D7h	; 1 Mbps, established, other drive

;-------------------------------------------------------------------------
; IRQ flag - fdc_calib_state, bit 7. Set by INT 0Eh (IRQ6) interrupt handler
;	     when IRQ6 happens to indicate completion of an I/O operation

fdc_irq_flag	equ	80h		; IRQ6 had occurred

; write flag - fdc_motor_state, bit 7. Set for write or format,
;	       unset for read or verify

fdc_write_flag	equ	80h		; write or format operation

;-------------------------------------------------------------------------
; floppy drive type definitions for floppy type byte - cmos_floppy

cmos_no_floppy	equ	00h
cmos_360	equ	01h
cmos_1200	equ	02h
cmos_720	equ	03h
cmos_1440	equ	04h
cmos_2880	equ	06h

;=========================================================================
; int_13_fn01 - Get status of last operation
; Input:
;	AH = 01h
;	DL = drive number (bit 7 not set - floppy drive)
; Output:
;	CF clear if last operation was successful
;		AH = 00h - successful completion
;	CF set on error
;		AH - error code of the last operation
; Note:
;	This function doesn't check drive number, but HDD BIOS (if installed)
;	normally will call this functions if bit 7 of DL is not set
;-------------------------------------------------------------------------
int_13_fn01:
	mov	ah,byte [fdc_last_error]
	or	ah,ah
	jz	.no_error
	stc
.no_error:
	jmp	int_13_exit

;=========================================================================
; int_13_fn08: Get drive parameters
; Input:
;	AH = 08h
;	DL = drive number (0 - 7Fh)
; Output:
;	CF clear if successful
;		AX = 0000h
;		BH = 00h
;		BL = CMOS drive type
;		CH = maximal cylinder number - 1
;		CL = maximal sector number
;		DH = maximal head number
;		DL = number of drives
;		ES:DI -> diskette parameter table
;	CF set on error
;		AH = 01h - drive number is greater than 7Fh
; Notes:
;	- If non-existent drive number is specified, function returns zeros in
;	AX,BX,CX,DH,ES,DI and number of floppy drives in DL (0 if no floppies)
;	- If media type is not established function sets media parameters 
;	based on CMOS drive type
;-------------------------------------------------------------------------
int_13_fn08:
	cmp	dl,80h			; valid floppy drive number?
	ja	.invalid_drive		; hard drive number specified
	mov	al,byte [equipment_list]
	and	al,equip_floppies|equip_floppy2
	mov	ah,2			; assume two floppy drives
	cmp	al,equip_floppies|equip_floppy2 ; two floppy drives?
	je	.check_drive_number
	dec	ah			; assume one floppy drive
	cmp	al,equip_floppies	; one floppy drive?
	jne	.no_drives

.check_drive_number:
	mov	byte [bp+int_13_dl],ah	; pass number of drives to caller
	cmp	dl,ah			; requested drive number is larger
					; than number of disks?
	jnb	.non_existent_drive	; jump if non existent drive requested
	call	read_cmos_type		; returns drive type in AL
	jc	.non_existent_drive	; return no drive if CMOS error

	mov	byte [bp+int_13_dh],1	; maximal head number is 1 for floppy
	mov	byte [bp+int_13_bl],al	; pass CMOS data type to caller
	mov	cx,cs
	mov	es,cx			; diskette parameter table segment

	cmp	al,cmos_360
	je	.set_360
	cmp	al,cmos_720
	je	.set_720
	cmp	al,cmos_1200
	je	.set_1200
	cmp	al,cmos_1440
	je	.set_1440

.set_2880:
	mov	al,fdc_m_try_2880	; try 2.88M in 2.88M drive
	lea	di,[media_2880]		; only 2.88M uses 1 Mbps rate
	mov	cx,4F24h		; 2.88M - 80 cylinders, 36 sectors
	jmp	.set_media_type

.set_360:
	mov	al,fdc_m_360in360
	lea	di,[media_360_in_360]
	mov	cx,2709h		; 360K - 40 cylinders, 9 sectors
	jmp	.set_media_type

.set_720:
	mov	al,fdc_m_720
	lea	di,[media_720]
	mov	cx,4F09h		; 720K - 80 cylinders, 9 sectors
	jmp	.set_media_type

.set_1200:
	mov	al,fdc_m_try_1200in1200
	lea	di,[media_1200]
	mov	cx,4F0Fh		; 1.2M - 80 cylinders, 15 sectors
	jmp	.set_media_type

.set_1440:
	mov	al,fdc_m_try_1440
	lea	di,[media_1440]
	mov	cx,4F12h		; 1.44M - 80 cylinders, 18 sectors

.set_media_type:
	mov	bx,fdc_media_state
	add	bl,dl			; BX -> drive media state
	test	byte [bx],fdc_m_established ; media type established?
	jnz	.set_parameters		; no need to update if established
	mov	[bx],al

.set_parameters:
	xor	ax,ax			; AH = 00h - successful completion
	mov	byte [bp+int_13_al],al	; successful completion
	mov	byte [fdc_last_error],al
	mov	byte [bp+int_13_bh],al	; clear BH just in case
	mov	word [bp+int_13_cx],cx	; cylinders / sectors
	mov	word [bp+int_13_di],di	; diskette parameter table pointer
	jmp	int_13_exit

.no_drives:
	mov	byte [bp+int_13_dl],0	; zero drives

.non_existent_drive:
	xor	cx,cx
	xor	di,di
	mov	byte [bp+int_13_bl],cl	; CMOS drive type is zero
	mov	byte [bp+int_13_dh],cl	; maximal head number is zero
	mov	es,cx			; disk parameter table segment = 0000h
	jmp	.set_parameters

.invalid_drive:
	mov	ah,fdc_e_invalid
	stc
	jmp	int_13_exit

;=========================================================================
; int_13_fn15: Get disk type
; Input:
;	AH = 15h
;	DL = drive number (0 or 1)
; Output:
;	AH = type code:
;		00h - no such drive (invalid drive specified)
;		01h - floppy without change-line support
;		02h - floppy with change-line support
;-------------------------------------------------------------------------
int_13_fn15:
	mov	ah,00h			; assume no drive
	cmp	dl,1
	ja	.exit			; jump if invalid drive number
	call	read_cmos_type		; returns drive type in AL
	jc	.exit			; jump if CMOS disk type is invalid
	mov	ah,01h			; assume no change-line support
	cmp	al,cmos_360
	je	.exit
	cmp	al,cmos_720		; no change-line support on 720K drives?
	je	.exit
	mov	ah,02h			; otherwise it supports change-line

.exit:
	clc
	mov	byte [fdc_last_error],0
	jmp	int_13_exit

;=========================================================================
; int_13_fn16 - Detect disk change
; Input:
;	AH = 16h
;	DL = drive number (0 or 1)
; Output:
;	CF clear if change line inactive
;		AH = 00h - disk not changed
;	CF set if change line active or error
;		AH = 01h - invalid drive number
;		AH = 06h - disk changed or change line not supported
;		AH = 80h - timeout / device not ready
;-------------------------------------------------------------------------
int_13_fn16:
	cmp	dl,1
	ja	.invalid_drive
	call	read_cmos_type		; returns drive type in AL
	jc	.invalid_drive

	cmp	al,cmos_360
	je	.no_change_line		; report disk changed for 360K drives
	cmp	al,cmos_720
	je	.no_change_line		; report disk changed for 720K drives

	call	fdc_motor_on		; turn motor on
	xor	ah,ah			; assume disk not changed
	mov	dx,fdc_dir_reg
	in	al,dx			; read disk change line
	shl	al,1			; bit 7 to CF
	jnc	.exit
	mov	ah,fdc_e_changed
	stc

.exit:
	mov	[fdc_last_error],ah	; save the error code
	pushf
	push	ds
	xor	si,si
	mov	ds,si
	lds	si,[1Eh*4]		; DS:SI -> INT 1Eh
	mov	cl,byte [si+2]
	pop	ds
	mov	byte [fdc_motor_tout],cl ; ticks before turning off the motor
	popf
	jmp	int_13_exit


.no_change_line:
	mov	ah,fdc_e_changed	; can't detect - report disk changed
	stc
	mov	byte [fdc_last_error],ah
	jmp	int_13_exit

.invalid_drive:
	mov	ah,fdc_e_invalid
	stc
	jmp	int_13_exit

;=========================================================================
; int_13_fn17 - Set disk type for format
; Input:
;	AH = 17h
;	AL = format type
;		01h - 320K / 360K disk in 360K drive
;		02h - 320K / 360K disk in 1.2M drive
;		03h - 1.2M disk in 1.2M drive
;		04h - 720K disk in 720K or 1.2M drive
;	DL = drive number (0 or 1)
; Output:
;	CF clear if successful
;		AH = 00h - successful completion
;	CF set on error
;		AH = 01h - invalid drive number
;		AH = 06h - disk changed
;		AH = 80h - timeout / device not ready
;-------------------------------------------------------------------------
int_13_fn17:
	cmp	dl,1
	ja	.invalid_parameters
	call	read_cmos_type		; get drive type in AL
	jc	.invalid_parameters
	mov	cl,[bp+int_13_al]	; get original AL value to CL
	cmp	cl,0			; validate parameters
	je	.invalid_parameters
	cmp	cl,4
	ja	.invalid_parameters
	mov	bx,fdc_media_state
	add	bl,dl			; BX -> drive media state
	cmp	cl,1			; 360K disk in 360K drive?
	jne	.not_360in360
	mov	byte [bx],fdc_m_360in360
	mov	ah,0			; no error
	jmp	.exit_check_error

.not_360in360:
	call	fdc_motor_on
	mov	si,bx
	call	fdc_disk_change		; check if disk (SI) changed
	cmp	ah,fdc_e_changed
	jbe	.set_type		; no errors other than "disk changed"
	cmp	ah,fdc_e_timeout
	jne	.set_type		; floppy disk is installed
	cmp	byte [bx],fdc_m_720	; 250 Kbps and not 5.25?
	je	.exit_check_error
	mov	byte [bx],fdc_m_try_360in1200 ; 300 Kpbs, try 360 in 1.2M
	jmp	.exit_check_error

.set_type:
	cmp	cl,4			; 720K in 720K?
	jne	.check_360in1200	; jump if not 720K in 720K
	mov	byte [bx],fdc_m_720
	jmp	.exit_check_error

.check_360in1200:
	cmp	cl,2			; 360K in 1.2M?
	jne	.set_1200in1200		; jump if not 360K in 1.2M
	mov	byte [bx],fdc_m_360in1200
	jmp	.exit_check_error

.set_1200in1200:
	mov	byte [bx],fdc_m_1200in1200 ; 1.2M in 1.2M

.exit_check_error:
	mov	byte [fdc_last_error],ah
	or	ah,ah
	jz	.exit			; jump if no error
	stc				; indicate error

.exit:
	jmp	int_13_upd_exit

.invalid_parameters:
	mov	ah,fdc_e_invalid
	jmp	.exit

;=========================================================================
; int_13_fn18 - Set media type for format
; Input:
;	AH = 18h
;	DL = drive number (0 or 1)
;	CH = number of cylinders - 1
;	CL = sectors per track
; Output:
;	CF = clear if successful
;		AH = 00h - requested format is supported
;		ES:DI -> diskette parameter table
;	CF = set on error
;		AH = 01h - invalid drive number specified
;		AH = 0Ch - format is not supported or drive type is unknown
;-------------------------------------------------------------------------
int_13_fn18:
	cmp	dl,1
	ja	.invalid_drive
	call	read_cmos_type		; get drive type in AL
	jc	.unsupported_format	; jump if CMOS drive type invalid

	mov	bx,fdc_media_state
	add	bl,dl			; BX -> drive media state

	cmp	al,cmos_360		; 360K drive?
	jne	.try_drive_1200
	cmp	cx,2709h	 	; 40 tracks 9 sectors?
	jnz	.unsupported_format
	mov	al,fdc_m_360in360	; 360K in 360K established, 250 Kbps
	lea	di,[media_360_in_360]
	jmp	.set_media

.try_drive_1200:
	cmp	al,cmos_1200		; 1.2M drive?
	jne	.try_drive_2880
	cmp	cx,4F0Fh		; 80 tracks 15 sectors?
	jne	.try_media_360_in_1200
	mov	al,fdc_m_1200in1200	; 1.2M in 1.2M established, 500Kbps
	lea	di,[media_1200]		; 1.2M
	jmp	.set_media

.try_media_360_in_1200:
	cmp	cx,2709h		; 80 tracks 9 sectors?
	jne	.unsupported_format
	mov	al,fdc_m_360in1200	; 360K in 1.2M established, 300Kbps
	lea	di,[media_360_in_1200]	; 360K in 1.2M
	jmp	.set_media

.try_drive_2880:
	cmp	al,cmos_2880		; 2.88M drive?
	jne	.try_drive_1440
	cmp	cx,4F24h		; 80 tracks 36 sectors?
	jne	.try_media_1440
	mov	al,fdc_m_2880		; indicate 2.88M
	lea	di,[media_2880]
	jmp	.set_media

.try_drive_1440:
	cmp	al,cmos_1440		; 1.44M drive?
	jne	.try_drive_720

.try_media_1440:
	cmp	cx,4F12h		; 80 tracks 18 sectors?
	jne	.try_media_720
	mov	al,fdc_m_1440		; indicate 1.44M
	lea	di,[media_1440]
	jmp	.set_media

.try_drive_720:
	cmp	al,cmos_720		; 720K drive?
	jne	.unsupported_format	; should never happen...

.try_media_720:	
	cmp	cx,4F09h		; 80 tracks 9 sectors?
	jne	.unsupported_format
	mov	al,fdc_m_720		; other established, 250Kbps
	lea	di,[media_720]		; 720K

.set_media:
	mov	byte [bx],al		; set physical media
	call	fdc_set_rate		; transfer rate in AL
	ror	al,1			; move rate from bits 1,0 to 7,6
	ror	al,1
	and	byte [fdc_last_rate],3Fh ; clear rate bits
	or	byte [fdc_last_rate],al	; store new rate
	mov	word [bp+int_13_di],di	; return parameters table - offset
	mov	cx,cs
	mov	es,cx			; return parameters table - segment
	mov	byte [fdc_last_error],0	; no errors
	xor	ah,ah
	jmp	int_13_upd_exit

.invalid_drive:
	mov	ah,fdc_e_invalid	; invalid function or parameter
	jmp	.error

.unsupported_format:
	mov	ah,fdc_e_format		; not supported or drive type unknown

.error:
	stc
	jmp	int_13_upd_exit

;=========================================================================
; fdc_recalibrate - Recalibrate disk drive, seek to cylinder 0
; Input:
;	DL = drive number (0 or 1)
; Output:
;	CF clear if successful
;		AH = 00h - successful completion
;	CF set on error
;		AH = 20h - controller failure
;		AH = 20h - timeout
;	AH trashed
;-------------------------------------------------------------------------
fdc_recalibrate:
	push	si
	push	cx
	push	dx
	mov	al,07h			; FDC Recalibrate command
	mov	ah,dl			; drive number
	mov	cl,2			; 2 bytes command
	and	byte [fdc_calib_state],~fdc_irq_flag ; clear IRQ6 flag
	call	fdc_send_cmd
	jc	recal_end		; failure
	call	fdc_wait_irq		; wait for IRQ6
	jc	recal_end		; timeout waiting for interrupt
	mov	al,08h			; FDC Sense Interrupt Status command
	mov	cl,1			; 1 byte command
	call	fdc_send_cmd
	jc	recal_end		; failure
	mov	cx,2			; 2 bytes result
	call	fdc_get_result		; store result
	jc	recal_end		; failure
	mov	bx,fdc_ctrl_status
	mov	ah,fdc_e_seek
	mov	dl,[bx]			; ST0
	and	dl,60h
	cmp	dl,60h			; abnormal termination + seek end
	stc				; indicate error
	je	recal_end		; failure
	pop	dx
	push	dx
	xor	dh,dh				 
	mov	bx,fdc_cylinder
	add	bx,dx
	mov	byte [bx],0		; current cylinder = 0
	mov	cl,dl
	mov	dl,1
	shl	dl,cl
	or	byte [fdc_calib_state],dl ; drive recalibrated
	mov	cx,43h
	call	delay_15us		; 1 ms delay
	xor	ah,ah

recal_end:
	mov	byte [fdc_last_error],ah
	pop	dx
	pop	cx
	pop	si
	ret

;=========================================================================
; fdc_seek - Move floppy drive head to the specified cylinder
; Input:
;	DL = drive number
;	DH = head number
;	CH = cylinder
; Output:
;	CF clear if successful
;		AH = 00h - successful completion
;	CF set on error
;		AH = 20h - controller failure
;		AH = 40h - seek failed
;		AH = 80h - timeout / device not ready
;-------------------------------------------------------------------------
fdc_seek:
	push	bx
	push	cx
	mov	ah,byte [fdc_calib_state]
	mov	cl,dl
	inc	cl
	shr	ah,cl
	jc	.skip_recalibrate	; jump if drive is already calibrated
	call	fdc_recalibrate
	jnc	.skip_recalibrate	; jump if calibration successful
	call	fdc_recalibrate		; try recalibrating again
	jc	.exit			; no luck...

.skip_recalibrate:
	mov	bh,00h
	mov	bl,dl			; BX = drive number
	test	byte [fdc_media_state+bx],fdc_m_double_step ; check double
					; steppig bit
	jz	.no_double_stepping
	shl	ch,1			; CH = CH * 2 (double cylinder number)

.no_double_stepping:
	cmp	byte [fdc_cylinder+bx],ch	; already at right cylinder?
	jne	.do_seek		; jump if seek is required
	cmp	byte [fdc_last_error],fdc_e_seek
	je	.do_seek		; jump if it was a seek error
	xor	ah,ah			; AH = 0, CF = 0 - success
	jmp	.exit

.do_seek:
	mov	al,ch			; cylinder
	mov	si,ax			; SI - low = cylinder number
	mov	al,0Fh			; FDC Seek command
	mov	ah,dh			; head
	shl	ah,1
	shl	ah,1
	or	ah,dl			; seek - byte 1 (head / drive)
	mov	cl,3			; 3 bytes command
	and	byte [fdc_calib_state],~fdc_irq_flag ; clear IRQ6 flag
	call	fdc_send_cmd
	jc	.set_result		; seek error
	call	fdc_wait_irq		; wait for IRQ6
	jc	.set_result		; timeout waiting for interrupt

	mov	al,08h			; FDC Sense Interrupt Status command
	mov	cl,1			; 1 byte command
	call	fdc_send_cmd
	jc	.set_result		; failure
	mov	cl,2
	push	bx
	call	fdc_get_result		; read result bytes
	pop	bx
	jc	.set_result		; error
	mov	ah,fdc_e_seek
	mov	al,byte [fdc_ctrl_status] ; ST0
	and	al,60h
	cmp	al,60h			; abnormal termination + seek end
	stc
	je	.set_result		; seek error
	mov	byte [fdc_cylinder+bx],ch	; save new cylinder number

	push	ds
	xor	si,si
	mov	ds,si
	lds	si,[1Eh*4]		; DS:SI -> INT 1Eh
	mov	al,byte [si+9]		; AL = head settle time in ms
	pop	ds

	or	al,al			; head settle time is zero?!
	jz	.get_settle_time

	mov	cl,67			; 15 us * 67 ~= 1 ms
	mul	cl			; AX = delay in 15 us intervals
	mov	cx,ax

.wait:
	call	delay_15us

.wait_end:
	xor	ah,ah

.set_result:
	mov	byte [fdc_last_error],ah

.exit:
	pop	cx
	pop	bx
	ret

.get_settle_time:
	test	byte [fdc_motor_state],fdc_write_flag ; check operation type
	jz	.wait_end		; jump if read / verify - no wait
	
	mov	ah,byte [fdc_media_state+bx]	; AH = media state
	and	ah,fdc_m_state_bits	; leave only drive media state bits
	mov	cx,1325			; 20 ms delay for 360K drives
	jz	.wait			; jump if 360K, media not established
	cmp	ah,fdc_m_360in360 & fdc_m_state_bits
	je	.wait			; jump if 360K, media established
	mov	cx,995			; 15 ms delay for other drives
	jmp	.wait

;=========================================================================
; fdc_motor_on - Turn motor on (if it is not on yet)
; Input:
;	DL = drive number (0 or 1)
; Output:
;	none
;-------------------------------------------------------------------------
fdc_motor_on:
	push	ax
	push	cx
	push	dx
	cli				; entering critical section
	mov	byte [fdc_motor_tout],0FFh  ; set timeout to maximum
	and	byte [fdc_motor_state],0CFh ; zero drive select bits (5-4)
	mov	cl,dl			; CL = drive number
	shl	dl,1
	shl	dl,1
	shl	dl,1
	shl	dl,1
	or	byte [fdc_motor_state],dl   ; select new drive
	inc	cl
	mov	dl,byte [fdc_motor_state]
	shr	dl,cl
	jc	.already_on
	mov	dl,1
	dec	cl
	shl	dl,cl
	or	byte [fdc_motor_state],dl   ; indicate that motor is on
	sti				; end of critical section
    	mov	al,byte [fdc_motor_state]
	ror	al,1
	ror	al,1
	ror	al,1
	ror	al,1
	or	al,0Ch			; DMA and IRQ enabled, no reset
	mov	dx,fdc_dor_reg
	out	dx,al			; send it to FDC - start motor
	mov	ax,90FDh
	int	15h			; call OS hook
	jc	.exit
	push	ds
	push	si
	xor	si,si
	mov	ds,si
	lds	si,[1Eh*4]		; DS:SI -> INT 1Eh
	mov	al,byte [si+0Ah]	; AL = motor start time in 1/8 second
	pop	si
	pop	ds

	test	byte [fdc_motor_state],fdc_write_flag
	jz	.read_verify		; jump if not write operation
	cmp	al,8
	jae	.wait_loop		; jump if at least 1 second start time
	mov	al,8			; wait at least 1 second for write
	jmp	.wait_loop

.read_verify:
	cmp	al,5
	jae	.wait_loop		; jump if at least 625 ms start time
	mov	al,5			; wait at least 625 ms for read / verify

.wait_loop:
	mov	cx,8287			; 8287 * 15.09us ~= 125ms
	call	delay_15us		; wait 125 ms
	dec	al
	jnz	.wait_loop		; repeat until AL = 0

.exit:
	pop	cx
	pop	dx
	pop	ax
	ret

.already_on:
	sti
	mov	al,byte [fdc_motor_state]	; start motor
	ror	al,1
	ror	al,1
	ror	al,1
	ror	al,1
	or	al,0Ch			; DMA and IRQ enabled, no reset
	mov	dx,fdc_dor_reg
	out	dx,al			; send it to FDC - start motor
	jmp	.exit

;=========================================================================
; fdc_end_io - Set motor timeout, return next sector to be transferred
; Input:
;	CH = cylinder
;	DH = head
; Output:
;	BL = next sector to be transferred
;-------------------------------------------------------------------------	

fdc_end_io:
	push	ax
	push	ds
	xor	bx,bx
	mov	ds,bx
	lds	bx,[1Eh*4]		; DS:BX -> INT 1Eh
	mov	ah,[bx+2]		; motor timeout (ticks)
	mov	al,[bx+4]		; sectors per track
	inc	al
	pop	ds
	mov	bx,fdc_ctrl_status
	cmp	ch,[bx+3]		; same result cylinder?
	jne	.exit
	cmp	dh,[bx+4]		; same result head?
	jne	.exit
	mov	al,[bx+5]		; result sector number
.exit:
	mov	byte [fdc_motor_tout],ah ; motor timeout
	mov	bl,al			; next sector to be transferred
	pop	ax
	ret

;========================================================================
; fdc_disk_change - Read disk change line, reset it if active
; Input:
;	AL = CMOS drive type
;	DS:SI = drive media type
; Output:
;	CF clear if disk not changed
;		AH = 00h - disk not changed
;	CF set if disk changed or on error
;		AH = 06h - disk changed
;		AH = error code
;		AL - trashed
; Note:
;	Motor needs to be turned on before calling this function
;------------------------------------------------------------------------
fdc_disk_change:
	push	cx
	mov	ah,al			; save AL value in AH
	cmp	al,cmos_360
	je	.not_changed		; jump if 360K drive (no change line)
	cmp	al,cmos_720
	je	.not_changed		; jump if 720K drive (no change line)
	mov	al,[si]			; media type
	and	al,fdc_m_state_bits
	jz	.not_changed		; jump if 360K drive (no change line)
	cmp	al,3
	je	.not_changed		; jump if 360K dirve (no change line)
	mov	dx,fdc_dir_reg
	in	al,dx			; read disk change line
	shl	al,1
	jnc	.not_changed		; no disk change
	and	byte [si],0EFH		; media not detected
	call	fdc_init		; full initialization
	jc	.exit
	mov	dx,word [bp+int_13_dx]	; restore DX
	mov	ch,1
	call	fdc_seek		; seek to cylinder 1
	jc	.exit
	mov	ch,0
	call	fdc_seek		; seek to cylinder 0
	jc	.exit
	mov	ah,fdc_e_changed
	mov	dx,fdc_dir_reg
	in	al,dx			; read disk change line
	shl	al,1
	jnc	.changed_or_error	; jump if disk change line was reset
	mov	ah,fdc_e_timeout	; failed: no floppy

.changed_or_error:
	stc
	jmp	.exit

.not_changed:
	mov	al,ah			; restore AL
	mov	ah,0			; AH = 00h - disk not changed

.exit:
	mov	dx,word [bp+int_13_dx]	; restore dx
	pop	cx
	ret

;=========================================================================
; fdc_configure_dma - Configure DMA controller for FDC operation (channel 2)
; Input:
;	AL = DMA mode byte
;		42h - verify (single mode, addr increment, verify, channel 2)
;		46h - read   (single mode, addr increment, write, channel 2)
;		4Ah - write  (single mode, addr increment, read, channel 2)
;	CX = byte count (minus 1)
;	ES:BX -> buffer address for DMA operation
; Output:
;	CF clear on success
;	CF set if error
;		AH = 08h - DMA overrun
;	AX,BX trashed
; Note:
;	Translates ES:BX to DMA page and base address and configures DMAC
;-------------------------------------------------------------------------
fdc_configure_dma:
	push	dx
	mov	dx,es			; user's buffer segment
	rol	dx,1
	rol	dx,1
	rol	dx,1
	rol	dx,1
	mov	ah,dl			; calculate DMA page number
	and	ah,0Fh			; AL = page number: bits 19 - 16 of ES
	and	dl,0F0h			; DX = DMA offset: bits 15 - 0 of ES
	add	dx,bx			; add user's buffer offset
	adc	ah,0			; increment page number on overflow

	mov	bx,dx			; check if crossing DMA page boundary:
	add	bx,cx			;   add DMA buffer address to byte count
	jc	.dma_boundary		; jump if crossing DMA boundary (64 KiB)

	cli
	out	dmac_ff_reg,al		; clear first/last flip-flop
	jmp	$+2
	jmp	$+2
	out	dmac_mode_reg,al	; send DMA mode byte
	jmp	$+2
	jmp	$+2
	mov	al,cl
	out	dmac_ch2_count_reg,al	; send word count - low byte
	jmp	$+2
	jmp	$+2
	mov	al,ch
	out	dmac_ch2_count_reg,al	; send word count - high byte
	jmp	$+2
	jmp	$+2
	mov	al,dl
	out	dmac_ch2_addr_reg,al	; send base address - low byte
	jmp	$+2
	jmp	$+2
	mov	al,dh
	out	dmac_ch2_addr_reg,al	; send base address - high byte
	jmp	$+2
	jmp	$+2
	mov	al,ah
	out	dmapage_ch2_reg,al	; channel 2 page register
	jmp	$+2
	jmp	$+2
	mov	al,2
	out	dmac_mask_reg,al	; enable DMA channel 2
	sti

.exit:
	pop	dx
	ret

.dma_boundary:				; Note: CF is already set
	mov	ah,fdc_e_boundary
	jmp	.exit

;=========================================================================
; fdc_send_cmd - Send a command to FDC
; Input:
;	AL = 1st byte of the command
;	AH = 2nd byte of the command
;	SI = 3rd and 4th bytes of the command
;	DI = 5th and 6th bytes of the command
;	BL = 7th byte of the command
;	BH = 8th byte of the command
;	CH = 9th byte of the command
;	CL = command length (number of bytes)
; Output:
;	CF clear if successful
;		AH = 00h - successful completion
;	CF set on error
;		AH = 20h - controller failure
;		AH = 80h - timeout / device not ready
;	AX trashed
;-------------------------------------------------------------------------
fdc_send_cmd:
	call	fdc_write		; send AL (1st byte)
	dec	cl
	jbe	.send_cmd_exit		; jump if done (ZF=1) or on error (CF=1)
	mov	al,ah
	call	fdc_write		; send AH (2nd byte)
	dec	cl
	jbe	.send_cmd_exit		; jump if done (ZF=1) or on error (CF=1)
	mov	ax,si
	call	fdc_write		; send SI / low byte (3th byte)
	dec	cl
	jbe	.send_cmd_exit		; jump if done (ZF=1) or on error (CF=1)
	mov	al,ah
	call	fdc_write		; send SI / high byte (4th byte)
	dec	cl
	jbe	.send_cmd_exit		; jump if done (ZF=1) or on error (CF=1)
	mov	ax,di
	call	fdc_write		; send DI / low byte (5th byte)
	dec	cl
	jbe	.send_cmd_exit		; jump if done (ZF=1) or on error (CF=1)
	mov	al,ah
	call	fdc_write		; send DI / high byte (6th byte)
	dec	cl
	jbe	.send_cmd_exit		; jump if done (ZF=1) or on error (CF=1)
	mov	al,bl
	call	fdc_write		; send BL (7rd byte)
	dec	cl
	jbe	.send_cmd_exit		; jump if done (ZF=1) or on error (CF=1)
	mov	al,bh
	call	fdc_write		; send BH (8th byte)
	dec	cl
	jbe	.send_cmd_exit		; jump if done (ZF=1) or on error (CF=1)
	mov	al,ch
	call	fdc_write		; send CH (9th byte)

.send_cmd_exit:
	ret

;=========================================================================
; fdc_write - Send byte to FDC
; Input:
;	AL = byte to send
; Output:
;	CF clear if successful
;	CF set if timeout
;		AH = 80h - timeout / device not ready
;-------------------------------------------------------------------------
fdc_write:
	push	dx
	push	ax
	call	fdc_wait_ready
	jc	.timeout
	and	al,fdc_stat_ready|fdc_stat_dir|fdc_stat_dma
	cmp	al,fdc_stat_ready
	jne	.timeout
	pop	ax
	mov	dx,fdc_data_reg
	out	dx,al			; write byte

.exit:
	pop	dx
	ret

.timeout:
	pop	ax
	mov	ah,fdc_e_timeout	; Note: CF is already set
	jmp	.exit

;=========================================================================
; fdc_get_result - Read FDC result
; Input:
;	CL = number of result bytes
; Output:
;	CF clear if successful
;		AH = 00h - operation successful
;		Result is returned in fdc_ctrl_status BIOS area
;	CF set on error
;		AH = 20h - controller failure
;		AH = 80h - timeout / device not ready
;	AL,BX - trashed
;-------------------------------------------------------------------------
fdc_get_result:
	push	dx
	mov	bx,fdc_ctrl_status

.next_byte:
	call	fdc_wait_ready
	jc	.timeout
	and	al,fdc_stat_ready|fdc_stat_dir|fdc_stat_dma|fdc_stat_busy
	mov	ah,al
	and	ah,~fdc_stat_busy
	cmp	ah,fdc_stat_ready	; controller finished sending result?
	je	.done			; jump if finished
	cmp	al,fdc_stat_ready|fdc_stat_dir|fdc_stat_busy
	jne	.failure		; jump on invalid controller status
	cmp	bx,fdc_ctrl_status+7	; already read 7 bytes?
	jnb	.failure		; controller error if so
	mov	dx,fdc_data_reg
	in	al,dx
	mov	byte [bx],al
	inc	bx
	jmp	.next_byte

.done:
	xor	ah,ah			; operation successful

.exit:
	pop	dx
	ret

.timeout:
	mov	ah,fdc_e_timeout	; Note: CF is already set
	jmp	.exit

.failure:
	mov	ah,fdc_e_failure
	stc
	jmp	.exit

;=========================================================================
; fdc_read - Read byte from FDC
; Input:
;	none
; Output:
;	CF clear if successful
;		AL = byte read from FDC
;		AH - unchanged
;	CF set on error
;		AH = 20h - controller failure
;		AH = 80h - timeout / device not ready
;	CX trashed
;-------------------------------------------------------------------------
fdc_read:
	push	dx
	mov	dx,fdc_status_reg
	call	fdc_wait_ready
	jc	.timeout
	in	al,dx
	test	al,40h			; FDC is ready to send a byte? 
	jz	.failure		; jump if not ready
	jmp	$+2			; I/O delay
	jmp	$+2
	mov	dx,fdc_data_reg
	in	al,dx			; read the byte

.exit:
	pop	dx
	ret

.timeout:
	mov	ah,fdc_e_timeout	; Note: CF is already set
	jmp	.exit

.failure:
	mov	ah,fdc_e_failure
	stc
	jmp	.exit

;=========================================================================
; fdc_get_error - Translate FDC status to BIOS error number
; Input:
;	DS:[fdc_ctrl_status] - FDC status bytes
; Output:
;	AH = error code
;-------------------------------------------------------------------------

fdc_get_error:
	mov	bx,fdc_ctrl_status
	mov	bx,[bx]
	test	bl,0C0h			; BL = ST1
	mov	ah,fdc_e_success
	jz	.exit			; jump if successful completion
	test	bl,40h			; abnormal termination?
	mov	ah,fdc_e_failure
	jz	.exit
	test	bh,1			; address mark not found?
	mov	ah,fdc_e_address
	jnz	.exit
	test	bh,2			; disk write protected?
	mov	ah,fdc_e_wprotect
	jnz	.exit
	test	bh,4			; sector not found?
	mov	ah,fdc_e_notfound
	jnz	.exit
	test	bh,10H			; DMA overrun?
	mov	ah,fdc_e_dma
	jnz	.exit
	test	bh,20H			; CRC error?
	mov	ah,fdc_e_crc
	jnz	.exit
	test	bh,80h			; access after last sector?
	mov	ah,fdc_e_notfound
	jnz	.exit
	mov	ah,fdc_e_failure	; return FDC failure for other errors

.exit:
	ret

;=========================================================================
; fdc_reset - Reset FDC
; Input:
;	none
; Output:
;	Resets FDC flags in BIOS area
;	AX,CX,DX - trashed 
;-------------------------------------------------------------------------

fdc_reset:
	cli
	and	byte [fdc_motor_state],~fdc_write_flag ; read/verify operation
	and	byte [fdc_calib_state],~fdc_irq_flag ; clear IRQ6 flag
	mov	al,byte [fdc_motor_state]
	rol	al,1			; after rol:
	rol	al,1			; 	bits 7-4: motor state
	rol	al,1			; 	bit 3: operation type (R/W)
	rol	al,1			;	bits 1-0: drive select
	and	al,0FBh			; clear reserved bit
	or	al,08h			; DMA and IRQ enabled, reset
	mov	dx,fdc_dor_reg
	out	dx,al			; send it to FDC
	mov	cx,3
	call	delay_15us		; 30-45 us delay
	or	al,0Ch
	out	dx,al			; DMA and IRQ enabled, no reset
	sti
	call	fdc_wait_irq		; wait for IRQ6
	jc	.exit
	and	byte [fdc_calib_state],~fdc_irq_flag ; clear IRQ flag
	xor	ah,ah			; no errors
.exit:
	ret

;=========================================================================
; read_cmos_type - Read drive type from CMOS
; Input:
;	DL = drive number (0 or 1)
; Output:
;	CF clear if successful
;		AL = drive type
;	CF set on error (invalid drive type)
;=========================================================================

read_cmos_type:
%ifdef BIOS_SETUP
	call	get_floppy
%else ; BIOS_SETUP
	mov	al,44h			; FIXME: fake two 1.44MB floppy drives
%endif ; BIOS_SETUP
	or	dl,dl			; drive 0?
	jnz	.drive_1		; jump if drive 1 - type in bits 3-0
	shr	al,1			; shift drive 0 bits 7-4 to 3-0
	shr	al,1
	shr	al,1
	shr	al,1
.drive_1:
	and	al,0Fh			; mask drive bits
	cmp	al,cmos_no_floppy
	je	.error
	cmp	al,5			; invalid value
	je	.error
	cmp	al,cmos_2880
	ja	.error
	clc
	ret

.error:
	stc
	ret

;=========================================================================
; fdc_detect_media - Detect media type, update it in fdc_media_state[drive]
; Input:
;	AL = CMOS drive type
;	DS:SI = drive media type
; Output:
;	CF clear if successful
;		AH = 00h
;	CF set on error
;		AH = 20h - invalid CMOS
;-------------------------------------------------------------------------
fdc_detect_media:
	push	dx
	push	cx
	mov	ah,0

	cmp	al,cmos_720
	je	.set_720
	cmp	al,cmos_1200
	je	.detect_1200
	cmp	al,cmos_1440
	je	.detect_1440
	cmp	al,cmos_2880
	je	.detect_2880
	
	mov	al,fdc_m_360in360	; set 360K disk in 360K drive
	jmp	.exit_set_media

.set_720:
	mov	al,fdc_m_720		; set 720K disk
	jmp	.exit_set_media

.detect_1200:
	mov	al,0			; try 500 Kbps
	call	fdc_read_id
	mov	al,fdc_m_1200in1200
	jnc	.exit_set_media		; jump if successful
	mov	al,40h			; try 300 Kbps
	call	fdc_read_id
	mov	al,fdc_m_360in1200
	jnc	.exit_set_media		; jump if successful
	mov	al,fdc_m_try_1200in1200
	jmp	.exit_set_media

.detect_1440:
	mov	al,0			; try 500 Kbps
	call	fdc_read_id
	mov	al,fdc_m_1440
	jnc	.exit_set_media		; jump if successful
	mov	al,80h			; try 250 Kbps
	call	fdc_read_id
	mov	al,fdc_m_720
	jnc	.exit_set_media		; jump if successful
	mov	al,fdc_m_try_1440
	jmp	.exit_set_media

.detect_2880:
	mov	al,0C0h			; try 1 Mbps
	call	fdc_read_id
	mov	al,fdc_m_2880
	jnc	.exit_set_media		; jump if successful
	mov	al,0			; try 500 Kbps
	call	fdc_read_id
	mov	al,fdc_m_1440
	jnc	.exit_set_media		; jump if successful
	mov	al,80h			; try 250 Kbps
	call	fdc_read_id
	mov	al,fdc_m_720
	jnc	.exit_set_media		; jump_if successful
	mov	al,fdc_m_try_2880

.exit_set_media:
	mov	byte [si],al		; set media type

.exit:
	mov	byte [fdc_last_error],ah
	pop	cx
	pop	dx
	ret

.invalid_cmos:
	mov	ah,fdc_e_failure
	stc
	jmp	.exit

;=========================================================================
; fdc_read_id - Read ID
; Input:
;	AL = data transfer rate (bits 7-6)
; Output:
;	CF clear if successful
;		AH = 0 - successful completion
;		AL = ID (bits 7-6)
;	CF set on error
;		AH = error code
;-------------------------------------------------------------------------
fdc_read_id:
	push	bx
	mov	byte [fdc_motor_tout],0FFh ; set timeout to maximum
	call	fdc_set_rate		; transfer rate in AL
	mov	dl,byte [bp+int_13_dl]
	call	fdc_recalibrate		; recalibrate
	jnc	.do_read_id
	call	fdc_recalibrate		; second attempt
	jc	.error

.do_read_id:
	mov	cx,3			; 3 attempts

.read_id_loop:
	push	cx
	mov	al,4Ah			; FDC Read ID command
	mov	ah,dl			; read id - byte 1 (head = 0 / drive)
	mov	cl,2			; 2 byte commands
	and	byte [fdc_calib_state],~fdc_irq_flag ; clear IRQ6 flag
	call	fdc_send_cmd
	jc	.error_cmd
	call	fdc_wait_irq		; wait for IRQ6
	jc	.error_cmd		; timeout waiting for interrupt
	mov	cl,7
	call	fdc_get_result		; read result bytes
	jc	.error_cmd
	call	fdc_get_error		; get error code
	pop	cx
	or	ah,ah
	jz	.exit			; if no errors
	loop	.read_id_loop		; retry
	jmp	.error

.error_cmd:
	pop	cx

.error:
	stc

.exit:
	pop	bx
	ret

;=========================================================================
; fdc_select_rate - Select FDC transfer rate
; Input:
;	DS:SI = drive media type
;	DS:008B = last data rate selected
; Output:
;	AL = data transfer rate (bits 7-6)
;	DS:008B = new data rate selected
;-------------------------------------------------------------------------
fdc_select_rate:
	mov	dh,byte [si]		; DH - media state
	mov	dl,byte [fdc_last_rate]	; DL - last selected rate
	and	dx,0C0C0h		; mask rate bits
	cmp	dl,dh			; new rate is the same as the last rate?
	je	.exit			; exit if rate is already selected
	and	byte [fdc_last_rate],3Fh ; clear previous rate bits
	or	byte [fdc_last_rate],dh	; add new bits
	mov	al,dh
	call	fdc_set_rate		; send new rate (in AL) to FDC

.exit:
	mov	dx,word [bp+int_13_dx]	; restore DX
	ret

;=========================================================================
; fdc_set_rate - Set transfer rate
; Input:
;	AL = transfer rate (bits 7 and 6)
;		00h - 500 Kbps (1.2M and 1.44M disks)
;		40h - 300 Kbps (360K disk in 1.2M drive)
;		80h - 250 Kbps (360K disk in 360K drive, or 720K disk)
;		0C0h - 1 Mbps (2.88M disks)
; Output:
;	AL = transfer rate (bits 1 and 0)
;		00h - 500 Kbps	01h - 300 Kbps
;		02h - 250 Kbps	03h - 1 Mbps
;	DX = 3F7h - FDC CCR
;-------------------------------------------------------------------------
fdc_set_rate:
	and	al,fdc_m_rate_bits
	rol	al,1
	rol	al,1
	mov	dx,fdc_ccr_reg
	out	dx,al
	ret

;=========================================================================
; fdc_wait_irq - Wait for FDC interrupt for 2 seconds
; Input:
;	none
; Output:
;	CF clear if interrupt had occurred
;		AH = 00h - successful completion
;	CF set if no interrupt
;		AH = 80h - timeout
;	BX = fdc_calib_state
;	AL,CX - trashed
;-------------------------------------------------------------------------
fdc_wait_irq:

	sti
	clc
	mov	ax,9001h
	int	15h			; call OS hook
	jc	.timeout
	mov	bx,fdc_calib_state	; contains IRQ flag
	xor	cx,cx

%ifdef AT_DELAY

.zero:
	test	byte [bx],fdc_irq_flag	; test IRQ flag
	jnz	.exit			; exit loop if interrupt had occurred

.zero_loop:
	in	al,ppi_pb_reg
	test	al,refresh_flag
	jz	.zero_loop		; wait 15.09 us
	loop	.one			; DEC CX; JNZ .one
	jmp	.timeout

.one:
	test	byte [bx],fdc_irq_flag	; test IRQ flag
	jnz	.exit			; exit loop if interrupt had occurred

.one_loop:
	in	al,ppi_pb_reg
	test	al,refresh_flag
	jnz	.one_loop		; wait 15.09 us
	loop	.zero			; DEC CX; JNZ .one

%else ; AT_DELAY

.1:
	test	byte [bx],fdc_irq_flag	; test IRQ flag
	jnz	.exit			; exit loop if interrupt had occurred

	; wait 38.13 us (Intel 8088 CPU running on 4.77 MHz)
	mov	al,10

.2:
	dec	al
	jnz	.2
	loop	.1

%endif ; AT_DELAY

.timeout:
	mov	ah,fdc_e_timeout
	stc
	ret

.exit:
	and	byte [bx],~fdc_irq_flag	; clear IRQ flag
	mov	ah,fdc_e_success
	clc				; DEBUG - remove after debug
	ret

;=========================================================================
; fdc_wait_ready - Wait until FDC is ready to receive or send commands
;		   but no more than one second
; Input:
; Output:
;	CF clear if FDC is ready
;		AL = FDC main status register
;	CF set on timeout
;		AL trashed
;	DX - trashed
;-------------------------------------------------------------------------

fdc_wait_ready:
	push	cx
	mov	dx,fdc_status_reg
	xor	cx,cx

%ifdef AT_DELAY

.zero:
	in	al,dx			; read I/O port
	test	al,fdc_stat_ready
	jnz	.exit			; exit loop if bit(s) set to 1

.zero_loop:
	in	al,ppi_pb_reg
	test	al,refresh_flag
	jz	.zero_loop		; wait 15.09 us
	dec	cx
	jz	.timeout
	in	al,dx			; read I/O port again
	test	al,fdc_stat_ready
	jnz	.exit			; exit loop of bit(s) set to 1

.one_loop:
	in	al,ppi_pb_reg
	test	al,refresh_flag
	jnz	.one_loop		; wait 15.09 us
	loop	.zero

%else ; AT_DELAY

.1:
	in	al,dx			; read I/O port
	test	al,fdc_stat_ready
	jnz	.exit			; exit loop if bit(s) set to 1

	; wait 38.13 us (Intel 8088 CPU running on 4.77 MHz)
	mov	al,10

.2:
	dec	al
	jnz	.2
	loop	.1

%endif ; AT_DELAY

.timeout:
	stc

.exit:
	pop	cx
	ret
;=========================================================================
; detect_floppy - Detect floppy configuration
;                 set equipment bits accordingly
; Input:
;	none
; Ouput:
;	AL = NVRAM floppy configuration byte
;-------------------------------------------------------------------------
detect_floppy:
%ifdef BIOS_SETUP
	call	get_floppy
%else ; BIOS_SETUP
	mov	al,44h			; FIXME: fake two 1.44MB floppy drives
%endif ; BIOS_SETUP
	cmp	al,00h			; No floppy drives?
	je	.exit
	or	byte [equipment_list],equip_floppies
	test	al,0Fh			; Second floppy drive installed?
	jz	.exit
	or	byte [equipment_list],equip_floppy2
.exit:
	ret

;=========================================================================
; print_floppy - Print floppy configuration
; Input:
;	AL = NVRAM floppy configuration byte
; Ouput:
;	none
;-------------------------------------------------------------------------
print_floppy:
	push	ax
	push	si
	mov	si,msg_floppy
	call	print
	ror	al,1
	ror	al,1
	ror	al,1
	ror	al,1
	mov	si,ax
	and	si,0007h
	shl	si,1
    cs	mov	si,word [tbl_floppy+si]
	call	print
	mov	si,msg_floppy_2
	call	print
	ror	al,1
	ror	al,1
	ror	al,1
	ror	al,1
	mov	si,ax
	and	si,0007h
	shl	si,1
    cs	mov	si,word [tbl_floppy+si]
	call	print
	mov	si,msg_crlf
	call	print
	pop	si
	pop	ax
	ret
