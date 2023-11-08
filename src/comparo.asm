cpu 186
org 0x100


pic1_reg0	equ	20h
pic1_reg1	equ	21h
pit_ch0_reg	equ	40h
pit_ch1_reg	equ	41h
pit_ch2_reg	equ	42h
pit_ctl_reg	equ	43h


; How this works:
; We have the core of the clock calculation logic here, and you can trigger different code to be ran 100x with a "run (operation)" line.
; It runs the instructions 100x at 3WS and 0WS and times it.
; While there are differences for all instructions, the idea is we pick a pair that have the same difference (for example, MUL CL and AAM)
; because if we compare the runtimes of those two instructions that "same difference" will cancel out and effectively neutralize the wait-state factor.



%macro run 1+
mov dx, 0xFFF5
mov al, 0xFF
out dx, al
mov si, msg_3_wait
call print


cli				; Disable interrupts for better accuracy
call io_wait_latch;
push ax
%rep 100		; Must be unrolled or it behaves inconsistently on different waitstates IME
    %1
    %endrep
call io_wait_latch;
sti
pop bx
cmp ax, bx
jae %%second_scan_is_bigger_simple
sub bx, ax

jmp %%sub_done_simple
%%second_scan_is_bigger_simple:
mov cx, 0ffffh
sub cx, ax
add bx, cx
%%sub_done_simple:

push bx	;
mov ax, bx
call print_dec
mov si, endline
call print





mov si, msg_0_wait
call print


mov dx, 0xFFF5
xor al, al
out dx, al


cli				; Disable interrupts for better accuracy
call io_wait_latch;
push ax
%rep 100		; Must be unrolled or it behaves inconsistently on different waitstates IME
    %1
    %endrep
call io_wait_latch;
sti
pop bx
cmp ax, bx
jae %%second_scan_is_bigger_simple2
sub bx, ax


jmp %%sub_done_simple2
%%second_scan_is_bigger_simple2:
mov cx, 0ffffh
sub cx, ax
add bx, cx
%%sub_done_simple2:

mov ax, bx
call print_dec
mov si, endline
call print
mov si, difference
call print
pop ax
sub ax, bx
call print_dec
%endmacro

mov si, m_aaa
call print
run AAA
mov si, m_aam
call print
run AAM
mov si, m_aas
call print
run AAS
mov si, m_aad
call print
run AAD

mov si, m_mul
call print
run MUL CL

mov si, m_div
call print
run DIV CL

mov si, m_neg
call print
run NEG AX





int 0x20


msg_3_wait db "3-wait mode: ", 0
msg_0_wait db "   0-wait mode: ", 0
difference db "   Difference: ", 0
m_aaa db 0x0d, 0x0a, "AAA - ",0
m_aam db 0x0d, 0x0a, "AAM - ",0
m_aas db 0x0d, 0x0a, "AAS - ",0
m_aad db 0x0d, 0x0a, "AAD - ",0
m_mul db 0x0d, 0x0a, "MUL CL - ",0
m_div db 0x0d, 0x0a, "DIV CL - ",0
m_neg db 0x0d, 0x0a, "NEG AX - ",0
endline db 0x0d, 0x0a, 0;


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

io_wait_latch:
	mov	al,0		; counter 0, latch (00b)
	pushf			; save current IF
	cli			; disable interrupts
	out	pit_ctl_reg,al	; write command to ctc
	in	al,pit_ch0_reg	; read low byte of counter 0 latch
	mov	ah,al		; save it
	in	al,pit_ch0_reg	; read high byte of counter 0 latch
	popf			; restore IF state
	xchg	al,ah		; convert endian
	ret
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