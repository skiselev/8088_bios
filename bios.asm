;=========================================================================
; main.asm - BIOS main file
;	Skeleton for the BIOS
;	Power On Self Test (POST)
;	Interrupt table setup
;       INT 11h - Get equipment list
;       INT 12h - Get memory size
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
; Fixed BIOS Entry Points
; Source:
;	Intel(R) Platform Innovation Framework for EFI
;	Compatibility Support Module Specification
;	Section 5.2
;
;	Location	Description
;	--------	-----------
;	F000:E05B	POST Entry Point
;	F000:E2C3	NMI Entry Point
;	F000:E401	HDD Parameter Table
;	F000:E6F2	INT 19 Entry Point
;	F000:E6F5	Configuration Data Table
;	F000:E729	Baut Rate Generator Table
;	F000:E739	INT 14 Entry Point
;	F000:E82E	INT 16 Entry Point
;	F000:E987	INT 09 Entry Point
;	F000:EC59	INT 13 (Floppy) Entry Point
;	F000:EF57	INT 0E Entry Point
;	F000:EFC7	Floppy Disk Controller Parameter Table
;	F000:EFD2	INT 17
;	F000:F065	INT 10 (Video) Entry Point
;	F000:F0A4	INT 1D MDA and CGA Video Parameter Table
;	F000:F841	INT 12 Entry Point
;	F000:F84D	INT 11 Entry Point
;	F000:F859	INT 15 Entry Point
;	F000:FA6E	Low 128 Characters of Graphic Video Font
;	F000:FE6E	INT 1A Entry Point
;	F000:FEA5	INT 08 Entry Point
;	F000:FF53	Dummy Interrupt Handler (IRET)
;	F000:FF54	INT 05 (Print Screen) Entry Point
;	F000:FFF0	Power-On Entry Point
;	F000:FFF5	ROM Date in ASCII "MM/DD/YY" Format (8 Characters)
;	F000:FFFE	System Model (0xFC - AT, 0xFE - XT)

	cpu	8086

%include "macro.inc"
%include "config.inc"
%include "errno.inc"

bioscseg	equ	0F000h
biosdseg	equ	0040h

pic1_reg0	equ	20h
pic1_reg1	equ	21h
pit_ch0_reg	equ	40h
pit_ch1_reg	equ	41h
pit_ch2_reg	equ	42h
pit_ctl_reg	equ	43h

; 8255 PPI port A I/O register - Read - keyboard data
ppi_pa_reg	equ	60h	; 8255 PPI port A I/O register

; Port 61h - 8255 PPI Port B - Write only
ppi_pb_reg	equ	61h	; 8255 PPI port B I/O register
iochk_disable	equ	08h	; clear and disable ~IOCHK NMI
refresh_flag	equ	10h	; refresh flag, toggles every 15us
iochk_enable	equ	0F7h	; enable ~IOCHK NMI
iochk_status	equ	40h	; ~IOCHK status - 1 = ~IOCHK NMI signalled

%ifdef MACHINE_XT or MACHINE_FE2010A
; Port 62h - 8255 PPI Port C - Read only
ppi_pc_reg	equ	62h	; 8255 PPI port C I/O registerA
; XT DIP switches 1-4 (read when bit 3 of PPI Port B is 1)
sw_post_loop	equ	01h	; XT DIP switch 1 - 1 = Loop on POST
sw_fpu_present	equ	02h	; XT DIP switch 2 - 1 = FPU present
sw_ram_256k	equ	00h	; XT DIP switches 3-4 - 256 KiB
sw_ram_512k	equ	04h	; XT DIP switches 3-4 - 512 KiB
sw_ram_576k	equ	08h	; XT DIP switches 3-4 - 576 KiB
sw_ram_640k	equ	0Ch	; XT DIP switches 3-4 - 640 KiB
; XT DIP switches 5-8 (read when bit 3 of PPI Port B is 0)
sw_vid_none	equ	00h	; XT DIP switches 5-6 - No video, EGA, or VGA
sw_vid_color_40	equ	01h	; XT DIP switches 5-6 - CGA, 80x25
sw_vid_color_80	equ	02h	; XT DIP switches 5-6 - CGA, 40x25
sw_vid_mono	equ	03h	; XT DIP switches 5-6 - Monochome, 80x25
sw_one_floppy	equ	00h	; XT DIP switches 7-8 - One floppy
sw_two_floppies	equ	01h	; XT DIP switches 7-8 - Two floppies
sw_three_floppies equ	02h	; XT DIP switches 7-8 - Three floppies
sw_four_floppies  equ	03h	; XT DIP switches 7-8 - Four floppies
%endif ; MACHINE_XT or MACHINE_FE2010A

; FE2010/FE2010 - Ports 62h-63h
%ifdef MACHINE_FE2010A
; Port 62h - Chipset Control Register - Write
fe_control_reg	equ	62h
fe_fpu_present	equ	02h	; FPU present
fe_ram_256k	equ	00h	; XT DIP switches 3-4 - 256 KiB
fe_ram_512k	equ	04h	; XT DIP switches 3-4 - 512 KiB
fe_ram_576k	equ	08h	; XT DIP switches 3-4 - 576 KiB
fe_ram_640k	equ	0Ch	; XT DIP switches 3-4 - 640 KiB
fe_one_floppy	equ	00h	; XT DIP switches 7-8 - One floppy
fe_two_floppies	equ	40h	; XT DIP switches 7-8 - Two floppies

; Port 63h - Chipset Configuration Register - Write only
fe_config_reg	equ	63h	; Chipset configuration register
fe_par_disable	equ	01h	; Disable memory parity checking
fe_fpu_nma_ena 	equ	02h	; Enable FPU NMI
fe_config_lock	equ	08h	; Write lock of control register and
				; bits 0-4 of configuration register
fe_clk_7_16mhz	equ	40h	; FE2010A 7.16 MHz CPU clock frequency
fe_clk_9_55mhz	equ	80h	; FE2010A 9.55 MHz CPU clock frequency
%endif ; MACHINE_FE2010A

; IBM PC/XT - Port 63h - 8255 PPI Control Word
%ifdef MACHINE_XT
ppi_cwd_reg	equ	63h	; 8255 PPI control word register
ppi_cwd_value	equ	99h	; 8255 PPI control word value for IBM XT:
				; Port A - mode 0 (simple I/O), input
				; Port B - mode 0 (simple I/O), output
				; Port C - output
%endif ; MACHINE_XT

post_reg	equ	80h	; POST status output port
nmi_mask_reg	equ	0A0h
%ifdef SECOND_PIC
pic2_reg0	equ	0A0h
pic2_reg1	equ	0A1h
%endif ; SECOND_PIC
unused_reg	equ	0C0h	; used for hardware detection and I/O delays
cga_mode_reg	equ	3D8h
mda_mode_reg	equ	3B8h

; NMI mask (written to 0A0h)
nmi_disable	equ	00h	; disable NMI
nmi_disa_mask	equ	7Fh	; disable NMI AND mask (bit 7 = 0)
nmi_enable	equ	80h	; enable NMI OR mask (bit 7 = 1)

pic_freq	equ	1193182	; PIC input frequency - 14318180 MHz / 12

;========================================================================
; BIOS data area variables
;------------------------------------------------------------------------
equip_serial	equ	00h	; word[4] - addresses of serial ports
				; or 0 if port doesn't exist
equip_parallel	equ	08h	; word[3] - addresses of parallel ports
				; or 0 if port doesn't exist
ebda_segment	equ	0Eh	; word - address of EBDA segment
equipment_list	equ	10h	; word - equpment list
equip_floppies	equ	0000000000000001b	; floppy drivers installed
equip_fpu	equ	0000000000000010b	; FPU installed
equip_mouse	equ	0000000000000100b
equip_video	equ	0000000000110000b	; video type bit mask
equip_color_40	equ	0000000000010000b	; color 40x24 (mode 1)
equip_color_80	equ	0000000000100000b	; color 80x25 (mode 3)
equip_mono	equ	0000000000110000b	; mono 80x25 (mode 7)
equip_floppy2	equ	0000000001000000b	; 2nd floppy drive installed
;			|||||||||||||||`-- floppy drives installed
;			||||||||||||||`-- FPU installed
;			|||||||||||||`-- PS/2 mouse installed
;			||||||||||||`-- reserved
;			||||||||||`--- initial video mode
;			||||||||`---- number of floppy drives - 1
;			|||||||`---- O = DMA installed
;			||||`------ number of serial ports
;			|||`------ game adapter installed
;			||`------ internal modem?!
;			`------- number of parallel ports

post_flags	equ	12h	; byte - post flags
post_setup	equ	01h	; run NVRAM setup
memory_size	equ	13h	; word - memory size in KiB
kbd_flags_1	equ	17h	; byte - keyboard shift flags 1
kbd_flags_2	equ	18h	; byte - keyboard shift flags 2
kbd_alt_keypad	equ	19h	; byte - work area for Alt+Numpad
kbd_buffer_head	equ	1Ah	; word - keyboard buffer head offset
kbd_buffer_tail	equ	1Ch	; word - keyboard buffer tail offset
kbd_buffer	equ	1Eh	; byte[32] - keyboard buffer
fdc_calib_state	equ	3Eh	; byte - floppy drive recalibration status
fdc_motor_state	equ	3Fh	; byte - floppy drive motor status
fdc_motor_tout	equ	40h	; byte - floppy drive motor off timeout (ticks)
fdc_last_error	equ	41h	; byte - status of last diskette operation
fdc_ctrl_status	equ	42h	; byte[7] - FDC status bytes
video_mode	equ	49h	; byte - active video mode number
video_columns	equ	4Ah	; word - number of text columns for active mode
video_page_size	equ	4Ch	; word - size of video page in bytes
video_page_offt	equ	4Eh	; word - offset of the active video page
video_cur_pos	equ	50h	; byte[16] - cursor position for each page
video_cur_shape	equ	60h	; word - cursor shape
video_page	equ	62h	; byte - active video page
video_port	equ	63h	; word - I/O port for the display adapter
video_mode_reg	equ	65h	; byte - video adapter mode register
video_palet_reg	equ	66h	; byte - color palette
last_irq	equ	6Bh	; byte - Last spurious IRQ number
ticks_lo	equ	6Ch	; word - timer ticks - low word
ticks_hi	equ	6Eh	; word - timer ticks - high word
new_day		equ	70h	; byte - 1 = new day flag
break_flag	equ	71h	; byte - bit 7 = 1 if Ctrl-Break was pressed
warm_boot	equ	72h	; word - Warm boot if equals 1234h
printer_timeout	equ	78h	; byte[3] - parallel port timeout values
serial_timeout	equ	7Ch	; byte[4] - serial port timeout values
kbd_buffer_start equ	80h	; word - keyboard buffer start offset
kbd_buffer_end	equ	82h	; word - keyboard buffer end offset
video_rows	equ	84h	; byte - number of text rows (EGA+)
fdc_last_rate	equ	8Bh	; byte - last data rate / step rate
fdc_info	equ	8Fh	; byte - floppy dist drive information
fdc_media_state	equ	90h	; byte[4] - drive media state (drives 0 - 3)
fdc_cylinder	equ	94h	; byte[2] - current cylinder (drives 0 - 1)
kbd_flags_3	equ	96h	; byte - keyboard status flags 3
kbd_flags_4	equ	97h	; byte - keyboard status flags 4
prt_scrn_flags	equ	100h	; byte - print screen flags
prt_scrn_ready	equ	00h	;	print screen is not in progress
prt_scrn_run	equ	01h	; 	print screen is in progress
prt_scrn_fail	equ	0FFh	;	last print screen attempt has failed

;=========================================================================
; Extended BIOS data area variables
;-------------------------------------------------------------------------
ebda_size	equ	0h
mouse_driver	equ	22h	; 4 bytes - pointer to mouse driver
mouse_flags_1	equ	26h
mouse_flags_2	equ	27h
mouse_data	equ	28h	; 8 bytes - mouse data buffer

	org	START		; Use only upper 32 KiB of ROM

;=========================================================================
; Includes
;-------------------------------------------------------------------------
%include	"messages.inc"		; POST messages
%ifndef MACHINE_XT			; No space in XT ROM for font
%include	"fnt80-FF.inc"		; font for graphics modes
%endif ; MACHINE_XT
;%include	"inttrace.inc"		; Uncomment for interrupt debugging
%ifdef AT_RTC
%include	"rtc.inc"		; RTC and CMOS read / write functions
%endif ; AT_RTC
%ifdef FLASH_NVRAM
%include	"flash.inc"		; Flash ROM configuration functions
%endif ; FLASH_NVRAM
%ifdef BIOS_SETUP
%include	"setup.inc"		; NVRAM setup functions
%endif ; BIOS_SETUP
%include	"delay.inc"		; delay function
%include	"time1.inc"		; time services
%include	"floppy1.inc"		; floppy services
%ifdef AT_KEYBOARD
%include	"at_kbc.inc"		; keyboard controller functions
%endif ; AT_KEYBOARD
%include	"scancode.inc"		; keyboard scancodes translation func.
%include	"serial1.inc"		; serial port services & detection
%include	"printer1.inc"		; parallel printer services & detection
%ifdef PS2_MOUSE
%ifndef SECOND_PIC
%error "PS2_MOUSE depends on SECOND_PIC. Please fix config.inc."
%endif ; SECOND_PIC
%include	"ps2aux.inc"
%endif
%include	"sound.inc"		; sound test
%include	"cpu.inc"		; CPU and FPU detection

%ifdef SECOND_PIC

;=========================================================================
; int_ignore2 - signal end of interrupt to PIC if hardware interrupt, return
;-------------------------------------------------------------------------
int_ignore2:
	push	ax
	mov	al,20h
	out	pic2_reg0,al	; signal EOI to the slave PIC
	out	pic1_reg0,al	; signal EOI to the master PIC
	pop	ax
	iret

;=========================================================================
; int_71 - IRQ9 ISR, emulate IRQ2
;-------------------------------------------------------------------------
int_71:
	push	ax
	mov	al,20h
	out	pic2_reg0,al	; signal EOI to the slave PIC
	pop	ax
	int	0Ah		; call IRQ2 ISR
	iret

;=========================================================================
; int_75 - IRQ13 ISR, emulate NMI by FPU
;-------------------------------------------------------------------------
int_75:
	push	ax
	mov	al,20h
	out	pic2_reg0,al	; signal EOI to the slave PIC
	out	pic1_reg0,al	; signal EOI to the master PIC
	pop	ax
	int	02h		; call NMI ISR
	iret

%endif ; SECOND_PIC

;=========================================================================
; boot_os -Boot the OS
;-------------------------------------------------------------------------
boot_os:

;-------------------------------------------------------------------------
; Check for F1 (setup key), run setup utility if pressed

	mov	ah,01h
	int	16h
	jz	.no_key
	mov	ah,00h
	int	16h			; read the keystroke
	cmp	ax,3B00h		; F1?
	jne	.no_key
	or	byte [post_flags],post_setup
.no_key:

	test	byte [post_flags],post_setup
	jz	.no_setup

%ifdef BIOS_SETUP
	call	nvram_setup
%endif ; BIOS_SETUP

.no_setup:

%ifdef TURBO_MODE
	call	get_config_a		; read BIOS configuration byte A
	and	al,nvram_trbo_mask
	call	set_cpu_clk		; set CPU clock
%endif ; TURBO_MODE

	mov	al,e_boot		; boot the OS POST code	
	call code_post

	mov	si,msg_boot
	call	print
	int	19h			; boot the OS

;=========================================================================
; print - print ASCIIZ string to the console
; Input:
;	CS:SI - pointer to string to print
; Output:
;	none
;-------------------------------------------------------------------------
print:
	pushf
	push	ax
	push	bx
	push	si
	push	ds
	push	cs
	pop	ds
	cld
.1:
	lodsb
	or	al,al
	jz	.exit
	mov	ah,0Eh
	mov	bl,0Fh
	int	10h
	jmp	.1
.exit:
	pop	ds
	pop	si
	pop	bx
	pop	ax
	popf
	ret

;=========================================================================
; print_hex - print 16-bit number in hexadecimal
; Input:
;	AX - number to print
; Output:
;	none
;-------------------------------------------------------------------------
print_hex:
	xchg	al,ah
	call	print_byte		; print the upper byte
	xchg	al,ah
	call	print_byte		; print the lower byte
	ret

;=========================================================================
; print_byte - print a byte in hexadecimal
; Input:
;	AL - byte to print
; Output:
;	none
;-------------------------------------------------------------------------
print_byte:
	rol	al,1
	rol	al,1
	rol	al,1
	rol	al,1
	call	print_digit
	rol	al,1
	rol	al,1
	rol	al,1
	rol	al,1
	call	print_digit
	ret

;=========================================================================
; print_dec - print 16-bit number in decimal
; Input:
;	AX - number to print
; Output:
;	none
;-------------------------------------------------------------------------
print_dec:
	push	ax
	push	cx
	push	dx
	mov	cx,10		; base = 10
	call	.print_rec
	pop	dx
	pop	cx
	pop	ax
	ret

.print_rec:			; print all digits recursively
	push	dx
	xor	dx,dx		; DX = 0
	div	cx		; AX = DX:AX / 10, DX = DX:AX % 10
	cmp	ax,0
	je	.below10
	call	.print_rec	; print number / 10 recursively
.below10:
	mov	ax,dx		; reminder is in DX
	call	print_digit	; print reminder
	pop	dx
	ret

;=========================================================================
; print_digit - print hexadecimal digit
; Input:
;	AL - bits 3...0 - digit to print (0...F)
; Output:
;	none
;-------------------------------------------------------------------------
print_digit:
	push	ax
	push	bx
	and	al,0Fh
	add	al,'0'			; convert to ASCII
	cmp	al,'9'			; less or equal 9?
	jna	.1
	add	al,'A'-'9'-1		; a hex digit
.1:
	mov	ah,0Eh			; Int 10 function 0Eh - teletype output
	mov	bl,07h			; just in case we're in graphic mode
	int	10h
	pop	bx
	pop	ax
	ret

;=========================================================================
; code_post
; Input:
;	AL - byte to print
; Output:
;	none
;-------------------------------------------------------------------------
code_post:
	
	out	post_reg,al
	push es
	push dx
	push ax
			
	mov dx,0b800h
	mov es,dx
	mov ah, 01Fh ; Attribute Bright White over Blue
	
	rol	al,1
	rol	al,1
	rol	al,1
	rol	al,1
	push ax	
	call get_digit	
	es	mov	word [13Ah],ax ; (Position 1, 77)
	pop ax
	
	rol	al,1
	rol	al,1
	rol	al,1
	rol	al,1	
	call get_digit	
	es	mov	word [13Ch],ax ; (Position 1, 78)
		
	pop ax
	pop dx
	pop es
	push cx
	mov cx, 0F000h ; A bit delay to visualization
.delay
	nop
	loop .delay
	pop cx
	ret
	
;=========================================================================
; get_digit - get hexadecimal digit
; Input:
;	AL - bits 3...0
; Output:
;	AL - digit to print (0...F)
;-------------------------------------------------------------------------
get_digit:	
	and	al,0Fh
	add	al,'0'			; convert to ASCII
	cmp	al,'9'			; less or equal 9?
	jna	.2
	add	al,'A'-'9'-1		; a hex digit
.2:
	ret

%ifdef EBDA_SIZE
;=========================================================================
; reserve_ebda - reserve EBDA (Extended BIOS Data Area) if using PS2_MOUSE
; Input:
;	AX = memory size in KiB
; Notes:
;	- Assumes that EBDA memory was cleaned
;	- Does not reserve EBDA if PS/2 auxiliary device is not detected
;-------------------------------------------------------------------------
reserve_ebda:
	push	ax
	push	cx
	test	word [equipment_list],equip_mouse
	jz	.no_mouse
	mov	ax,word [memory_size]	; get conventional memory size
	sub	ax,EBDA_SIZE		; substract EBDA size
	mov	word [memory_size],ax	; store new conventional memory size
	mov	cl,6
	shl	ax,cl			; convert to segment
	mov	word [ebda_segment],ax	; store EBDA segment to BIOS variable
	push	ds
	mov	ds,ax
	mov	ax,EBDA_SIZE
    	mov	byte [ebda_size],al	; store EBDA size to EBDA
	pop	ds
	push	si
	mov	si,msg_ebda
	call	print
	call	print_dec
	mov	si,msg_kib
	call	print
	pop	si
.no_mouse:
	pop	cx
	pop	ax
	ret
%endif ; EBDA_SIZE

;=========================================================================
; print display type
;-------------------------------------------------------------------------
print_display:
	mov	si,msg_disp
	call	print
	mov	al,byte [equipment_list] ; get equipment - low byte
	and	al,equip_video		; get video adapter type
	mov	si,msg_disp_mda
	cmp	al,equip_mono		; monochrome?
	jz	.print_disp
	mov	si,msg_disp_cga_80
	cmp	al,equip_color_80	; CGA 80x25?
	jz	.print_disp
	mov	si,msg_disp_cga_40
	cmp	al,equip_color_40	; CGA 40x25?
	jz	.print_disp
	mov	si,msg_disp_ega		; otherwise EGA or later
.print_disp:
	call	print
	ret

%ifdef PS2_MOUSE

;=========================================================================
; print PS/2 mouse presence
;-------------------------------------------------------------------------

print_mouse:
	mov	si,msg_mouse
	call	print
	mov	si,msg_absent
	test	byte [equipment_list],equip_mouse
	jz	.print_mouse
	mov	si,msg_present
.print_mouse:
	call	print
	ret

%endif ; PS2_MOUSE

;=========================================================================	
; interrupt_table - offsets only (BIOS segment is always 0F000h)
;-------------------------------------------------------------------------
interrupt_table:
	dw	int_dummy		; INT 00 - Divide by zero
	dw	int_dummy		; INT 01 - Single step
	dw	int_02			; INT 02 - Non-maskable interrupt
	dw	int_dummy		; INT 03 - Debugger breakpoint
	dw	int_dummy		; INT 04 - Integer overlow (into)
	dw	int_05			; INT 05 - BIOS Print Screen
	dw	int_dummy		; INT 06
	dw	int_dummy		; INT 07
	dw	int_08			; INT 08 - IRQ0 - Timer Channel 0
	dw	int_09			; INT 09 - IRQ1 - Keyboard
	dw	int_ignore		; INT 0A - IRQ2
	dw	int_ignore		; INT 0B - IRQ3
	dw	int_ignore		; INT 0C - IRQ4
	dw	int_ignore		; INT 0D - IRQ5
	dw	int_0E			; INT 0E - IRQ6 - Floppy
	dw	int_ignore		; INT 0F - IRQ7
	dw	int_10			; INT 10 - BIOS Video Services
	dw	int_11			; INT 11 - BIOS Get Equipment List
	dw	int_12			; INT 12 - BIOS Get Memory Size
	dw	int_13			; INT 13 - BIOS Floppy Disk Services
	dw	int_14			; INT 14 - BIOS Serial Communications
	dw	int_15			; INT 15 - BIOS Misc. System Services
	dw	int_16			; INT 16 - BIOS Keyboard Services
	dw	int_17			; INT 17 - BIOS Parallel Printer svc.
	dw	int_18			; INT 18 - BIOS Start ROM BASIC
	dw	int_19			; INT 19 - BIOS Boot the OS
	dw	int_1A			; INT 1A - BIOS Time Services
	dw	int_dummy		; INT 1B - DOS Keyboard Break
	dw	int_dummy		; INT 1C - User Timer Tick
	dw	int_1D			; INT 1D - Video Parameters Table
	dw	int_1E			; INT 1E - Floppy Parameters Table
%ifndef MACHINE_XT
	dw	int_1F			; INT 1F - Font For Graphics Mode
%else ; MACHINE_XT
	dw	int_ignore
%endif ; MACHINE_XT

%ifdef SECOND_PIC
interrupt_table2:
	dw	int_70			; INT 70 - IRQ8 - RTC
	dw	int_71			; INT 71 - IRQ9 - redirection
	dw	int_ignore2		; INT 72 - IRQ10
	dw	int_ignore2		; INT 73 - IRQ11
%ifndef PS2_MOUSE
	dw	int_ignore2		; INT 74 - IRQ12 - PS/2 mouse
%else
	dw	int_74			; INT 74 - IRQ12 - PS/2 mouse
%endif
	dw	int_75			; INT 75 - IRQ13 - FPU
	dw	int_ignore2		; INT 76 - IRQ14
	dw	int_ignore2		; INT 77 - IRQ15
%endif ; SECOND_PIC

;=========================================================================
; cold_start, warm_start - BIOS POST (Power on Self Test) starts here
;-------------------------------------------------------------------------	
	setloc	0E05Bh		; POST Entry Point
cold_start:
	mov	ax,biosdseg
	mov	ds,ax
	mov	word [warm_boot],0	; indicate cold boot

warm_start:
	cli				; disable interrupts
	cld				; clear direction flag
	mov	al,e_cpu_test	
	call code_post		; POST start code
	

;-------------------------------------------------------------------------
; test CPU's FLAG register

	xor	ax,ax			; AX = 0
	jb	cpu_fail
	jo	cpu_fail
	js	cpu_fail
	jnz	cpu_fail
	jpo	cpu_fail
	add	ax,1			; AX = 1
	jz	cpu_fail
	jpe	cpu_fail
	sub	ax,8002h
	js	cpu_fail
	inc	ax
	jno	cpu_fail
	shl	ax,1
	jnb	cpu_fail
	jnz	cpu_fail
	shl	ax,1
	jb	cpu_fail

;-------------------------------------------------------------------------
; Test CPU registers

	mov	ax,0AAAAh
.1:
	mov	ds,ax
	mov	bx,ds
	mov	es,bx
	mov	cx,es
	mov	ss,cx
	mov	dx,ss
	mov	bp,dx
	mov	sp,bp
	mov	si,sp
	mov	di,si
	cmp	di,0AAAAh
	jnz	.2
	mov	ax,di
	not	ax
	jmp	.1
.2:
	cmp	di,5555h
	jz	cpu_ok

cpu_fail:
	mov	al,e_cpu_fail	
	call code_post

;-------------------------------------------------------------------------
; CPU error: continious beep - 400 Hz

	mov	al,0B6h
	out	pit_ctl_reg,al		; PIT - channel 2 mode 3
	mov	ax,pic_freq/400		; 400 Hz signal
	out	pit_ch2_reg,al
	mov	al,ah
	out	pit_ch2_reg,al
	in	al,ppi_pb_reg
	or	al,3			; turn speaker on and enable
	out	ppi_pb_reg,al		; PIT channel 2 to speaker

.1:
	hlt
	jmp	.1

;-------------------------------------------------------------------------
; CPU test passed

cpu_ok:

;-------------------------------------------------------------------------
; disable NMI, turbo mode, and video output on CGA and MDA

	mov	al,e_init_cfg	
	call code_post

%ifdef AT_NMI
	mov	al,0Dh & nmi_disa_mask
	out	rtc_addr_reg,al		; disable NMI
	jmp	$+2
	in	al,rtc_data_reg		; dummy read to keep RTC happy
%else
	mov	al,nmi_disable
	out	nmi_mask_reg,al		; disable NMI
%endif ; AT_NMI

%ifdef MACHINE_XI8088
	mov	al,iochk_disable	; clear and disable ~IOCHK
	out	ppi_pb_reg,al
	mov	al,00h			; clear turbo bit
	out	ppi_pb_reg,al		; and also turn off the speaker
%endif ; MACHINE_XI8088

%ifdef MACHINE_FE2010A
	mov	al,fe_par_disable	; Disable parity checking
	out	fe_config_reg,al	; FE2010A chipset configuration register
	mov	al,10110000b		; Clear keyboard, disable keyb clock
	out	ppi_pb_reg,al		; Disable parity and IOCHK
%endif ; MACHINE_FE2010A

%ifdef MACHINE_XT
	mov	al,ppi_cwd_value	; PPI port A and port C inputs
	out	ppi_cwd_reg,al		; PPI control word register
	mov	al,10100101b		; FIXME: Add documentation
	out	ppi_pb_reg,al
%endif ; MACHINE_XT

	mov	al,00h
	mov	dx,cga_mode_reg
	out	dx,al			; disable video output on CGA
	inc	al
	mov	dx,mda_mode_reg		; disable video output on MDA
	out	dx,al			; and set MDA high-resolution mode bit

;-------------------------------------------------------------------------
; Initialize DMAC (8237)
 
	mov	al,e_init_dmac	
	call code_post	
 	out	0Dh,al			; DMA Master Clear register - reset DMA
 	mov	al,40h			; single mode, verify, channel 0
 	out	dmac_mode_reg,al	; DMA Mode register
 	mov	al,41h			; single mode, verify, channel 1
 	out	dmac_mode_reg,al	; DMA Mode register
 	mov	al,42h			; single mode, verify, channel 2
 	out	dmac_mode_reg,al	; DMA Mode register
 	mov	al,43h			; single mode, verify, channel 3
 	out	dmac_mode_reg,al	; DMA Mode register
 	mov	al,0			; DMA Command register bits:
 					; DACK active low, DREQ active high,
 					; late write, fixed priority,
 					; normal timing, controller enable
 					; channel 0 addr hold disable
 					; memory to memory disable
 	out	08h,al			; DMA Command register
 	out	81h,al			; DMA Page, channel 2
 	out	82h,al			; DMA Page, channel 3
 	out	83h,al			; DMA Page, channels 0,1

;-------------------------------------------------------------------------
; Test first 32 KiB (MIN_RAM_SIZE) of RAM

	mov	al,e_low_ram_test	
	call code_post	
	xor	si,si
	xor	di,di
	mov	ds,di
	mov	es,di
	mov	dx,word [warm_boot+biosdseg*16] ; save soft reset flag to DX
	mov	ax,55AAh		; first test pattern
	mov	cx,MIN_RAM_SIZE*512	; RAM size to test in words
    rep	stosw				; store test pattern
	mov	cx,MIN_RAM_SIZE*512	; RAM size to test in words
.1:
	lodsw
	cmp	ax,55AAh		; compare to the test pattern
	jne	low_ram_fail
	loop	.1
	xor	si,si
	xor	di,di
	mov	ax,0AA55h		; second test pattern
	mov	cx,MIN_RAM_SIZE*512	; RAM size to test in words
    rep stosw				; store test pattern
	mov	cx,MIN_RAM_SIZE*512	; RAM size to test in words
.2:
	lodsw
	cmp	ax,0AA55h		; compare to the test pattern
	jne	low_ram_fail
	loop	.2
	xor	di,di
	xor	ax,ax			; zero
	mov	cx,MIN_RAM_SIZE*512	; RAM size to test in words
    rep stosw				; zero the memory
	jmp	low_ram_ok		; test passed

low_ram_fail:
	mov	al,e_low_ram_fail	; test failed	
	call code_post

;-------------------------------------------------------------------------
;  Low memory error: beep - pause - beep - pause ... - 400 Hz

	mov	al,0B6h
	out	pit_ctl_reg,al		; PIT - channel 2 mode 3
	mov	ax,pic_freq/400		; 400 Hz signal
	out	pit_ch2_reg,al
	mov	al,ah
	out	pit_ch2_reg,al
	in	al,ppi_pb_reg
.1:
	or	al,3			; turn speaker on and enable
	out	ppi_pb_reg,al		; PIT channel 2 to speaker
	mov	cx,0
.2:
	nop
	loop	.2
	and	al,0FCh			; turn of speaker
	out	ppi_pb_reg,al
	mov	cx,0
.3:
	nop
	loop	.3
	jmp	.1

;-------------------------------------------------------------------------
; Low memory test passed

low_ram_ok:
	mov	word [warm_boot+biosdseg*16],dx ; restore soft reset flag

;-------------------------------------------------------------------------
; Set up stack - using upper 256 bytes of interrupt table

	mov	ax,0030h
	mov	ss,ax
	mov	sp,0100h

;-------------------------------------------------------------------------
; Initialize interrupt table

	mov     al,e_int_table	
	call code_post
	push	cs
	pop	ds
	xor	di,di
	mov	es,di
	mov	si,interrupt_table
	mov	cx,0020h		; 32 Interrupt vectors
	mov	ax,bioscseg
.1:
	movsw				; copy ISR address (offset part)
	stosw				; store segment part
	loop	.1
%ifdef SECOND_PIC
	mov	di,70h*4		; starting from IRQ 70
	mov	si,interrupt_table2
	mov	cx,8			; 8 Interrupt vectors
.2:
	movsw				; copy ISR address (offset part)
	stosw				; store segment part
	loop	.2
%endif ; SECOND_PIC

;-------------------------------------------------------------------------
; set DS to BIOS data area

	mov	ax,biosdseg		; DS = BIOS data area
	mov	ds,ax

;-------------------------------------------------------------------------
; Initialize PIT (8254 timer)

	mov	al,36h			; channel 0, LSB & MSB, mode 3, binary
	out	pit_ctl_reg,al
	mov	al,0
	out	pit_ch0_reg,al
	out	pit_ch0_reg,al
	mov	al,54h			; channel 1, LSB only, mode 2, binary
	out	pit_ctl_reg,al		; used for DRAM refresh on IBM PC/XT/AT
	mov	al,12h			; or for delays (using port_b, bit 4)
	out	pit_ch1_reg,al		; pulse every 15ms
	mov	al,40h			; channel 1, counter latch
	out	pit_ctl_reg,al		; FIXME - not needed?

;-------------------------------------------------------------------------
; Play "power on" sound - also tests PIT functionality

	mov     al,e_pit_init	
	call code_post
	call	sound

;-------------------------------------------------------------------------
; Initialize PIC (8259)

	mov	al,e_pic_init	
	call code_post
%ifdef SECOND_PIC
	mov	al,11h			; ICW1 - edge triggered, cascade, ICW4
	out	pic1_reg0,al
	out	pic2_reg0,al
	mov	al,8			; ICW2 - interrupt vector offset = 8
	out	pic1_reg1,al
	mov	al,70h			; ICW2 - interrupt vector offset = 70h
	out	pic2_reg1,al
	mov	al,4			; ICW3 - slave is connected to IR2
	out	pic1_reg1,al
	mov	al,2			; ICW3 - slave ID = 2 (IR2)
	out	pic2_reg1,al
	mov	al,1			; ICW4 - 8086/8088
	out	pic1_reg1,al
	out	pic2_reg1,al
%else
	mov	al,13h			; ICW1 - edge triggered, single, ICW4
	out	pic1_reg0,al
	mov	al,8			; ICW2 - interrupt vector offset = 8
	out	pic1_reg1,al
	mov	al,9			; ICW4 - buffered mode, 8086/8088
	out	pic1_reg1,al
%endif ; SECOND_PIC

;-------------------------------------------------------------------------
; initialize keyboard controller (8242), keyboard and PS/2 auxiliary device

	mov	al,e_kbd_init	
	call code_post
%ifdef AT_KEYBOARD
	call	kbc_init
%else ; AT_KEYBOARD
	in	al,ppi_pb_reg
	and	al,00111111b		; set keyboard clock low
	out	ppi_pb_reg,al
	mov	cx,10582		; hold clock low for 20 ms
.kbd_reset_wait:
	loop	.kbd_reset_wait
	or	al,11000000b		; set keyboard clear bit, enable clock
	out	ppi_pb_reg,al
	and	al,01111111b		; unset keyboard clear bit
	out	ppi_pb_reg,al
	mov	cx,1000
.kbd_flush:
	mov 	ah,01h
	int	16h
	jz	.kbd_no_key
	mov	ah,00h
	int	16h
.kbd_no_key:
	loop	.kbd_flush

%endif ; AT_KEYBOARD

	call	kbd_buffer_init		; setup keyboard buffer

;-------------------------------------------------------------------------
; enable interrupts

	mov	al,e_int_ena	
	call code_post	
%ifdef SECOND_PIC
	mov	al,0B8h		; OSW1: unmask timer, keyboard, IRQ2 and FDC
	out	pic1_reg1,al
%ifndef PS2_MOUSE
	mov	al,0FDh		; OSW1: unmask IRQ9
%else
	mov	al,0EDh		; OSW1: unmask IRQ9 and IRQ12
%endif ; PS2_MOUSE
	out	pic2_reg1,al
%else
	mov	al,0BCh		; OSW1: unmask timer, keyboard and FDC
	out	pic1_reg1,al
%endif ; SECOND_PIC
	sti

	mov	al,nmi_enable
	out	nmi_mask_reg,al	; enable NMIs

%ifdef MACHINE_FE2010A or MACHINE_XT
;-------------------------------------------------------------------------
; Read video mode switches into equipment_list
	in	al,ppi_pb_reg
%ifdef MACHINE_FE2010A
	and	al,0FDh		; clear switch select bit - select SW5-SW8
%endif ; MACHINE_FE2010A
%ifdef MACHINE_XT
	and	al,0F7h		; clear switch select bit - select SW5-SW8
%endif ; MACHINE_XT
	out	ppi_pb_reg,al
	in	al,ppi_pc_reg	; read switches SW5-SW8
	and	al,03h		; video mode is in SW5 and SW6
	mov	cl,4
	shl	al,cl		; move video mode to bits 5-4
	or	[equipment_list],al
%endif ; MACHINE_FE2010A or MACHINE_XT
; 
;-------------------------------------------------------------------------
; look for video BIOS, initialize it if present

	mov	al,e_vid_bios_scan	
	call code_post
	mov	dx,0C000h
	mov	bx,0C800h
	call	extension_scan
	cmp	word [67h],0
	jz	.no_video_bios
	mov	al,e_vid_bios_init	
	call code_post
	call	far [67h]
	mov	ax,biosdseg		; DS = BIOS data area
	mov	ds,ax
; set video bits to 00 - EGA or later (Video adapter with BIOS)		
	and	word [equipment_list],~equip_video
	jmp	.video_initialized

.no_video_bios:
	mov	al,e_vid_no_bios	
	call code_post
	mov	ah,byte [equipment_list] ; get equipment - low byte
	and	ah,equip_video		; get video adapter type
	mov	al,07h			; monochrome 80x25 mode
	cmp	ah,equip_mono		; monochrome?
	jz	.set_mode
	mov	al,03h			; color 80x25 mode
	cmp	ah,equip_color_80	; 80x25 color?
	jz	.set_mode
	mov	al,01h			; color 40x24 mode

.set_mode:
	mov	ah,00h			; INT 10, AH=00 - Set video mode
	int	10h

.video_initialized:

;-------------------------------------------------------------------------
; print the copyright message

	mov	si,msg_copyright
	call	print

%ifdef AT_RTC

;-------------------------------------------------------------------------
; Initialize RTC / NVRAM
; Read equipment byte from CMOS and set it in BIOS data area
	call	rtc_init

%endif ; AT_RTC

%ifdef BIOS_SETUP
	mov	si,msg_setup		; print setup prompt
	call	print
%endif ; BIOS_SETUP


;-------------------------------------------------------------------------
; detect and print availability of various equipment

	call	detect_cpu		; detect and print CPU type
	call	detect_fpu		; detect and print FPU presence
%ifdef AT_RTC
	call	print_rtc		; print current RTC time
%endif ; AT_RTC
	call	print_display		; print display type
%ifdef PS2_MOUSE
	call	print_mouse		; print mouse presence
%endif ; PS2_MOUSE
	call	detect_serial		; detect serial ports and print findings
	call	detect_parallel		; detect parallel ports and print
					; findings
	call	detect_floppy		; detect floppy drive types
	call	print_floppy		; print floppy drive types

	call	detect_ram		; test RAM, get RAM size in AX

	mov	si,msg_ram_total
	call	print
	call	print_dec		; print RAM size
	mov	si,msg_kib
	call	print

; FE2010A - Configure chipset according to detected equipment
%ifdef MACHINE_FE2010A
; Write settings for emulated DIP switches
	mov	al,byte [equipment_list] ; switches are in low byte of equipment
	and	al,0CEh			; keep floppy, memory size, and FPU bits
	out	fe_control_reg,al
; Enable FPU NMI if needed, and lock chipset configuration
	and	al,equip_fpu		; enable NMI if the FPU is installed
	or	al,fe_config_lock	; set lock chipset configuration bit
	out	fe_config_reg,al
%endif ; MACHINE_FE2010A

%ifdef EBDA_SIZE
	call	reserve_ebda		; reserve EBDA if needed

	mov	si,msg_ram_avail
	call	print
	mov	ax,word [memory_size]
	call	print_dec		; print remaining RAM size
	mov	si,msg_kib
	call	print
%endif ; EBDA_SIZE

	call	detect_rom_ext		; detect and initialize extension ROMs

	jmp boot_os

;=========================================================================
; int_02 - NMI
; Note: Xi 8088 only implements IOCHK NMI, system board parity is not
;	implemented
;-------------------------------------------------------------------------
	setloc	0E2C3h			; NMI Entry Point
int_02:
	push	ax
%ifdef AT_NMI
	mov	al,0Dh & nmi_disa_mask
	call	rtc_read		; disable NMI
%else
	mov	al,nmi_disable
	out	nmi_mask_reg,al
%endif ; AT_NMI
	in	al,ppi_pb_reg		; read Port B
	mov	ah,al
	or	al,iochk_disable	; clear and disable ~IOCHK
	out	ppi_pb_reg,al
	test	al,iochk_status
	jnz	.iochk_nmi
	mov	al,ah
	out	ppi_pb_reg,al		; restore original bits
	jmp	.exit

.iochk_nmi:
	push	si
	mov	si,msg_iochk_nmi
	call	print
	pop	si
.1:
	mov	ah,0h
	int	16h
	or	al,20h			; convert to lower case
	cmp	al,'d'
	je	.exit			; leave NMIs disabled and exit
	cmp	al,'i'
	je	.ignore			; enable NMIs and exit
	cmp	al,'r'
	je	cold_start
	jmp	.1
.ignore:
%ifdef AT_NMI
	mov	al,0Dh | nmi_enable
	call	rtc_read		; enable NMI
%else
	mov	al,nmi_enable
	out	nmi_mask_reg,al
%endif ; AT_NMI
.exit:
	pop	ax
	iret

msg_iochk_nmi:
	db	"IOCHK NMI detected. Type 'i' to ignore, 'd' to disable NMIs, or 'r' to reboot."
	db	0Dh, 0Ah, 00h

;=========================================================================
; int_18 - execute ROM BASIC
; Note:
;	Prints an error message since we don't have ROM BASIC
;-------------------------------------------------------------------------
int_18:
	mov	si,msg_no_basic
	call	print
.1:
	hlt
	jmp	.1

;=========================================================================
; int_19 - load and execute the boot sector
;-------------------------------------------------------------------------
	setloc	0E6F2h			; INT 19 Entry Point
int_19:
	jmp	ipl

;=========================================================================
; configuration data table
;-------------------------------------------------------------------------
	setloc	0E6F5h
config_table:
	dw	.size			; bytes 0 and 1: size of the table
.bytes:
	db	MODEL_BYTE		; byte 2: model
	db	00h			; byte 3: submodel = 0
	db	00h			; byte 4: release = 0
%ifdef SECOND_PIC
%ifdef AT_RTC
	db	01110000b		; byte 5: feature byte 1
;		|||||||`-- system has dual bus (ISA and MCA)
;		||||||`-- bus is Micro Channel instead of ISA
;		|||||`-- extended BIOS area allocated (usually on top of RAM)
;		||||`-- wait for external event (INT 15h/AH=41h) supported
;		|||`-- INT 15h/AH=4Fh called upon INT 09h
;		||`-- real time clock installed
;		|`-- 2nd interrupt controller installed
;		`-- DMA channel 3 used by hard disk BIOS
%else ; AT_RTC
	db	01010000b		; byte 5: feature byte 1
;		|||||||`-- system has dual bus (ISA and MCA)
;		||||||`-- bus is Micro Channel instead of ISA
;		|||||`-- extended BIOS area allocated (usually on top of RAM)
;		||||`-- wait for external event (INT 15h/AH=41h) supported
;		|||`-- INT 15h/AH=4Fh called upon INT 09h
;		||`-- real time clock installed
;		|`-- 2nd interrupt controller installed
;		`-- DMA channel 3 used by hard disk BIOS
%endif ; AT_RTC
%else ; SECOND_PIC
%ifdef AT_RTC
	db	00100000b		; byte 5: feature byte 1
;		|||||||`-- system has dual bus (ISA and MCA)
;		||||||`-- bus is Micro Channel instead of ISA
;		|||||`-- extended BIOS area allocated (usually on top of RAM)
;		||||`-- wait for external event (INT 15h/AH=41h) supported
;		|||`-- INT 15h/AH=4Fh called upon INT 09h
;		||`-- real time clock installed
;		|`-- 2nd interrupt controller installed
;		`-- DMA channel 3 used by hard disk BIOS
%else ; AT_RTC
	db	00000000b		; byte 5: feature byte 1
;		|||||||`-- system has dual bus (ISA and MCA)
;		||||||`-- bus is Micro Channel instead of ISA
;		|||||`-- extended BIOS area allocated (usually on top of RAM)
;		||||`-- wait for external event (INT 15h/AH=41h) supported
;		|||`-- INT 15h/AH=4Fh called upon INT 09h
;		||`-- real time clock installed
;		|`-- 2nd interrupt controller installed
;		`-- DMA channel 3 used by hard disk BIOS
%endif ; AT_RTC
%endif ; SECOND_PIC
	db	00h			; byte 6: feature byte 2
	db	00h			; byte 7: feature byte 3
	db	00h			; byte 8: feature byte 4
	db	00h			; byte 9: feature byte 5
.size	equ	$-.bytes

;=========================================================================
; Includes with fixed entry points (for IBM compatibility)
;-------------------------------------------------------------------------

%include	"serial2.inc"		; INT 14 - BIOS Serial Communications
%include	"keyboard.inc"		; INT 16, INT 09
%include	"floppy2.inc"		; INT 13
%include	"printer2.inc"		; INT 17
%include	"video.inc"		; INT 10

;=========================================================================
; detect_rom_ext - Look for BIOS extensions, initialize if found
;-------------------------------------------------------------------------

detect_rom_ext:
	mov	al,e_ext_start		; ROM extension scan start	
	call code_post

	mov	dx,0C800h
	mov	bx,0F800h
%ifdef AT_RTC_NVRAM or FLASH_NVRAM
	call	get_config_a
	test	al,nvram_ext_scan
	jz	.ext_scan_loop		; ext_scan clear - scan till F8000
	mov	bx,0F000h		; ext_scan set - scan till F0000
%endif ; AT_RTC_NVRAM or FLASH_NVRAM

.ext_scan_loop:
	call	extension_scan
	cmp	word [67h],0
	jz	.ext_scan_done		; No ROM extension found
	mov	al,e_ext_detect		; ROM extension found	
	call code_post
	mov	si,msg_rom_found
	call	print
	mov	ax,word [69h]		; ROM extension's segment
	call	print_hex
	mov	si,msg_rom_init
	call	print
	push	bx
	push	dx
	call	far [67h]
	mov	ax,biosdseg		; DS = BIOS data area
	mov	ds,ax
	mov	al,e_ext_init_ok	; ROM extension initialized	
	call code_post
	pop	dx
	pop	bx
	jmp	.ext_scan_loop

.ext_scan_done:
	mov	al,e_ext_complete	; ROM extension scan complete	
	call code_post

	ret

;=========================================================================
; int_12 - Get memory size
; Input:
;	none
; Output:
;	AX = memory size
;-------------------------------------------------------------------------
	setloc	0F841h			; INT 12 Entry Point
int_12:
	sti
	push	ds
	mov	ax,biosdseg
	mov	ds,ax
	mov	ax,word [memory_size]
	pop	ds
	iret

;=========================================================================
; int_11 - Get equipment list
; Input:
;	none
; Output:
;	AX = equipment list
;-------------------------------------------------------------------------
	setloc	0F84Dh			; INT 11 Entry Point
int_11:
	sti
	push	ds
	mov	ax,biosdseg
	mov	ds,ax
	mov	ax,word [equipment_list]
	pop	ds
	iret

;=========================================================================
; Includes with fixed entry points (for IBM compatibility)
;-------------------------------------------------------------------------

%include	"misc.inc"

;=========================================================================
; extension_scan - scan for BIOS extensions
; Input:
;	DX - start segment
;	BX - end segment
; Returns:
;	DX - address for the continuation of the scan
;	biosdseg:67h - address of the extension, 0000:0000 if not found
;-------------------------------------------------------------------------
extension_scan:
	mov	word [67h],0
	mov	word [69h],0
.scan:
	mov	es,dx
    es	cmp	word [0],0AA55h		; check for signature
	jnz	.next			; no signature, check next 2 KiB
    es	mov	al,byte [2]		; AL = rom size in 512 byte blocks
	mov	ah,0
	mov	cl,5
	shl	ax,cl			; convert size to paragraphs
	add	dx,ax
	add	dx,007Fh		; round DX to the nearest 2 KiB
	and	dx,0FF80h		; (2 KiB = 128 x 16 bytes)
	mov	cl,4
	shl	ax,cl			; convert size to bytes
	mov	cx,ax
	mov	al,0
	xor	si,si
.checksum:
    es	add	al,byte [si]
	inc	si
	loop	.checksum
	or	al,al			; AL == 0?
	jnz	.next			; AL not zero - bad checksum
	mov	word [67h],3		; extension initialization offset
	mov	word [69h],es		; extension segment
	jmp	.exit
.next:
	add	dx,80h			; add 2 KiB
	cmp	dx,bx
	jb	.scan
.exit:
	ret

;=========================================================================
; ipl - Initial Program Load - try to read and execute boot sector
;-------------------------------------------------------------------------
ipl:
	sti
	xor	ax,ax
	mov	ds,ax
	mov	word [78h],int_1E	; set Floppy Parameters Table location
	mov	word [7Ah],cs

.boot_retry:
	mov	cx,4			; retry booting from floppy 4 times

.fd_read_retry:
	push	cx
	mov	ah,00h			; reset disk system
	mov	dl,00h			; drive 0
	int	13h
	jb	.fd_failed
	mov	ah,08h			; get drive parameters
	mov	dl,00h			; drive 0
	int	13h
	jc	.fd_failed
	cmp	dl,00h
	jz	.try_hdd		; jump if zero drives
	mov	ax,0201h		; read one sector
	xor	dx,dx			; head 0, drive 0
	mov	es,dx			; to 0000:7C00
	mov	bx,7C00h
	mov	cx,0001h		; track 0, sector 1
	int	13h
	jc	.fd_failed
	jmp	.check_signature	; read successful, check for boot sector

.fd_failed:
	pop	cx
	loop	.fd_read_retry

; try booting from HDD
.try_hdd:
	mov	ah,0Dh			; reset hard disks
	mov	dl,80h			; drive 80h
	int	13h
	jc	.boot_failed
	mov	ax,0201h		; read one sector
	mov	dx,0080h		; head 0, drive 80h
	xor	cx,cx
	mov	es,cx
	mov	bx,7C00h		; to 0000:7C00
	inc	cx			; CX == 0001h; track 0, sector 1
	int	13h
	jc	.boot_failed

.check_signature:
    es	cmp	word [7DFEh],0AA55h
	jnz	.boot_failed		; boot sector signature not found
	jmp	0000h:7C00h		; jump to the boot sector

.boot_failed:
	mov	si,msg_boot_failed
	call	print
	mov	ah,00h
	int	16h
	jmp	.boot_retry

;=========================================================================
; detect_ram - Determine the size of installed RAM and test it
; Input:
;	none
; Output:
;	AX = RAM size
;	CX, SI - trashed
;-------------------------------------------------------------------------
detect_ram:
	mov	al,e_ram_start		; RAM scan start	
	call code_post

	push	ds
	mov	cl,6			; for SHL - converting KiB to segment
	mov	ax,MIN_RAM_SIZE

.fill_loop:
	push	ax
	shl	ax,cl			; convert KiB to segment (mult. by 64)
	mov	ds,ax
	mov	word [RAM_TEST_BLOCK-2],ax
	pop	ax
	add	ax,RAM_TEST_BLOCK/1024
	cmp	ax,MAX_RAM_SIZE
	jne	.fill_loop
	mov	ax,MIN_RAM_SIZE

.size_loop:
	push	ax
	shl	ax,cl			; convert KiB to segment (mult. by 64)
	mov	ds,ax
	cmp	word [RAM_TEST_BLOCK-2],ax
	jne	.size_done
	pop	ax
	add	ax,RAM_TEST_BLOCK/1024
	cmp	ax,MAX_RAM_SIZE
	jnb	.size_exit
	jmp	.size_loop

.size_done:
	pop	ax

.size_exit:
	pop	ds
	mov	word [memory_size],ax	; store it for now... might change later

; AX = detected memory size, now test the RAM

	cmp	word [warm_boot],1234h	; warm boot - don't test RAM
	je	.test_done

	mov	si,msg_ram_testing
	call	print
	mov	ax,MIN_RAM_SIZE		; start from 32 KiB

.test_loop:
	push	ax
	mov	ah,03h			; INT 10h, AH=03h - get cursor position
	mov	bh,00h			; page 0
	int	10h			; position returned in DX
	pop	ax
	call	print_dec
	push	ax
	mov	ah,02h			; INT 10h, AH=02h - set cursor position
	mov	bh,00h			; page 0
	int	10h
	mov	ah,01h
	int	16h
	jz	.test_no_key
	mov	ah,00h
	int	16h			; read the keystroke
	cmp	al,1Bh			; ESC?
	je	.test_esc
	cmp	ax,3B00h		; F1?
	jne	.test_no_key
	or	byte [post_flags],post_setup

.test_no_key:
	pop	ax
	call	ram_test_block
	jc	.test_error		; error in last test
	add	ax,RAM_TEST_BLOCK/1024	; test the next block
	cmp	ax,word [memory_size]
	jb	.test_loop

	push	ax
	mov	al,e_ram_complete	; RAM scan complete	
	call code_post
	pop	ax

	jmp	.test_done

.test_esc:
	pop	ax
	mov	ax,word [memory_size]

	push	ax
	mov	al,e_ram_esc		; RAM scan canceled	
	call code_post
	pop	ax

	jmp	.test_done

.test_error:
	mov	word [memory_size],ax	; store size of good memory
	mov	si,msg_ram_error
	call	print
	call	print_dec
	mov	si,msg_kib
	call	print
	mov	si,msg_crlf
	call	print

	push	ax
	mov	al,e_ram_fail		; RAM scan failed	
	call code_post
	pop	ax

.test_done:
	ret

;=========================================================================
; ram_test_block - Test a 16 KiB (RAM_TEST_BLOCK) of RAM
; Input:
;	AX = address of the memory to test (in KiB)
; Output:
;	CF = status
;		0 = passed
;		1 = failed
;-------------------------------------------------------------------------
ram_test_block:
	push	ax
	push	bx
	push	cx
	push	si
	push	di
	push	ds
	push	es
	mov	cl,6			; convert KiB to segment address
	shl	ax,cl			; (multiply by 64)
	mov	ds,ax
	mov	es,ax
	xor	si,si
	xor	di,di
	mov	bx,RAM_TEST_BLOCK/2	; RAM test block size in words
	mov	ax,55AAh		; first test pattern
	mov	cx,bx
    rep	stosw				; store test pattern
	mov	cx,bx			; RAM test block size
.1:
	lodsw
	cmp	ax,55AAh		; compare to the test pattern
	jne	.fail
	loop	.1
	xor	si,si
	xor	di,di
	mov	ax,0AA55h		; second test pattern
	mov	cx,bx			; RAM test block size
    rep stosw				; store test pattern
	mov	cx,bx			; RAM test block size
.2:
	lodsw
	cmp	ax,0AA55h		; compare to the test pattern
	jne	.fail
	loop	.2
	xor	di,di
	xor	ax,ax			; zero
	mov	cx,bx			; RAM test block size
    rep stosw				; zero the memory
	clc				; test passed, clear CF
	jmp	.exit

.fail:
	stc				; test failed, set CF
	
.exit:
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	cx
	pop	bx
	pop	ax
	ret

;=========================================================================
; Includes with fixed entry points (for IBM compatibility)
;-------------------------------------------------------------------------
%include	"fnt00-7F.inc"
%include	"time2.inc"

;=========================================================================
; int_ignore - signal end of interrupt to PIC if hardware interrupt, return
;-------------------------------------------------------------------------
	setloc	0FF23h			; Spurious IRQ Handler Entry Point
int_ignore:
	push	ax
	push	ds
	mov	ax,biosdseg
	mov	ds,ax
	mov	al,0Bh			; PIC OCW3 - read in-service register
	out	pic1_reg0,al
	nop
	in	al,pic1_reg0		; get IRQ number
	mov	ah,al
	or	al,al
	jnz	.1
	mov	ah,0FFh
	jmp	.2
.1:
	in	al,pic1_reg1		; clear the interrupt
	or	al,ah
	out	pic1_reg1,al
	mov	al,20h			; end of interrupt
	out	pic1_reg0,al		; signal end of interrupt
.2:
	mov	byte [last_irq],ah
	pop	ds
	pop	ax
	iret

;=========================================================================
; int_dummy - Dummy interrupt handler. Do nothing, return.
;-------------------------------------------------------------------------
	setloc	0FF53h			; Dummy Interrupt Handler
int_dummy:
	iret

;=========================================================================
; int_05 - BIOS Print Screen
;-------------------------------------------------------------------------
	setloc	0FF54h			; INT 05 (Print Screen) Entry Point
int_05:
	sti
	push	ax
	push	bx
	push	cx
	push	dx
	push	ds
	mov	ax,biosdseg
	mov	ds,ax			; DS = BIOS data segment
	cmp	byte [prt_scrn_flags],prt_scrn_run
	je	.exit			; print screen is already in progress
	mov	byte [prt_scrn_flags],prt_scrn_run
					; signal that print screen is running

	mov	ah,0Fh			; get video mode parameters
	int	10h			; returns number of columns in AH
					; and active display page in BH
	mov	cl,ah			; store number columns

	mov	ch,byte [video_rows]	; try getting number of rows
	or	ch,ch
	jz	.wrong_num_rows		; CH == 0, apparently not initialized

	inc	ch			; CH = number of rows (on EGA/VGA)
	cmp	ch,60			; 60 rows maximum (as far as I know)
	jbe	.get_cursor_pos

.wrong_num_rows:
	mov	ch,25			; assume 25 rows

.get_cursor_pos:
	mov	ah,03h			; get cursor position and size
	int	10h			; returns cursor position in DX
	push	dx			; save original position / DX in stack

	
	mov	ah,0Dh			; move to the next line
	call	.print_char
	jnz	.error
	mov	ah,0Ah
	call	.print_char
	jnz	.error

	mov 	dh,0			; start from the first row (0)

.row_loop:
	mov 	dl,0			; start from the first column (0)

.column_loop:
	mov	ah,02h
	int	10h			; set cursor position (position in DX)

	mov	ah,08h
	int	10h			; read character at cursor position

	cmp	al,20h			; control character?
	jae	.continue		; no, print it
	mov	al,20h			; print space instead

.continue:
	call	.print_char
	jnz	.error
	inc	dl
	cmp	dl,cl			; on the last column?
	jb	.column_loop		; print next column

	mov	ah,0Dh			; move to the next line
	call	.print_char
	jnz	.error
	mov	ah,0Ah
	call	.print_char
	jnz	.error

	inc	dh
	cmp	dh,ch			; on the last row?
	jb	.row_loop		; print next row

	mov	byte [prt_scrn_flags],prt_scrn_ready
					; ready for the next call

.restore_cursor:
	pop	dx			; DX = original cursor position
	mov	ah,02h
	int	10h			; set cursor position (position in DX)

.exit:
	pop	ds
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	iret

.error:
	mov	byte [prt_scrn_flags],prt_scrn_fail
					; signal failure
	jmp	.restore_cursor
	

.print_char:
	push	dx
	xor	dx,dx			; DX = 0 - first printer port
	mov	ah,00h			; INT 17h, AH=10h - print character
	int	17h
	pop	dx
	test	ah,25h			; ZF = 0 - no error
	ret

;=========================================================================
; start - at power up or reset execution starts here (F000:FFF0)
;-------------------------------------------------------------------------
        setloc	0FFF0h			; Power-On Entry Point
start:
        jmp     bioscseg:cold_start

	setloc	0FFF5h			; ROM Date in ASCII
	db	DATE			; BIOS release date MM/DD/YY
	db	20h

	setloc	0FFFEh			; System Model byte
	db	MODEL_BYTE
	db	0ffh
