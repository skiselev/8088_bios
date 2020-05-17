;========================================================================
; serial2.inc -  BIOS Serial Port Communication Services (part 2 of 2)
;       INT 14h, function AH=00h
;       INT 14h, function AH=03h
;       INT 14h, function AH=04h
;       INT 14h, function AH=05h
;	- see serial1.inc for other INT 14h functions
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
; int_14 - BIOS Serial Port Communication Services
;-------------------------------------------------------------------------
	setloc	0E729h			; INT 14 Entry Point
int_14:
	sti
	push	cx
	push	dx
	push	si
	push	ds
	push	bx
	mov	bx,biosdseg
	mov	ds,bx
	cmp	ah,.max/2
	jae	int_14_error		; invalid function number specified
	cmp	dx,num_serial
	jae	int_14_error		; invalid port number specified
	mov	si,serial_timeout	; serial port timeout setting in BDA
	add	si,dx			; [SI] = timeout for the selected port
	mov	bx,dx
	shl	bx,1
	mov	dx,word [equip_serial+bx] ; DX = serial port address
	or	dx,dx
	jz	int_14_error		; specified port is not installed
	mov	bh,0
	mov	bl,ah
	shl	bx,1
    cs	jmp	near [.dispatch+bx]
.dispatch:
	dw	int_14_fn00
	dw	int_14_fn01
	dw	int_14_fn02
	dw	int_14_fn03
	dw	int_14_fn04
	dw	int_14_fn05
.max	equ	$-.dispatch

int_14_error:
	xor	ax,ax

int_14_exit:
	pop	bx
	pop	ds
	pop	si
	pop	dx
	pop	cx
	iret

;=========================================================================
; int_14_fn00 - Initialize serial port
; Input:
;	AH = 0 - function 00h - initialize serial port
;	AL - initialization parameters
;		bit 1,0	= 10	- 7 data bits
;			= 11	- 8 data bits
;		bit 2	= 0	- 1 stop bit
;			= 1	- 2 stop bits
;		bit 3	= 0	- parity disable
;			= 1	- parity enable
;		bit 4	= 0	- odd parity (if parity enabled)
;			= 1	- even parity (if parity enabled)
;		bit 7-5 = 000	- 110 bps
;			= 001	- 150 bps
;			= 010	- 300 bps
;			= 011	- 600 bps
;			= 100	- 1200 bps
;			= 101	- 2400 bps
;			= 110	- 4800 bps
;			= 111	- 9600 bps
;	DX = serial port number (0-3)
; Output:
;	AL = modem status
;		bit 0	= 1	- delta clear to send
;		bit 1	= 1	- delta data set ready
;		bit 2	= 1	- trailing edge ring indicator
;		bit 3	= 1	- delta data carrier detect
;		bit 4	= 1	- clear to send
;		bit 5	= 1	- data set ready
;		bit 6	= 1	- ring indicator
;		bit 7	= 1	- data carrier detect
;	AH = line status
;		bit 0	= 1	- data ready
;		bit 1	= 1	- overrun error
;		bit 2	= 1	- parity error
;		bit 3	= 1	- framing error
;		bit 4	= 1	- break interrupt
;		bit 5	= 1	- transmitter holding register
;		bit 6	= 1	- transmitter empty
;		bit 7	= 1	- error in RCVR FIFO
;-------------------------------------------------------------------------
int_14_fn00:
	mov	ah,al			; save AL to AH
	add	dx,uart_lcr_reg		; DX = UART LCR address
	mov	al,uart_lcr_ldiv	; AL = LCR load divisor bit
	out	dx,al
	xor	bx,bx
	mov	bl,ah
	and	bl,0E0h			; get divisor bits from init params
	shr	bl,1
	shr	bl,1
	shr	bl,1
	shr	bl,1
    cs	mov	bx,word [uart_divisors+bx] ; get the divisor value
	sub	dx,uart_lcr_reg		; DX = UART base address
	mov	al,bl
	out	dx,al			; output divisor - low byte
	inc	dx			; DX = UART base address + 1
	mov	al,bh
	out	dx,al			; output divisor - high byte
	inc	dx
	inc	dx			; DX = UART LCR address
	mov	al,ah			; AL = initialization parameters
	and	al,(uart_lcr_wlen | uart_lcr_stop | uart_lcr_pen | uart_lcr_peven)
					; get control bits
	out	dx,al			; set LCR
	dec	dx
	dec	dx			; DX = UART IER address
					; OPTIMIZATION:
					; uart_ier_reg = uart_lcr_reg - 2
	mov	al,0
	out	dx,al			; disable interrupts
	dec	dx			; DX = UART base address
					; OPTIMIZATION:
					; uart_base = uart_ier_reg - 1
	jmp	int_14_fn03		; exit returning modem and line status

;=========================================================================
; int_14_fn03 - Return serial port status
; Input:
;	AH = 03h - function 03h - return serial port status
;	DX = serial port number (0-3)
; Output:
;	AL = modem status (see int_14_fn00 for complete description)
;	AH = line status (see int_14_fn00 for complete description)
;-------------------------------------------------------------------------
int_14_fn03:
	add	dx,uart_lsr_reg		; DX = UART LSR address
	in	al,dx
	mov	ah,al
	inc	dx			; DX = UART MSR address
	in	al,dx
	jmp	int_14_exit

;=========================================================================
; int_14_fn04 - Extended initialize serial port
; Input:
;	AH = 04h - function 04h - extended initialize serial port
;	AL - break status:
;		00h = no break
;		01h = break
;	BH - parity:
;		00h = no parity
;		01h = odd parity
;		02h = even parity
;		03h = stick parity odd
;		04h = stick parity even
;	BL - stop bits:
;		00h = 1 stop bit
;		01h = 2 stop bits (or 1.5 stop bits for 5 bit word length)
;	CH - word length:
;		00h = 5 bits
;		01h = 6 bits
;		02h = 7 bits
;		03h = 8 bits
;	CL - bps rate:
;		00h = 110 bps
;		01h = 150 bps
;		02h = 300 bps
;		03h = 600 bps
;		04h = 1200 bps
;		05h = 2400 bps
;		06h = 6000 bps
;		07h = 9600 bps
;		08h = 19200 bps
;		09h = 38400 bps
;		0Ah = 57600 bps
;		0Bh = 115200 bps
;	DX = serial port number (0-3)
; Output:
;	AL = modem status (see int_14_fn00 for complete description)
;	AH = line status (see int_14_fn00 for complete description)
;-------------------------------------------------------------------------
int_14_fn04:
	mov	ah,al			; save AL to AH
	cmp	cl,num_divisors
	jae	int_14_fn03		; invalid divisor value
					; exit returning modem and line status
	add	dx,uart_lcr_reg		; DX = UART LCR address
	mov	al,uart_lcr_ldiv	; AL = LCR load divisor bit
	out	dx,al
	xor	bx,bx
	mov	bl,cl			; get divisor number from parameters
	shl	bl,1			; index to the word table
    cs	mov	bx,word [uart_divisors+bx] ; get the divisor value
	sub	dx,uart_lcr_reg		; DX = UART base address
	mov	al,bl
	out	dx,al			; output divisor - low byte
	inc	dx			; DX = UART base address + 1
	mov	al,bh
	out	dx,al			; output divisor - high byte
	inc	dx
	inc	dx			; DX = UART LCR address

	and	ah,01h			; break setting (mask defined bits)
	mov	al,ah			; add it to AL (LCR setting)

	shl	al,1
	shl	al,1
	shl	al,1			; make place for parity settings
	
	pop	bx			; get original BX value from the stack
	push	bx			; push it back to the stack

	cmp	bh,1			; parity setting
	jbe	.set_parity		; no parity or odd parity
	dec	bh
	and	bh,03h			; get the valid bits only
	shl	bh,1
	inc	bh			; 010 -> 011, 011 -> 101, 100 -> 111

.set_parity:
	or	al,bh			; add it to AL (LCR setting)

	shl	al,1			; make place for stop bit setting
	
	and	bl,01h			; get stop bits (defined bit only)
	or	al,bl			; add it to AL (LCR setting)

	shl	al,1
	shl	al,1			; make place for word length setting

	and	ch,03h			; get word length (mask defined bits)
	or	al,ch			; add it to AL (LCR setting)

	out	dx,al			; set LCR
	dec	dx
	dec	dx			; DX = UART IER address
					; OPTIMIZATION:
					; uart_ier_reg = uart_lcr_reg - 2
	mov	al,0
	out	dx,al			; disable interrupts
	dec	dx			; DX = UART base address
					; OPTIMIZATION:
					; uart_base = uart_ier_reg - 1
	jmp	int_14_fn03		; exit returning modem and line status

;=========================================================================
; int_14_fn05 - Extended serial port control
; Input:
;	AH = 05h - function 05h - extended serial port control
;	AL - sub-function:
;		00h = read modem control register
;		01h = write modem control register
;			BL = modem control register
;	DX = serial port number (0-3)
; Output:
;	AX = status	
;	sub-function AL = 00h:
;		BL = modem control register
;-------------------------------------------------------------------------
int_14_fn05:
	and	al,01h			; sub-function (get the valid bit)
	jnz	int_14_fn05_01

;-------------------------------------------------------------------------
; sub-function 00h - read modem control register
; int_14_fn05_00:
	add	dx,uart_lcr_reg		; DX = UART MCR address
	in	al,dx
	pop	bx			; get BX from the stack
	mov	bl,al			; BL = MCR content
	push	bx			; put BX back to the stack
	sub	dx,uart_lcr_reg		; DX = UART base address
	jmp	int_14_fn03		; exit returning modem and line status

;-------------------------------------------------------------------------
; sub-function 01h - write modem control register
int_14_fn05_01:
	add	dx,uart_lcr_reg		; DX = UART MCR address
	pop	bx			; get BX from the stack
	push	bx			; put BX back to the stack
	mov	al,bl			; BL = new MCR content
	out	dx,al
	sub	dx,uart_lcr_reg		; DX = UART base address
	jmp	int_14_fn03		; exit returning modem and line status
