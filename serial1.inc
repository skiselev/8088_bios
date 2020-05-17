;========================================================================
; serial1.inc -  BIOS Serial Port Communication Services (part 1 of 2)
;       INT 14h, function AH=01h
;	INT 14h, function AH=02h
;	detect_serial
;	serial port related definitions and tables
;	- see serial2.inc for other INT 14h functions
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

uart_osc	equ	1843200		; 1.8432 MHz is the default UART clock
one_sec_timeout equ	33135		; 30.18 us intervals in one second

uart_ier_reg	equ	1		; UART Interrupt Enable Register

uart_lcr_reg	equ	3		; UART Line Control Register
uart_lcr_ldiv	equ	80h		; LCR - load divisor bit
uart_lcr_wlen	equ	03h		; LCR - word length bits
uart_lcr_stop	equ	04h		; LCR - number of stop bits bit
uart_lcr_pen	equ	08h		; LCR - parity enable bit
uart_lcr_peven	equ	10h		; LCR - parity even bit

uart_mcr_reg	equ	4		; UART Modem Control Register
uart_mcr_dtr	equ	1		; MCR - data terminal ready (DTR)
uart_mcr_rts	equ	2		; MCR - request to send (RTS)

uart_lsr_reg	equ	5		; UART Line Status Register
uart_lsr_dr	equ	1		; LSR - data ready (DR)
uart_lsr_thre	equ	20h		; LSR - THR empty (THRE)

uart_msr_reg	equ	6		; UART Modem Status Register
uart_msr_cts	equ	10h		; MSR - clear to send (CTS)
uart_msr_dsr	equ	20h		; MSR - data set ready (DSR)

uart_err_timeout equ	80h		; timeout error

num_serial	equ	4		; number of serial ports

;=========================================================================
; int_14_fn01 - Send character
; Input:
;	AH = 01h - function 01h - send character
;	AL = character to send
;	DX = serial port number (0-3)
; Output:
;	AH = line status (see int_14_fn00 for complete description)
;-------------------------------------------------------------------------
int_14_fn01:
	push	ax
	mov	al,(uart_mcr_dtr | uart_mcr_rts) ; set DTR and RTS in MCR:
					; ready to communicate, request to send
	mov	bh,(uart_msr_dsr | uart_msr_cts) ; wait for DSR and CTS in MSR:
					; ready to communicate, clear to send
	mov	bl,uart_lsr_thre	; and for THRE in LSR:
					; UART is ready to accept new character
	call	uart_wait_status
	pop	cx
	jnz	int_14_timeout		; timeout, original AL is in CL
	mov	al,cl
	out	dx,al			; sent the character
	jmp	int_14_exit

int_14_timeout:
	mov	al,cl			; original AL saved in CL, restore it
	or	ah,uart_err_timeout	; set timeout bit
	jmp	int_14_exit

;=========================================================================
; int_14_fn02 - Receive character
; Input:
;	AH = 02h - function 02h - receive character
;	DX = serial port number (0-3)
; Output:
;	AL = character received
;	AH = line status (see int_14_fn00 for complete description)
;-------------------------------------------------------------------------
int_14_fn02:
	push	ax
	mov	al,uart_mcr_dtr		; set DTR in MCR:
					; ready to communicate
	mov	bh,uart_msr_dsr		; wait for DSR in MSR:
					; ready to communicate
	mov	bl,uart_lsr_dr		; and for DT in LSR:
					; UART has received a character
	call	uart_wait_status
	pop	cx
	jnz	int_14_timeout
	in	al,dx			; receive the character
	jmp	int_14_exit

;=========================================================================
; uart_wait_status - wait for modem and line status
; Input:
;	AL = value to write to MCR (Modem Control Register)
;	BH = mask to wait for in MSR (Modem Status Register)
;	BL = mask to wait for in LSR (Line Status Register)
;	DX = UART base address
;	SI = address of the timeout value (in seconds)
; Output:
;	AH = line status
;	ZF = 0 - timeout
;	CX,BX - destroyed
;-------------------------------------------------------------------------
uart_wait_status:
	push	dx
	add	dx,uart_mcr_reg		; DX = UART MCR address
	out	dx,al			; output MCR value
	inc	dx
	inc	dx			; DX = UART MSR address
					; OPTIMIZATION:
					; uart_msr_reg = uart_mcr_reg + 2
	call	wait_for_port		; wait for MSR status
	jnz	.wait_exit
	dec	dx			; DX = UART LSR address
					; OPTIMIZATION:
					; uart_lsr_reg = uart_msr_reg - 1
	mov	bh,bl			; BH = LSR mask
	call	wait_for_port		; wait for LSR status
.wait_exit:
	pop	dx
	ret

;=========================================================================
; wait_for_port - wait for set bits in a register
;
; Input:
;	BH = mask to wait for (bits that need to be 1)
;	DX = register address
;	SI = address of the timeout value (in seconds)
; Output:
;	AH = last port reading
;	ZF clear on timeout
;	ZF set on success
;	AL - trashed
;	CX - trashed
;-------------------------------------------------------------------------
wait_for_port:

	push	bx
	mov	bl,byte [SI]		; get the timeout value in seconds

.loop_outer:
	mov	cx,one_sec_timeout	; 30.18 us intervals in one second

.loop_inner:
	in	al,dx			; read the register
	mov	ah,al			; save register in AH
	and	al,bh
	cmp	al,bh			; required bits are set?
	je	.exit

%ifdef AT_DELAY

.zero_loop:
	in	al,ppi_pb_reg
	test	al,refresh_flag
	jz	.zero_loop		; wait 15.09 us

.one_loop:
	in	al,ppi_pb_reg
	test	al,refresh_flag
	jnz	.one_loop		; wait 15.09 us

%else ; AT_DELAY

	; wait 30.17 us (Intel 8088 CPU running on 4.77 MHz)
        mov     al,8
.delay_loop:
        dec     al
        jnz     .delay_loop

%endif ; AT_DELAY

	loop	.loop_inner

	dec	bl
	jnz	.loop_outer
	or	bh,bh			; timeout set ZF = 0 (BH != 0)

.exit:
	pop	bx
	ret

;=========================================================================
; detect_serial - detect and print serial ports
; Input:
;	none
; Output:
;	none (updates BIOS data area)
;-------------------------------------------------------------------------
detect_serial:
	push	ax
	push	bx
	push	dx
	push	si
	mov	al,e_serial_scan
	out	post_reg,al

	mov	si,msg_serial
	call	print
	mov	bx,0101h		; the default timeout is 1 sec
	mov	word [serial_timeout],bx   ; initialize timeout values
	mov	word [serial_timeout+2],bx ; for all 4 ports
	xor	bx,bx			; BX = 0
.detect_loop:
	mov	si,msg_serial_com
	call	print
	mov	al,bl
	shr	al,1
	inc	al			; AL = COM port number (1..4)
	call	print_digit
	mov	si,msg_colon
	call	print
    cs	mov	dx,word [uart_ier_regs+bx] ; DX = UART IER address
	in	al,dx
	mov	al,0Fh
	out	dx,al			; enable interrupts
	mov	al,00h
	out	unused_reg,al		; clear ISA data bus floating signals
	in	al,dx			; get interrupt enable register to AL
	cmp	al,0Fh
	jne	.no_port		; no port at this address
	mov	al,00h
	out	dx,al			; disable interrupts
	mov	al,0FFh
	out	unused_reg,al		; set ISA data bus floating signals
	in	al,dx			; get interrupt enable register to AL
	cmp	al,00h
	jne	.no_port		; no port at this address
	dec	dx			; DX = UART base address
					; OPTIMIZATION:
					; uart_base = uart_ier_reg - 1
	mov	word [equip_serial+bx],dx ; store port address to the BIOS
					; data area
	add	word [equipment_list],0200h ; increment number of serial ports
	mov	ax,dx
	call	print_hex
	jmp	.next

.no_port:
	mov	word [equip_serial+bx],0 ; no port
	mov	si,msg_none
	call	print
	
.next:
	inc	bx
	inc	bx
	cmp	bx,(num_serial << 1)	; all serial ports have been scanned?
	je	.done
	mov	si,msg_semicolon
	call	print
	jmp	.detect_loop

.done:
	mov	si,msg_crlf
	call	print

	pop	si
	pop	dx
	pop	bx
	pop	ax
	ret

;-------------------------------------------------------------------------
; serial port Interrupt Enable Register addresses - used for port detection
uart_ier_regs	dw	(3F8h + uart_ier_reg), (2F8h + uart_ier_reg)
		dw	(3E8h + uart_ier_reg), (2E8h + uart_ier_reg)

;-------------------------------------------------------------------------
; UART divisor values table
uart_divisors	dw	uart_osc/16/110		; 110 bps
		dw	uart_osc/16/150		; 150 bps
		dw	uart_osc/16/300		; 300 bps
		dw	uart_osc/16/600		; 600 bps
		dw	uart_osc/16/1200	; 1200 bps
		dw	uart_osc/16/2400	; 2400 bps
		dw	uart_osc/16/4800	; 4800 bps
		dw	uart_osc/16/9600	; 9600 bps
		dw	uart_osc/16/19200	; 19200 bps
		dw	uart_osc/16/38400	; 38400 bps
		dw	uart_osc/16/57600	; 57600 bps
		dw	uart_osc/16/115200	; 115200 bps
num_divisors	equ ($-uart_divisors)/2
