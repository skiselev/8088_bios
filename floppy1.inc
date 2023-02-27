;=========================================================================
; floppy1.inc - BIOS floppy disk services (part 1 of 2)
;	INT 13h, function AH=00h
;	INT 13h, function AH=01h
;	INT 13h, function AH=08h
;	INT 13h, function AH=15h
;	INT 13h, function AH=16h
;	INT 13h, function AH=17h
;	INT 13h, function AH=18h
;	INT 13h, function AH=08h
;	- see floppy2.inc for other INT 13h functions
;-------------------------------------------------------------------------
;
; Compiles with NASM 2.13.02, might work with other versions
;
; Copyright (C) 2010 - 2023 Sergey Kiselev.
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
fdc_data_reg	equ	3F5h		; FDC data register (R/W)
fdc_dir_reg	equ	3F7h		; FDC digital input register (W)
fdc_ccr_reg	equ	3F7h		; FDC control configuration register (R)

; FDC status register bits
fdc_stat_ready	equ	80h		; 1 = FDC is ready to send / receive
fdc_stat_dir	equ	40h		; 0 = CPU -> FDC; 1 = FDC -> CPU
fdc_stat_dma	equ	20h		; 1 = DMA and IRQ enabled
fdc_stat_busy	equ	10h		; 1 = FDC is busy

;-------------------------------------------------------------------------
; DMAC registers

dmac_ch0_addr_reg	equ	00h	; DMAC channel 0 base addres (W)
dmac_ch0_count_reg	equ	01h	; DMAC channel 0 word count (W)
dmac_ch1_addr_reg	equ	02h	; DMAC channel 1 base addres (W)
dmac_ch1_count_reg	equ	03h	; DMAC channel 1 word count (W)
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
; floppy drive type definitions

type_none	equ	00h
type_360	equ	01h
type_1200	equ	02h
type_720	equ	03h
type_1440	equ	04h
type_2880	equ	06h

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
	mov	byte [fdc_last_error],ah  ; save the error code
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
	mov	dx,fdc_status_reg	; get status register address
	in	al,dx
	test	al,80h
	jz	.try_again		; try again if FDC not ready
	test	al,40h
	jz	.fdc_ready		; FDC ready to recieve data
.try_again:
	call	fdc_reset
	mov	dx,fdc_status_reg	; get status register address
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
	mov	byte [fdc_ctrl_status],al  ; save ST0
	mov	ah,al			; save AL to AH
	call	fdc_read		; read current cylinder
	jc	fdc_init_error
	mov	byte [fdc_ctrl_status+1],al  ; save
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
	xor	si,si
	mov	ds,si
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
;		BL = drive type
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
;	based on drive type
;-------------------------------------------------------------------------
int_13_fn08:
	cmp	dl,80h			; valid floppy drive number?
	jae	.invalid_drive		; hard drive number specified
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
	call	get_drive_type		; returns drive type in AL
	jc	.non_existent_drive	; return no drive if there is an error

	mov	byte [bp+int_13_dh],1	; maximal head number is 1 for floppy
	mov	byte [bp+int_13_bl],al	; pass drive type to caller
	mov	cx,cs
	mov	es,cx			; diskette parameter table segment

	cmp	al,type_360
	je	.set_360
	cmp	al,type_720
	je	.set_720
	cmp	al,type_1200
	je	.set_1200
	cmp	al,type_1440
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
	call	get_media_state
	test	bl,fdc_m_established	; media type established?
	jnz	.set_parameters		; no need to update if established
	call	set_media_state

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
	mov	byte [bp+int_13_bl],cl	; drive type is zero
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
	cmp	al,type_none
	je	.exit
	mov	ah,01h			; assume no change-line support
	cmp	al,type_360
	je	.exit
	cmp	al,type_720		; no change-line support on 720K drives?
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
	cmp	al,type_360
	je	.no_change_line		; report disk changed for 360K drives
	cmp	al,type_720
	je	.no_change_line		; report disk changed for 720K drives

	call	fdc_motor_on		; turn motor on
	xor	ah,ah			; assume disk not changed
	mov	dx,fdc_dir_reg		; get digital input register address
	in	al,dx			; read disk change line
	shl	al,1			; bit 7 to CF
	jnc	.exit
	mov	ah,fdc_e_changed
	stc

.exit:
	mov	byte [fdc_last_error],ah  ; save the error code
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
	mov	cl,byte [bp+int_13_al]	; get original AL value to CL
	cmp	cl,0			; validate parameters
	je	.invalid_parameters
	cmp	cl,4
	ja	.invalid_parameters
	cmp	cl,1			; 360K disk in 360K drive?
	jne	.not_360in360
	mov	ah,0			; no error
	mov	al,fdc_m_360in360	; 360K disk in 360K drive
	jmp	.exit_check_error

.not_360in360:
	call	fdc_motor_on
	call	fdc_disk_change		; check if disk has been changed
	cmp	ah,fdc_e_changed
	jbe	.set_type		; disk changed (no errors other than
					; "disk changed" from fdc_disk_change)
	cmp	ah,fdc_e_timeout
	jne	.set_type		; floppy disk is installed
	call	get_media_state		; BL = media state
	cmp	bl,fdc_m_720		; 250 Kbps and not 5.25?
	je	.exit_check_error
	mov	al,fdc_m_try_360in1200	; 300 Kpbs, try 360 in 1.2M
	jmp	.exit_check_error

.set_type:
	cmp	cl,4			; 720K in 720K?
	jne	.check_360in1200	; jump if not 720K in 720K
	mov	al,fdc_m_720
	jmp	.exit_check_error

.check_360in1200:
	cmp	cl,2			; 360K in 1.2M?
	jne	.set_1200in1200		; jump if not 360K in 1.2M
	mov	al,fdc_m_360in1200
	jmp	.exit_check_error

.set_1200in1200:
	mov	al,fdc_m_1200in1200	; 1.2M in 1.2M

.exit_check_error:
	call	set_media_state		; set media state to value in AL
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
	cmp	al,type_360		; 360K drive?
	jne	.try_drive_1200
	cmp	cx,2709h	 	; 40 tracks 9 sectors?
	jnz	.unsupported_format
	mov	al,fdc_m_360in360	; 360K in 360K established, 250 Kbps
	lea	di,[media_360_in_360]
	jmp	.set_media

.try_drive_1200:
	cmp	al,type_1200		; 1.2M drive?
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
	cmp	al,type_2880		; 2.88M drive?
	jne	.try_drive_1440
	cmp	cx,4F24h		; 80 tracks 36 sectors?
	jne	.try_media_1440
	mov	al,fdc_m_2880		; indicate 2.88M
	lea	di,[media_2880]
	jmp	.set_media

.try_drive_1440:
	cmp	al,type_1440		; 1.44M drive?
	jne	.try_drive_720

.try_media_1440:
	cmp	cx,4F12h		; 80 tracks 18 sectors?
	jne	.try_media_720
	mov	al,fdc_m_1440		; indicate 1.44M
	lea	di,[media_1440]
	jmp	.set_media

.try_drive_720:
	cmp	al,type_720		; 720K drive?
	jne	.unsupported_format	; should never happen...

.try_media_720:	
	cmp	cx,4F09h		; 80 tracks 9 sectors?
	jne	.unsupported_format
	mov	al,fdc_m_720		; other established, 250Kbps
	lea	di,[media_720]		; 720K

.set_media:
	call	set_media_state		; set drive media state (in AL)
	call	fdc_set_rate		; transfer rate in AL (bits 7-6)
	mov	word [bp+int_13_di],di	; return parameters table - offset
	mov	cx,cs
	mov	es,cx			; return parameters table - segment
	mov	byte [fdc_last_error],0	; no errors
	xor	ah,ah
	jmp	int_13_upd_exit

.unsupported_format:
	mov	ah,fdc_e_format		; not supported or drive type unknown
	stc
	jmp	int_13_upd_exit

;=========================================================================
; fdc_recalibrate - Recalibrate disk drive, seek to cylinder 0
; Input:
;	[BP+int_13_dl] = drive number (0 or 1)
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
	mov	ah,byte [bp+int_13_dl]	; drive number - 2nd byte of command
	mov	cl,2			; 2 bytes command
	and	byte [fdc_calib_state],~fdc_irq_flag ; clear IRQ flag
	call	fdc_send_cmd
	jc	recal_end		; failure
	call	fdc_wait_irq		; wait for IRQ
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
	mov	dl,byte [bx]			; ST0
	and	dl,60h
	cmp	dl,60h			; abnormal termination + seek end
	stc				; indicate error
	je	recal_end		; failure

	mov	ch,0
	call	set_cylinder		; set current cylinder to 0
	call	set_drive_calibrated	; set drive calibrated in data area
	mov	cx,43h
	call	delay_15us		; 1 ms delay
	xor	ah,ah			; AH = 0, CF = 0 - no error

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
	push	dx
	call	check_drive_calibrated
	jc	.skip_recalibrate	; jump if drive is already calibrated
	call	fdc_recalibrate
	jnc	.skip_recalibrate	; jump if calibration successful
	call	fdc_recalibrate		; try recalibrating again
	jc	.exit			; no luck...

.skip_recalibrate:
	call	get_media_state		; BL = media state
	test	bl,fdc_m_double_step 	; check double stepping bit
	jz	.no_double_stepping
	shl	ch,1			; CH = CH * 2 (double cylinder number)

.no_double_stepping:
	call	check_cylinder		; already at the right cylinder?
	jnz	.do_seek		; jump if seek is required
	cmp	byte [fdc_last_error],fdc_e_seek
	je	.do_seek		; jump if it was a seek error
	xor	ah,ah			; AH = 0, CF = 0 - success
	jmp	.exit

.do_seek:
	mov	al,ch			; cylinder
	mov	si,ax			; SI - low = cylinder number
	mov	al,0Fh			; FDC Seek command
	mov	ah,dh			; head
	shl	ah,1			; move head to bit 2
	shl	ah,1
	or	ah,byte [bp+int_13_dl]	; seek - byte 1 (head / drive)
	mov	cl,3			; 3 bytes command
	and	byte [fdc_calib_state],~fdc_irq_flag ; clear IRQ flag
	call	fdc_send_cmd
	jc	.set_result		; seek error
	call	fdc_wait_irq		; wait for IRQ
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
	call	set_cylinder		; save new cylinder number

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
	xor	ah,ah			; AH = 0, CF = 0 - no error

.set_result:
	mov	byte [fdc_last_error],ah

.exit:
	pop	dx
	pop	cx
	pop	bx
	ret

.get_settle_time:
	test	byte [fdc_motor_state],fdc_write_flag ; test the write bit
	jz	.wait_end		; jump if read / verify - no wait

	call	get_media_state	
	mov	ah,bl			; AH = media state
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
;	DL = drive number (0 or 1) -- FIXME: Does it receive DL?
; Output:
;	none
;-------------------------------------------------------------------------
fdc_motor_on:
	push	ax
	push	cx
	push	dx
	cli				; entering critical section
	mov	byte [fdc_motor_tout],0FFh  ; set timeout to maximum
	mov	dl,byte [bp+int_13_dl]	; DL = drive number
	mov	cl,dl			; CL = drive number
	inc	cl			; CL = drive number + 1
	mov	al,byte [fdc_motor_state] ; AL = motor state byte
	mov	dh,al			; keep a copy in motor state in DH
	and	al,0CFh			; clear drive select bits (5-4)

	shl	dl,1			; shift drive number to bits (5-4)
	shl	dl,1
	shl	dl,1
	shl	dl,1
	or	al,dl			; select drive
	shr	dh,cl			; set CF if motor for is on
	jc	.already_on
	mov	dl,1
	dec	cl
	shl	dl,cl
	or	al,dl			; indicate that motor is on
	mov	byte [fdc_motor_state],al ; update motor state in data area
	sti
	or	al,40h			; no reset
	call	set_fdc_dor
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

	test	byte [fdc_motor_state],fdc_write_flag ; test the write bit
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
	pop	dx
	pop	cx
	pop	ax
	ret

.already_on:
	mov	byte [fdc_motor_state],al ; update motor state (AL) in data area
	sti
	or	al,40h			; no reset
	call	set_fdc_dor
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
	mov	ah,byte [bx+2]		; motor timeout (ticks)
	mov	al,byte [bx+4]		; sectors per track
	inc	al
	pop	ds
	mov	bx,fdc_ctrl_status
	cmp	ch,byte [bx+3]		; same result cylinder?
	jne	.exit
	cmp	dh,byte [bx+4]		; same result head?
	jne	.exit
	mov	al,byte [bx+5]		; result sector number
.exit:
	mov	byte [fdc_motor_tout],ah  ; motor timeout
	mov	bl,al			; next sector to be transferred
	pop	ax
	ret

;========================================================================
; fdc_disk_change - Read disk change line, reset it if active
; Input:
;	AL = drive type
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
	push	bx
	push	cx
	mov	ah,0
	cmp	al,type_360
	je	.exit			; jump if 360K drive (no change line)
	cmp	al,type_720
	je	.exit			; jump if 720K drive (no change line)

	call	get_media_state
	mov	al,bl			; AL = drive media state
	and	al,fdc_m_state_bits
	jz	.exit			; jump if 360K drive (no change line)
	cmp	al,3
	je	.exit			; jump if 360K drive (no change line)
	mov	dx,fdc_dir_reg		; get digital input register address
	in	al,dx			; read disk change line
	shl	al,1
	jnc	.exit			; no disk change
	mov	al,bl			; AL = drive media state
	and	al,~fdc_m_established	; media not detected
	call	set_media_state
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
	mov	dx,fdc_dir_reg		; get digital input register address
	in	al,dx			; read disk change line
	shl	al,1
	jnc	.changed_or_error	; jump if disk change line was reset
	mov	ah,fdc_e_timeout	; failed: no floppy

.changed_or_error:
	stc

.exit:
	mov	dx,word [bp+int_13_dx]	; restore dx
	pop	cx
	pop	bx
	ret

;=========================================================================
; fdc_configure_dma - Configure DMA controller for FDC operation (channel 2)
; Input:
;	AL = DMA mode byte
;		42h - verify (single mode, addr increment, verify, channel 2)
;		46h - read   (single mode, addr increment, write, channel 2)
;		4Ah - write  (single mode, addr increment, read, channel 2)
;	CX = byte count (minus 1)
;	ES:BX -> buffer address for DMA operation (BX is from [BP+int_13_bx])
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
	mov	bx,word [bp+int_13_bx]	; get BX from the stack
	mov	dx,es			; user's buffer segment
	rol	dx,1
	rol	dx,1
	rol	dx,1
	rol	dx,1
	mov	ah,dl			; calculate DMA page number
	and	ah,0Fh			; AH = page number: bits 19 - 16 of ES
	and	dl,0F0h			; DX = DMA offset: bits 15 - 0 of ES
	add	dx,bx			; add user's buffer offset
	adc	ah,0			; increment page number on overflow

	mov	bx,dx			; check if crossing DMA page boundary:
	add	bx,cx			;   add DMA buffer address to byte count
	jc	.dma_boundary		; jump if crossing DMA boundary (64 KiB)

	cli
	out	dmac_mode_reg,al	; send DMA mode byte
	jmp	$+2
	jmp	$+2
	out	dmac_ff_reg,al		; clear first/last flip-flop
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
	out	dmac_ff_reg,al		; clear first/last flip-flop
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
	clc				; CF = 0 - no error

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
	push	cx
	push	dx
	push	ax
	mov	dx,fdc_status_reg	; get status register address
	xor	cx,cx			; busy wait counter

.wait_ready:
	in	al,dx			; read fdc_status_reg
	test	al,fdc_stat_ready	; exit loop if FDC is ready
	jnz	.fdc_ready
	loop	.wait_ready
	pop	ax
	mov	ah,fdc_e_timeout	; timeout
	stc				; indicate error
	jmp	.exit

.fdc_ready:
	test	al,fdc_stat_dir|fdc_stat_dma ; check that DIR and DMA flags are 0
	jnz	.failure		; FDC failure if they are set
	pop	ax
	inc	dx			; DX = fdc_data_reg = fdc_status_reg + 1
	out	dx,al			; write byte
	clc				; no error

.exit:
	pop	dx
	pop	cx
	ret

.failure:
	pop	ax
	mov	ah,fdc_e_failure	; FDC failure
	stc
	jmp	.exit

;=========================================================================
; fdc_get_result - Read FDC result
; Input:
;	none
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
	push	cx
	push	dx
	mov	bx,fdc_ctrl_status
	mov	dx,fdc_status_reg	; get status register address

.next_byte:
        xor     cx,cx			; busy wait counter

.wait_ready:
	in	al,dx			; read fdc_status_reg
	test	al,fdc_stat_ready	; exit loop if FDC is ready
	jnz	.fdc_ready
	loop	.wait_ready
	mov	ah,fdc_e_timeout	; timeout
	stc				; indicate error
	jmp	.exit

.fdc_ready:
	and	al,fdc_stat_dir|fdc_stat_dma|fdc_stat_busy
	test	al,~fdc_stat_busy	; controller finished sending result?
	je	.done
	cmp	al,fdc_stat_dir|fdc_stat_busy
	jne	.failure		; invalid FDC status, indicate FDC failure
	cmp	bx,fdc_ctrl_status+7	; already read 7 bytes?
	jnb	.failure		; controller error if so
	inc	dx			; DX = fdc_data_reg = fdc_status_reg + 1
	in	al,dx
	mov	byte [bx],al
	inc	bx
	dec	dx			; DX = fdc_status_reg = fdc_data_reg - 1
	jmp	.next_byte

.done:
	xor	ah,ah			; operation successful

.exit:
	pop	dx
	pop	cx
	ret

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
;-------------------------------------------------------------------------
fdc_read:
	push	cx
	push	dx
	mov	dx,fdc_status_reg	; get status register address
        xor     cx,cx			; busy wait counter

.wait_ready:
	in	al,dx			; read fdc_status_reg
	test	al,fdc_stat_ready	; exit loop if FDC is ready
	jnz	.fdc_ready
	loop	.wait_ready
	mov	ah,fdc_e_timeout	; timeout
	stc				; indicate error
	jmp	.exit

.fdc_ready:
	in	al,dx
	test	al,fdc_stat_dir		; FDC is ready to send a byte? 
	jz	.failure		; jump if not ready
;	jmp	short $+2		; I/O delay
;	jmp	short $+2
	inc	dx			; DX = fdc_data_reg = fdc_status_reg + 1
	in	al,dx			; read the byte
	clc				; no error

.exit:
	pop	dx
	pop	cx
	ret

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
	mov	bx,word [fdc_ctrl_status]
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
	and	byte [fdc_calib_state],~fdc_irq_flag ; clear IRQ flag
	and	byte [fdc_motor_state],~fdc_write_flag ; clear the write bit
	mov	al,byte [fdc_motor_state] ; AL = motor state byte
	and	al,0BFh			; clear bit 6 - reset
	push	ax			; set_fdc_dor trashes AX, save it
	call	set_fdc_dor
	pop	ax
	mov	cx,3
	call	delay_15us		; 30-45 us delay
	or	al,40h			; set bit 6 - no reset
	call	set_fdc_dor
	sti
	call	fdc_wait_irq		; wait for IRQ
	jc	.exit
	and	byte [fdc_calib_state],~fdc_irq_flag ; clear IRQ flag
	xor	ah,ah			; no errors
.exit:
	ret

;=========================================================================
; fdc_detect_media - Detect media type, update it in fdc_media_state[drive]
; Input:
;	[BP+int_13_dl] = drive number
; Output:
;	CF clear if successful
;		AH = 00h
;	CF set on error
;		AH = 20h - invalid drive type
;-------------------------------------------------------------------------
fdc_detect_media:
	push	dx
	push	cx

	mov	ah,fdc_e_failure	; in case the get_drive_type fails
	mov	dl,byte [bp+int_13_dl]	; restore DL in case it was trashed
	call	get_drive_type		; get drive type to AL
	jc	.exit			; drive doesn't exist or invalid type

	cmp	al,type_720
	je	.set_720
	cmp	al,type_1200
	je	.detect_1200
	cmp	al,type_1440
	je	.detect_1440
	cmp	al,type_2880
	je	.detect_2880

	mov	al,fdc_m_360in360	; set 360K disk in 360K drive
	jmp	.set_rate

.set_720:
	mov	al,fdc_m_720

.set_rate:
	push	ax
	call	fdc_set_rate		; transfer rate in AL
	pop	ax
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
	call	set_media_state		; set media state
	xor	ah,ah			; AH = 0, CF = 0 - no error
.exit:
	mov	byte [fdc_last_error],ah
	pop	cx
	pop	dx
	ret

;=========================================================================
; fdc_read_id - Read ID
; Input:
;	[BP+int_13_dl] = drive number
; Output:
;	CF clear if successful
;		AH = 0 - successful completion
;		AL = ID (bits 7-6)
;	CF set on error
;		AH = error code
;	AX,CX - trashed
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
; FIXME - does drive number in DL gets to here?
;	mov	ah,dl			; read id - byte 1 (head = 0 / drive)
	mov	ah,byte [bp+int_13_dl]	; read id - byte 1 (head = 0 / drive)
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
; fdc_set_rate - Set transfer rate
; Input:
;	AL = transfer rate (bits 7 and 6)
;		00h - 500 Kbps (1.2M and 1.44M disks)
;		40h - 300 Kbps (360K disk in 1.2M drive)
;		80h - 250 Kbps (360K disk in 360K drive, or 720K disk)
;		0C0h - 1 Mbps (2.88M disks)
; Output:
;	none
;-------------------------------------------------------------------------
fdc_set_rate:
	push	ax
	and	al,fdc_m_rate_bits	; get the data rate bits only
	and	byte [fdc_last_rate],~fdc_m_rate_bits ; clear rate bits
	or	byte [fdc_last_rate],al	; set new rate bits
	rol	al,1
	rol	al,1
	mov	dx,fdc_ccr_reg		; get control configuration register address
	out	dx,al
	mov	dx,word [bp+int_13_dx]	; restore DX
	pop	ax
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
;	AL - trashed
;-------------------------------------------------------------------------
fdc_wait_irq:
	push	cx
	sti
	clc
	mov	ax,9001h
	int	15h			; call OS hook
	jc	.timeout
	mov	bx,fdc_calib_state	; contains IRQ flag
	xor	cx,cx

%ifdef AT_DELAY

.zero_loop:
	test	byte [bx],fdc_irq_flag	; test IRQ flag
	jnz	.exit			; exit loop if interrupt had occurred
	in	al,ppi_pb_reg
	test	al,refresh_flag
	jz	.zero_loop		; wait 15.09 us
	loop	.one_loop		; DEC CX; JNZ .one_loop
	jmp	.timeout

.one_loop:
	test	byte [bx],fdc_irq_flag	; test IRQ flag
	jnz	.exit			; exit loop if interrupt had occurred
	in	al,ppi_pb_reg
	test	al,refresh_flag
	jnz	.one_loop		; wait 15.09 us
	loop	.zero_loop		; DEC CX; JNZ .zero_loop

%else ; AT_DELAY

.1:
	; wait 38.13 us (Intel 8088 CPU running on 4.77 MHz)
	mov	al,10
.2:
	test	byte [bx],fdc_irq_flag	; test IRQ flag
	jnz	.exit			; exit loop if interrupt had occurred
	dec	al
	jnz	.2
	loop	.1

%endif ; AT_DELAY

.timeout:
	mov	ah,fdc_e_timeout	; timeout
	stc
	pop	cx
	ret

.exit:
	and	byte [bx],~fdc_irq_flag	; clear IRQ flag
	xor	ah,ah			; AH = 0, CF = 0 - no error
	pop	cx
	ret

;=========================================================================
; set_fdc_dor - set FDC Digital Output Register - DOR
; Input:
;	AL = new motor state
; Output:
;	AL, DX trashed
;-------------------------------------------------------------------------
set_fdc_dor:
	ror	al,1
	ror	al,1
	ror	al,1
	ror	al,1
	or	al,08h			; DMA+IRQ enabled
	mov	dx,fdc_dor_reg		; get digital output register address
	out	dx,al			; send the state to FDC
	ret

;=========================================================================
; get_media_state - Get drive's media state from the data area
; Input:
;	[BP+int_13_dl] = drive number
; Output:
;	BL = drive's media state
;	BH = 0 (destroyed)
;-------------------------------------------------------------------------
get_media_state:
	mov	bl,byte [bp+int_13_dl]
	mov	bh,0			; BX = drive number
	mov	bl,byte [fdc_media_state+bx]
	ret

;=========================================================================
; set_media_state - Store drive's media state in the data area
; Input:
;	AL = media state
;	[BP+int_13_dl] = drive number
; Output:
;	none
;-------------------------------------------------------------------------
set_media_state:
	push	bx
	mov	bl,byte [bp+int_13_dl]
	mov	bh,0			; BX = drive number
	mov	byte [fdc_media_state+bx],al
	pop	bx
	ret

;=========================================================================
; check_cylinder - Compare specified cylinder with value in the BIOS data area
; Input:
;	CH = current cylinder
;	[BP+int_13_dl] = drive number
; Output:
;	ZF = 1 - cylinder matches
;-------------------------------------------------------------------------
check_cylinder:
	push	bx
	mov	bl,byte [bp+int_13_dl]
	mov	bh,0			; BX = drive number
	cmp	byte [fdc_cylinder+bx],ch
	pop	bx
	ret

;=========================================================================
; set_cylinder - Store drive's current cylinder into the BIOS data area
; Input:
;	CH = current cylinder
;	[BP+int_13_dl] = drive number
; Output:
;	none
;-------------------------------------------------------------------------
set_cylinder:
	push	bx
	mov	bl,byte [bp+int_13_dl]
	mov	bh,0			; BX = drive number
	mov	byte [fdc_cylinder+bx],ch
	pop	bx
	ret

;=========================================================================
; check_drive_calibrated - Check if the drive calibrated bit is set
;			   in the BIOS data area
; Input:
;	[BP+int_13_dl] = drive number
; Output:
;	CF = 1 - bit set (drive calibrated)
;-------------------------------------------------------------------------
check_drive_calibrated:
	push	cx
	mov	ch,byte [fdc_calib_state] ; calibration state for primary FDC
	mov	cl,byte [bp+int_13_dl]	; CL = drive number
	inc	cl
	shr	ch,cl			; set CF if drive is calibrated
	pop	cx
	ret

;=========================================================================
; set_drive_calibrated - Set drive calibrated bit in the BIOS data area
; Input:
;	[BP+int_13_dl] = drive number
; Output:
;	none
;-------------------------------------------------------------------------
set_drive_calibrated:
	push	cx
	mov	cl,byte [bp+int_13_dl]	; CL = drive number
	mov	ch,1			; bit 0 set
	shl	ch,cl			; move it into the right position
	or	byte [fdc_calib_state],ch ; set the bit for the primary FDC
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
; get_drive_type - Read drive type from NVRAM or Flash ROM configuration
; Input:
;	DL = drive number (0 or 1)
; Output:
;	CF clear if successful
;		AL = drive type
;	CF set on error (invalid drive type)
;=========================================================================

get_drive_type:
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
	cmp	al,type_none
	je	.error
	cmp	al,5			; invalid value
	je	.error
	cmp	al,type_2880
	ja	.error
	clc
	ret

.error:
	stc
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
