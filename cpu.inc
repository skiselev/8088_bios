;=========================================================================
; cpu.inc - Detect and print CPU type and FPU presence
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

flags_tf	equ	0100h		; TF flag - enables single-step

;=========================================================================
; detect_cpu - detect and print CPU type
; Note:
;	- Designed to detect 8088, 80C88, and V20 CPU. Likely will return
;	  incorrect results if used on other CPU types.
;-------------------------------------------------------------------------
detect_cpu:
	push	ax
	push	si
	mov	al,e_cpu_detect
	out	post_reg,al

	mov	si,msg_cpu
	call	print

; test for NEC V20
	mov	ax,0101h
	aad	10h			; NEC V20 ignores the argument
	cmp	al,0Bh			; and always does AL = AL * 0Ah + AH
	je	.nec_v20

; not NEC V20, test for various 8088 versions
.i8088:					; 8088 - NMOS version of 8088
	cli				; disable interrupts
	push	ds
	xor	ax,ax
	mov	ds,ax			; interrupt table segment
	mov	ax,.int_01		; new int 01h offset
	xchg	ax,word [4]		; switch with original int 1 offset
	push	ax			; save to stack
	mov	ax,cs			; new int 01h segment
	xchg	ax,word [6]		; switch with original int 1 segment
	push	ax			; save to stack
	pushf				; push flags to stack (save)
	pushf				; push flags to stack (modify)
	pop	ax			; and move them to AX
	or	ax,flags_tf		; set TF
	push	ax
	xor	ax,ax			; AX = 0
	popf				; load modified value to FLAGS register
					; enable single-step
	nop
	push	ds			; Harris 80C88 disables interrupts
					; after PUSH SREG instructions
.push_sreg:				; and will not single-step to here
	pop	ds			; 1981 and newer 8088 disable interrupts
					; after modifying segment registers
.pop_sreg:				; and will not single-step to here
	nop
	popf				; restore original FLAGS, disable
					; single-step

	pop	si			; get original int 01h segment
	mov	word [6],si		; save it to interrupt table
	pop	si			; get original int 01h offset
	mov	word [4],si		; save it to interrupt table
	pop	ds
	sti				; enable interrupts
	cmp	ax,0001h		; if AX = 0001h - we have INTEL (C) 1978
	jz	.i8088_78

	cmp	ax,0002h
	jz	.i8088_81

.harris:				; Harris / Intersil 80C88
	mov	si,msg_cpu_harris
	call	print
	jmp	.cpu_freq

.i8088_78:
	mov	si,msg_cpu_8088_78
	call	print
	jmp	.cpu_freq

.i8088_81:
	mov	si,msg_cpu_8088_81
	call	print
	jmp	.cpu_freq

.nec_v20:
	mov	si,msg_cpu_nec_v20
	call	print

.cpu_freq:

%ifdef TURBO_MODE
; print CPU clock frequency

	mov	si,msg_at
	call	print

	push	ax
	call	get_config_a		; read BIOS configuration byte A
	call	print_cpu_clk
	pop	ax

%endif ; TURBO_MODE

	mov	si,msg_crlf
	call	print

	cmp	ax,0001h		; if AX = 0001h - we have INTEL (C) 1978
	jnz	.exit
	mov	si,msg_cpu_bug
	call	print

.exit:
	pop	si
	pop	ax
	ret

; stack frame after "push bp"
; BP - word [BP]
; IP - word [BP+2]
; CS - word [BP+4]
; FLAGS - word [BP+6]
.int_01:
	push	bp
	mov	bp,sp
	cmp	word [bp+2],.pop_sreg	; if IP = .pop_sreg we have '78 CPU
	jne	.1
	mov	al,01h			; (C) INTEL '79
	jmp	.int_01_exit
.1:
	cmp	word [bp+2],.push_sreg	; if IP = .push_sreg it is not Harris
	jne	.int_01_exit
	mov	al,02h			; not Harris / Intersil 80C88
.int_01_exit:
	pop	bp
	iret

;=========================================================================
; detect_fpu - detect and print FPU (mathematics coprocessor) presence
;-------------------------------------------------------------------------
test_word	equ	03FEh		; right at the end of interrupt table
detect_fpu:
	push	ax
	push	cx
	push	si
	mov	al,e_fpu_detect
	out	post_reg,al

	mov	si,msg_fpu
	call	print
	fninit				; initialize coprocessor
	mov	cx,3
.wait:
	loop	.wait			; wait for coprocessor to initialize
					;   (can't use fwait, since it could
					;   be no coprocessor at all)
	mov	cx,word [test_word]	; save test_word to CX
	mov	word [test_word],0000h	; clear space for control word
	fnstcw	word [test_word]	; store control word
	cmp	word [test_word],03FFh	; 8087 sets control word to 03FFh
	jne	.no_fpu			;   after finit

.fpu:
	mov	si,msg_fpu_present
	call	print
	or	word [equipment_list],equip_fpu ; update equipment word
	jmp	.exit

.no_fpu:

	mov	si,msg_absent
	call	print

.exit:
	mov	word [test_word],cx	; restore initial value of test_word

	pop	si
	pop	cx
	pop	ax
	ret

%ifdef TURBO_MODE
;=========================================================================
; print_cpu_clk - Print CPU clock frequency
; Input:
;	AL - NVRAM CPU clock frequency configuration (bits 1:0)
;		0 - 4.77 MHz / Normal
;		1 - 7.16 MHz / Turbo
;		2 - 9.55 MHz (FE2010A)
; Output:
;	none
;-------------------------------------------------------------------------
print_cpu_clk:
	push	ax
	push	si
	and	al,nvram_trbo_mask	; get turbo bits
%ifdef MACHINE_FE2010A
	mov	si,msg_clk_7_16mhz
	cmp	al,1
	je	.print
	mov	si,msg_clk_9_55mhz
	cmp	al,2
	je	.print
%endif ; MACHINE_FE2010A
%ifdef MACHINE_XI8088
	mov	si,msg_clk_turbo
	cmp	al,1
	je	.print
%endif ; MACHINE_XI8088
	mov	si,msg_clk_4_77mhz

.print:
	call    print
	pop	si
	pop	ax
	ret

;=========================================================================
; set_cpu_clk - Set CPU clock frequency
; Input:
;	AL - NVRAM CPU clock frequency configuration
;		0 - 4.77 MHz / Normal
;		1 - 7.16 MHz / Turbo
;		2 - 9.55 MHz (FE2010A)
; Output:
;	none
;-------------------------------------------------------------------------
set_cpu_clk:
%ifdef MACHINE_FE2010A
	cmp	al, 1 
	je	set_cpu_clk_7_16mhz
	cmp	al, 2
	je	set_cpu_clk_9_55mhz
	jmp	set_cpu_clk_4_77mhz   
%endif ; MACHINE_FE2010A
%ifdef MACHINE_XI8088
	cmp	al,1
	je	set_cpu_clk_turbo
	jmp	set_cpu_clk_4_77mhz   
%endif ; MACHINE_XI8088
	ret

;=========================================================================
; set_cpu_clk_4_77mhz - Set CPU clock at 4.77 MHz
; set_cpu_clk_7_16mhz - Set CPU clock at 7.16 MHz
; set_cpu_clk_9_55mhz - Set CPU clock at 9.55 MHz
; set_cpu_clk_turbo - Set CPU clock at turbo frequency (system dependent)
; Input:
;	none
; Output:
;	none
;-------------------------------------------------------------------------
%ifdef MACHINE_FE2010A
set_cpu_clk_4_77mhz:
	push ax
	mov	al,byte [equipment_list] ; get lower byte of the equipment list
	and	al,equip_fpu		; isolate FPU installed bit
	or	al,fe_par_disable	; disable memory parity checking
	out	fe_config_reg,al	; clear bits 7-5 - 4.77 MHz CPU clock
	pop ax
	ret

set_cpu_clk_7_16mhz:
	push ax
	mov	al,byte [equipment_list] ; get lower byte of the equipment list
	and	al,equip_fpu		; isolate FPU installed bit
	or	al,fe_par_disable	; disable memory parity checking
	or	al,fe_clk_7_16mhz	; set bit 6 - 7.16 MHz CPU clock
	out	fe_config_reg,al
	pop ax
	ret

set_cpu_clk_9_55mhz:
	push ax
	mov	al,byte [equipment_list] ; get lower byte of the equipment list
	and	al,equip_fpu		; isolate FPU installed bit
	or	al,fe_par_disable	; disable memory parity checking
	or	al,fe_clk_9_55mhz	; set bit 7 - 9.55 MHz CPU clock
	out	fe_config_reg,al
	pop ax
	ret
%endif ; MACHINE_FE2010A
%ifdef MACHINE_XI8088
set_cpu_clk_4_77mhz:
	push	ax
	in	al,ppi_pb_reg
	and	al,0FBh			; clear bit 2 (turbo enable bit)
	out	ppi_pb_reg,al
	pop	ax
	ret

set_cpu_clk_turbo:
	push	ax
	in	al,ppi_pb_reg
	or	al,04h			; set bit 2 (turbo enable bit)
	out	ppi_pb_reg,al
	pop	ax
	ret
%endif ; MACHINE_XI8088

%endif ; TURBO_MODE
