;=========================================================================
; video.inc - BIOS video services
;       INT 10h, functions AH=00h to AH=0Fh
;-------------------------------------------------------------------------
;
; Compiles with NASM 2.07, might work with other versions
;
; This code is adopted from XT-clone BIOS by Anonymous
;
;=========================================================================

;-------------------------------------------------------------------------
; CRTC registers
crtc_cur_start	equ	0Ah		; CRTC cursor start line register
crtc_cur_end	equ	0Bh		; CRTC cursor end line register
crtc_offset_hi	equ	0Ch		; CRTC start address high register
crtc_offset_lo	equ	0Dh		; CRTC start address low register
crtc_cur_pos_hi	equ	0Eh		; CRCT cursor location high register
crtc_cur_pos_lo	equ	0Fh		; CRTC cursor location low register
crtc_pen_hi	equ	10h		; CRTC light pen position high byte
crtc_pen_lo	equ	11h		; CRTC light pen position low byte

;-------------------------------------------------------------------------
; control characters
bel	equ	07h
bs	equ	08h
lf	equ	0Ah
cr	equ	0Dh

	setloc	0F045h			; int 10 functions table

int_10_dispatch:
	dw	int_10_fn00		; Set video mode
	dw	int_10_fn01		; Set text mode cursor shape
	dw	int_10_fn02		; Set cursor position
	dw	int_10_fn03		; Get cursor position and shape
	dw	int_10_fn04		; Read light pen position
	dw	int_10_fn05		; Set active display page
	dw	int_10_fn06		; Scroll up window
	dw	int_10_fn07		; Scroll down window
	dw	int_10_fn08		; Read character and attribute
	dw	int_10_fn09		; Write character and attribute
	dw	int_10_fn0A		; Write character only
	dw	int_10_fn0B		; Set background color or palette
	dw	int_10_fn0C		; Write graphics pixel
	dw	int_10_fn0D		; Read graphics pixel
	dw	int_10_fn0E		; Teletype output
	dw	int_10_fn0F		; Get current video mode
int_10_num_func	equ ($-int_10_dispatch)/2


;-------------------------------------------------------------------------
; offsets for registers on stack

int_10_ax	equ	0
int_10_al	equ	int_10_ax
int_10_ah	equ	int_10_ax+1
int_10_bx	equ	int_10_ax+2
int_10_bl	equ	int_10_bx
int_10_bh	equ	int_10_bx+1
int_10_cx	equ	int_10_bx+2
int_10_ch	equ	int_10_cx+1
int_10_dx	equ	int_10_cx+2

;=========================================================================
; int_10 - BIOS video services
; Input:
;	AH - Function
;		00h - Set video mode
;		01h - Set text mode cursor shape
;		02h - Set cursor position
;		03h - Get cursor position and shape
;		04h - Read light pen position
;		05h - Select active display page
;		06h - Scroll up window
;		07h - Scroll down window
;		08h - Read character and attribute at cursor position
;		09h - Write character and attribute at cursor position
;		0Ah - Write character only at cursor position
;		0Bh -
;			BH = 00h - Set background/border color
;			BH = 01h - Set palette
;		0Ch - Write graphics pixel
;		0Dh - Read graphics pixel
;		0Eh - Teletype output
;		0Fh - Get current video mode
;-------------------------------------------------------------------------
	setloc	0F065h			; int 10 Entry Point
int_10:
	sti
	cld				;  ...strings auto-increment
	push	bp
	push	es
	push	ds
	push	si
	push	di
	push	dx
	push	cx
	push	bx
	push	ax
	mov	bx,biosdseg
	mov	ds,bx
	mov	bl,byte [video_mode]
	cmp	bl,07h			; check for monochrome mode
	mov	bx,0B800h		; assume CGA, BX = CGA video segment
	jb	.color			; it is CGA
	mov	bh,0B0h			; else MDA, BX = MDA video segment

.color:
	mov	es,bx			; load video segment to ES
	mov	bp,sp			;  ...start of stack frame
	cmp	ah,int_10_num_func	; dispatch table size
	jae	.exit			; invalid function
	mov	bh,0
	mov	bl,ah
	shl	bx,1
    cs	call	near [int_10_dispatch+bx]

.exit:
	pop	ax
	pop	bx
	pop	cx
	pop	dx
	pop	di
	pop	si
	pop	ds
	pop	es
	pop	bp
	iret


;=========================================================================
; int_1D - Video parameters tables
;-------------------------------------------------------------------------
	setloc	0F0A4h			; int 1D (MDA and CGA Video Parm Table)
int_1D:
	; CGA test 40x25 modes
	db	38h, 28h, 2Dh, 0Ah, 1Fh, 06h, 19h, 1Ch, 02h, 07h, 06h, 07h
	db	00h, 00h, 00h, 00h

	; CGA text 80x25 modes
	db	71h, 50h, 5Ah, 0Ah, 1Fh, 06h, 19h, 1Ch, 02h, 07h, 06h, 07h
	db	00h, 00h, 00h, 00h

	; CGA graphics modes
	db	38h, 28h, 2Dh, 0Ah, 7Fh, 06h, 64h, 70h, 02h, 01h, 06h, 07h
	db	00h, 00h, 00h, 00h

	; MDA text 80x25 mode
	db	61h, 50h, 52h, 0Fh, 19h, 06h, 19h, 19h, 02h, 0Dh, 0Bh, 0Ch
	db	00h, 00h, 00h, 00h

page_size:
	dw	0800h			; text 40x25 mode
	dw	1000h			; text 80x25 mode
	dw	4000h			; graphics modes
	dw	4000h

columns:
	db	40, 40			; modes 0, 1 - 40x25
	db	80, 80			; modes 2, 3 - 80x25
	db	40, 40			; modes 4, 5 - 40x25
	db	80, 80			; modes 6, 7 - 80x25

MODES	db	2Ch,28h,2Dh,29h,2Ah,2Eh,1Eh,29h ; Table of mode sets

TABMUL	db	00h,00h,10h,10h,20h,20h,20h,30h ; Table lookup for multiply

;=========================================================================
; int_10_fn00 - Set video mode
; Input:
;	AH = 00h
;	AL = video mode
;		00h - CGA - text 40x25, 16 shades of gray
;		01h - CGA - text 40x25, 16 colors
;		02h - CGA - text 80x25, 16 shades of gray
;		03h - CGA - text 80x25, 16 colors
;		04h - CGA - graphics 320x200, 4 colors
;		05h - CGA - graphics 320x200, 4 shades of gray
;		06h - CGA - graphics 640x200, monochrome
;		07h - MDA - text 80x25, monochrome
;-------------------------------------------------------------------------
int_10_fn00:
	mov	bl,byte [bp+int_10_al]	; BL = video mode
	mov	cx,0B800h		; video memory segment for CGA
	mov	dx,3D4h			; assume CGA mode
	mov	al,0
	cmp	bl,07h
	jb	.color			; jump if monochrome mode
	mov	ch,0B0h			; video memory segment for MDA
	mov	dl,0B4h 		; 3B4h - monochrome port XXX
	inc	al

.color:
	mov	es,cx			; ES = video memory segment
	mov	word [video_port],dx	; Save cur. CRTC display port
	add	dl,4
	out	dx,al			; Reset the video
	mov	byte [video_mode],bl	;  ...save cur. CRTC mode
	mov	bh,0
	push	bx
	push	es
	xor	ax,ax
	mov	es,ax			; load interrupt table segment to ES
    es	les	si,[1Dh*4]		; load INT 1Dh vector to ES:SI
    cs	mov	bl,byte [bx+TABMUL]	; Get bl for index into int_1D
	add	si,bx

    es	mov	cx,word [si+0Ah]	; cursor shape from INT 1Dh
	xchg	cl,ch			; convert to LSB format
	mov	word [video_cur_shape],cx ; store cursor shape

	mov	cx,10h			; Sixteen values to send

.setup_crt_loop:
    es	mov	al,byte [si]		; Value to send in si
	call	vid_crtc_writeb			;  ...send it
	inc	ah			;  ...bump count
	inc	si			;  ...point to next
	loop	.setup_crt_loop		;  ...loop until done
	pop	es
	xor	di,di
	call	vid_check_mode		; Set flags according to mode
	mov	cx,2000h		; video memory size for CGA
	xor	ax,ax			; fill word for graphics mode
	jb	.clear_screen		; jump if graphics mode
	jnz	.text_fill		; jump if CGA mode
	mov	cx,0800h 		; video memory size for MDA

.text_fill:
	mov	ax,07h << 8 | ' '	; fill word for test mode

.clear_screen:
	repz	stosw			; clear screen with fill word

	mov	dx,word [video_port]	; Get the port
	add	dl,4
	pop	bx
    cs	mov	al,byte [bx+MODES]	; Load data to set for mode
	out	dx,al			;  ...and send it
	mov	byte [video_mode_reg],al ;  ...then save active data
	inc	dx
	mov	al,30h			; Assume not 640 x 200 b/w
	cmp	bl,6			;  ...correct?
	jnz	.set_palette
	mov	al,3Fh			; Palette for 640 x 200 b/w

.set_palette:
	mov	byte [video_palet_reg],al ;  ...save palette
	out	dx,al			;  ...send palette

	mov	ax,ds
	mov	es,ax
	xor	ax,ax
	mov	byte [video_page],al	;  ...active page=page 0
	mov	cx,9			; video_page_offt + video_cur_pos * 8
	mov	di,video_page_offt
 	rep	stosw			; zero page offset and cursor position

    cs	mov	al,byte [bx+columns]	; Get display width
	mov	word [video_columns],ax	;  ...save it
	and	bl,0FEh			; XXX: this cleans up LSB
					; but it means that for mode 7
					; we'll get graphics page size?!
    cs	mov	ax,word [bx+page_size]	; Get video page size
	mov	word [video_page_size],ax ;  ...save it
	ret

;=========================================================================
; int_10_fn01 - Set text-mode cursor shape
; Input:
;	AH = 01h
;	CH = cursor scan line start
;	CL = cursor scan line end
; Output:
;	none
;-------------------------------------------------------------------------
int_10_fn01:
	mov	word [video_cur_shape],cx ; save cursor shape to BIOS data area
	mov	ah,crtc_cur_start	; select CRTC cursor shape registers
	call	vid_crtc_writew		; write it to CRTC
	ret

;=========================================================================
; int_10_fn02 - Set cursor position
; Input:
;	AH = 02h
;	BH = page number
;	DH = cursor row (00h is top)
;	DL = cursor column (00h is left)
; Output:
;	none
;-------------------------------------------------------------------------
int_10_fn02:
	mov	bl,byte [bp+int_10_bh]	; BL = page number
	cmp	byte [video_page],bl	; is it on current page?
	jne	bios_set_cur_pos	; if not visible only update BIOS data

;=========================================================================
; set_cur_pos - set CRTC cursor position, update BIOS cursor location
;	BL = page
;	DH = cursor row (00h is top)
;	DL = cursor column (00h is left)
;-------------------------------------------------------------------------
set_cur_pos:
	mov	ax,dx			; AX = cursor position
	call	vid_position_to_offset			; AX - offset
	add	ax,word [video_page_offt]	;  + byte offset, regen reg.
	shr	ax,1
	mov	cx,ax
	mov	ah,crtc_cur_pos_hi	; CRCT cursor location high register
	call	vid_crtc_writew		; send cursor position to CRTC
bios_set_cur_pos:
	mov	bh,0
	shl	bl,1			; index to words table
	mov	word [bx+video_cur_pos],dx ; save position to BIOS data area
	ret

;=========================================================================
; int_10_fn03 - Get cursor position and shape
; Input:
;	AH = 03h
;	BH = page number
; Output:
;	CH = cursor start scan line
;	CL = cursor end scan line
;	DH = cursor row (00h is top)
;	DL = cursor column (00h is left)
;-------------------------------------------------------------------------
int_10_fn03:
	mov	bh,0
	mov	bl,byte [bp+int_10_bh]	; BL = page number
	shl	bl,1
	mov	ax,word [bx+video_cur_pos] ; get current cursor position
	mov	word [bp+int_10_dx],ax	; return position in DX
	mov	ax,word [video_cur_shape]  ; get cursor shape
	mov	word [bp+int_10_cx],ax	; return cursor shape in CX
	ret

;=========================================================================
; int_10_fn04 - Read light pen position
; Input:
;	AH = 04h
; Output:
;	AH - light pen trigger flag
;		00h not down/triggered
;		01h down/triggered
;	If light pen is triggered:
;		DH = character row
;		DL = character column
;		CH = pixel row
;		BX = pixel column
;-------------------------------------------------------------------------
int_10_fn04:
	mov	byte [bp+int_10_ah],0	; set AH = 0, light pen not triggered
	mov	dx,word [video_port]
	add	dl,6			; CRTC status register
	in	al,dx			; read it
	test	al,4			; test light pen switch bit
	jz	.reset_pen		; reset pen and return if switch is off
	test	al,2			; test light pen tigger bit
	jnz	.read_pen		; continue if triggered
	ret				; not triggered - return

.read_pen:
	mov	dx,word [video_port]	; CRTC index register
	mov	al,crtc_pen_hi		; CRTC pen position high byte register
	out	dx,al			; select it
	inc	dx			; CRTC data register
	in	al,dx			; read high byte of pen position
	mov	ah,al
	dec	dx			; CRTC index register
	mov	al,crtc_pen_lo		; CRTC pen position low byte register
	out	dx,al			; select it
	inc	dx			; CRTC data register
	in	al,dx			; read low byte of pen position

	mov	bh,0
	mov	bl,byte [video_mode]	; get current video mode
    cs	mov	bl,byte [bx+.correction] ; light pen correction factor
	sub	ax,bx
	jns	.1
	xor	ax,ax			; set to zero if negative result

.1:
	call	vid_check_mode			; check video mode
	jnb	.text			; calculate character position if text
	mov	dl,40			; divide by 40
	div	dl			; AL = row, AH = column (reminder)
	mov	bh,0
	mov	bl,ah
	mov	cl,3
	shl	bx,cl			; BX = AH * 8 - pixel column
	mov	ch,al
	shl	ch,1			; CH = AL * 2 - pixel row
	mov	dl,ah			; DL = AH - character column
	mov	dh,al
	shr	dh,1
	shr	dh,1			; DH = AL / 4 - character row
	cmp	byte [video_mode],6	; check for 640x200 mode
	jnz	.exit
	shl	bx,1			; adjust pixel column (double it)
	shl	dl,1			; same or character column
	jmp	.exit

.text:
	div	byte [video_columns] 	; divide by number of columns
	xchg	al,ah			; AL = column, AH = row
	mov	dx,ax			; save characer row,column to DH,AL
	mov	cl,3
	shl	ah,cl
	mov	ch,ah			; CH = AH * 8 - pixel row
	mov	bh,0
	mov	bl,al
	shl	bx,cl			; BX = AL * 8 - pixel column

.exit:
	mov	byte [bp+int_10_ah],1	; set AH = 1, light pen triggered
	mov	word [bp+int_10_dx],dx	;  ...row, column in user dx
	mov	word [bp+int_10_bx],bx	;  ...pixel column in user bx
	mov	byte [bp+int_10_ch],ch	;  ...raster line in user ch

.reset_pen:
	mov	dx,word [video_port]	; Get port of active CRTC card
	add	dl,7			; clear light pen strobe reg
	out	dx,al			; reset it
	ret

.correction:
	db	3, 3, 5, 5, 3, 3, 3, 4	; light pen correction

;=========================================================================
; int_10_fn05 - Select active display page
; Input:
;	AH = 05h
;	AL - new page number (00h is the first page)
; Output:
;	none
;-------------------------------------------------------------------------
int_10_fn05:
	mov	byte [video_page],al	; update page number in BIOS data area
	mov	bl,al			; also copy it to BL
	mov	ah,0
	mul	word [video_page_size]	; calculate page offset
	mov	word [video_page_offt],ax ; save the offset
	shr	ax,1			; calculate CRTC page start address
	mov	cx,ax			; save a copy to CX
	mov	ah,crtc_offset_hi	; CRTC start address high register
	call	vid_crtc_writew		; write new offset to CRTC

	mov	bh,0
	shl	bx,1
	mov	ax,word [bx+video_cur_pos] ; AX - cursor position for new page
	call	vid_position_to_offset			; AX - offset relative to start of page
	shr	ax,1
	add	cx,ax			; add to the page offset
	mov	ah,crtc_cur_pos_hi	; CRCT cursor location high register
	call	vid_crtc_writew		; send cursor position to CRTC
	ret

;=========================================================================
; int_10_fn06 - scroll up window
; int_10_fn07 - scroll down window
; Input:
;	AH = 06h (scroll up) or AH = 07 (scroll down)
;	AL = number of rows by which to scroll up (00h = clear entire window)
;	BH = attribute used to write blank rows at bottom of window
;	CH,CL = row,column of window's upper left corner
;	DH,DL = row,column of window's lower right corner
; Output:
;	none
; TODO:
;	optimize graphics fill
;-------------------------------------------------------------------------
int_10_fn06:
int_10_fn07:
	call	vid_check_mode
	jb	.graphics_scroll

	xor	si,si			; SI - "snow" workaround not required
	cmp	byte [video_mode],2
	jb	.no_snow
	cmp	byte [video_mode],3
	ja	.no_snow
	mov	si,5555h		; CGA "snow" workaround required
.no_snow:
	mov	ax,word [bp+int_10_dx]	; AX = window's lower right corner
	push	ax
	cmp	byte [bp+int_10_ah],07h	; check for scroll down function
	jz	.1			; jump if scroll down
	mov	ax,word [bp+int_10_cx]	; AX = window's upper left corner

.1:
	call	vid_position_to_offset
	add	ax,word [video_page_offt]
	mov	di,ax			; DI = scroll copy destination address

; calculate scroll window size (DX)

	pop	dx			; DX = window's lower right corner
	sub	dx,word [bp+int_10_cx]	; substract windows's upper left corner
	add	dx,101h 		; add 1x1

; calculate offset between the source and the destination (AX)

	mov	bx,word [video_columns]	; BX = columns (note BX <= 80)
	shl	bx,1			; each character takes two bytes
	mov	al,byte [bp+int_10_al]	; AL = number of rows to scroll
	push    dx
        mov     ah,0
        mul     bx
        pop     dx

	sub	bl,dl			; BX = distance between end of one
	sub	bl,dl			;   row and beggining of another
	push	ds
	mov	cx,es
	mov	ds,cx			; load video segment to DS
	cmp	byte [bp+int_10_ah],06h	; check for scroll up function
	jz	.2			; jump if scroll up
	neg	ax			; negate offset
	neg	bx			; negate distance
	std				; copy backwards

.2:
	mov	cl,byte [bp+int_10_al]	; CL = number of rows to scroll
	or	cl,cl
	jz	.text_fill		; jump if clear window only requested
	xchg	ax,si			; AX = snow workaround flag, SI = offset
	add	si,di			; SI = scroll copy source address
;XXX	sub	dh,byte [bp+int_10_al]	; DH = number of rows to copy
	sub	dh,cl			; DH = number of rows to copy

.text_scroll_loop:
	mov	ch,0
	mov	cl,dl			; CX = characters in row to copy
	ror	ax,1			; rotate snow workaround flag
	jnc	.text_scroll_no_retrace	; jump if LSB was zero - no wait
	call	.retrace_wait		; wait for vertical retrace

.text_scroll_no_retrace:
	repz	movsw			; copy one rows
	add	si,bx			; SI = next row to copy source address
	add	di,bx			; DI = next row to copy destination
	dec	dh			; decrement row counter
	jnz	.text_scroll_loop	; jump if there is more rows to copy

	mov	dh,byte [bp+int_10_al]	; DH = number of rows to fill
	mov	si,ax			; SI = snow workaround flag

.text_fill:
	mov	ch,0
	mov	ah,byte [bp+int_10_bh]	; AH = blank attribute
	mov	al,' '			; AL = blank character

.text_fill_loop:
	mov	cl,dl			; CX = characters in row to fill
	ror	si,1			; rotate snow workaround flag
	jnc	.text_fill_no_retrace	; jump if LSB was zero - no wait
	call	.retrace_wait		; wait for vertical retrace

.text_fill_no_retrace:
	repz	stosw			; fill one row
	add	di,bx			; DI = next row to fill destination
	dec	dh			; decrement row counter
	jnz	.text_fill_loop		; jump if there is more rows to fill

	pop	ds
	ret

;-------------------------------------------------------------------------
; .retrace_wait - next till the next vertical retrace

.retrace_wait:
	push	ax
	push	dx
	mov	dx,03DAh		; DX = CGA status register

.retrace_wait_not_set:
	in	al,dx
	test	al,08h			; bit 3 set if vertical retrace
	jnz	.retrace_wait_not_set	; jump if retrace

.retrace_wait_set:
	in	al,dx
	test	al,08h			; bit 3 set if vertical retrace
	jz	.retrace_wait_set	; jump if no retrace
	pop	dx
	pop	ax

.retrace_exit:
	ret

;-------------------------------------------------------------------------
; .graphics_scroll - scroll for graphics modes

.graphics_scroll:
	mov	ax,word [bp+int_10_dx]	; AX = window's lower right corner
	push	ax
	cmp	byte [bp+int_10_ah],07h ; check for scroll down function
	jz	.3			; jump if scroll down
	mov	ax,word [bp+int_10_cx]	; AX = window's upper left corner

.3:
	call	vid_gfx_pos_to_offset	
	mov	di,ax			; DI = scroll copy destination address

; calculate scroll windows size (DX)

	pop	dx			; DX = window's lower right corner
	sub	dx,word [bp+int_10_cx]	; substract window's upper left corner
	add	dx,101h 		; add 1x1
	shl	dh,1			; multiply by four: one character takes
	shl	dh,1			;   four bytes in each plane
	mov	al,byte [bp+int_10_ah]	; AL = function
	cmp	byte [video_mode],06h	; check for 640x200 mode
	jz	.4			; jump if 640x200 mode
	shl	dl,1			; double character width for 320x200 
	shl	di,1			; double character width for 320x200
	cmp	al,07h			; check for scroll down function
	jnz	.5			; jump if scroll down
	inc	di			; scroll up - adjust source address

.4:
	cmp	al,07h			; check for scroll down function
	jnz	.5			; jump if not scroll down
	add	di,0F0h			; adjust destination address
					;   for copying backwards

.5:
	mov	bl,byte [bp+int_10_al]	; BL = number of rows to scroll
	shl	bl,1			; multiply by four: one character takes
	shl	bl,1			;   four bytes in each plane
	push	bx
	sub	dh,bl			; DH = number of rows to copy
	mov	al,50h
	mul	bl
	mov	bx,1FB0h
	cmp	byte [bp+int_10_ah],06h	; check for scroll up function
	jz	.6			; jump if scroll up
	neg	ax			; negate offset for scroll down
	mov	bx,2050h
	std				; copy backwards

.6:
	mov	si,di
	add	si,ax			; SI = scroll copy source address
	pop	ax
	mov	cx,es
	mov	ds,cx			; load video segment to DS
	or	al,al
	jz	.graphics_fill		; jump if clear window only requested
	push	ax

.graphics_scroll_loop:
	mov	ch,0
	mov	cl,dl			; CX = bytes in row to copy
	push	si
	push	di
	repz	movsb			; copy one row in the first plane
	pop	di
	pop	si
	add	si,2000h		; point SI and DI to the second plane
	add	di,2000h
	mov	cl,dl			; CX = bytes in row to copy
	push	si
	push	di
	repz	movsb			; copy one row in the second plane
	pop	di
	pop	si
	sub	si,bx			; SI = next row to copy source address
	sub	di,bx			; DI = next row to copy destination
	dec	dh			; decrement row counter
	jnz	.graphics_scroll_loop	; jump if there is more rows to copy

	pop	ax
	mov	dh,al			; DH = number of rows to fill

.graphics_fill:
	mov	al,byte [bp+int_10_bh]	; AL = fill color
	mov	ch,0

.graphics_fill_loop:
	mov	cl,dl			; CX = bytes in row to fill
	push	di
	repz	stosb			; fill one row in the first plane
	pop	di
	add	di,2000h		; point DI to the second plane
	mov	cl,dl			; CX = bytes in row to fill
	push	di
	repz	stosb			; fill one row in the second plane
	pop	di
	sub	di,bx
	dec	dh			; decrement row counter
	jnz	.graphics_fill_loop	; jumpif there is more rows to fill
	ret

;=========================================================================
; int_10_fn08 - Read character and attribute
; Input:
;	AH = 08h
; Output:
;	AL - character read
;	BH - video attribute (text modes only)
; int_10_fn09 - Write character and attribute
; Input:
;	AH = 09h
;	AL - character to write
;	BH - page number
;	BL - attribute (text modes) or color (graphics modes)
;	CX - number of times to write character
; Output:
;	none
; int_10_fn0A - Write character only
; Input:
;	AH = 0Ah
;	AL - character to write
;	BH - page number
;	CX - repeat count
; Output:
;	none
;-------------------------------------------------------------------------
int_10_fn08:
int_10_fn09:
int_10_fn0A:
	call	vid_check_mode
	jb	.graphics		; jump if graphics mode
	mov	bl,byte [bp+int_10_bh]	; BL = page number
	mov	bh,0
	push	bx
	call	vid_current_offset
	mov	di,ax			; DI = character offset in the page
	pop	ax			; AX = page number
	mul	word [video_page_size] 	; AX = page number * page size
	add	di,ax			; DI = character offset
	mov	si,di			; SI = character offset
	mov	dx,word [video_port]	; DX = CRTC port
	add	dx,6			; DX = CGA status register
	push	ds
	mov	bx,es
	mov	ds,bx			; load video segment to DS
	mov	al,byte [bp+int_10_ah]	; AL = function
	cmp	al,08h			; check for read character function
	jnz	.text_write		; jump if not read char (write char)

.read_retrace_wait:
	in	al,dx
	test	al,01h			; bit 0 set if horizontal retrace
	jnz	.read_retrace_wait	; jump if retrace
	cli

.read_no_retrace_wait:
	in	al,dx
	test	al,01h			; bit 0 set if horizontal retrace
	jz	.read_no_retrace_wait	; jump if no retrace

	lodsw				; read character and attribute
	sti
	pop	ds
	mov	word [bp+int_10_ax],ax	; return character and attribute in AX
	ret

.text_write:
	mov	bl,byte [bp+int_10_al]	; BL = character to write
	mov	bh,byte [bp+int_10_bl]	; BH = attribute to write
	mov	cx,word [bp+int_10_cx]  ; CX = number of times to write char
	cmp	al,0Ah			; check for write char only function
	jz	.text_write_char_only	; jump if write char only

.write_char_retrace:
	in	al,dx
	test	al,08h			; bit 3 set if vertical retrace
	jnz	.do_write_char_attr	; retrace is in progress - write char

.write_retrace_wait1:
	in	al,dx
	test	al,01h			; bit 0 set if horizontal retrace
	jnz	.write_retrace_wait1	; jump if retrace
	cli

.write_no_retrace_wait1:
	in	al,dx
	test	al,01h			; bit 0 set if horizontal retrace
	jz	.write_no_retrace_wait1	; jump if no retrace

.do_write_char_attr:
	mov	ax,bx			; AX = character / attribute
	stosw				; write it to video memory
	sti
	loop	.write_char_retrace	; repeat CX times
	pop	ds
	ret

.text_write_char_only:
	in	al,dx
	test	al,08h			; bit 3 set if vertical retrace
	jnz	.do_write_char_only	; retrace is in progress - write char

.write_retrace_wait2:
	in	al,dx
	test	al,01h			; bit 0 set if horizontal retrace
	jnz	.write_retrace_wait2	; jump if retrace
	cli

.write_no_retrace_wait2:
	in	al,dx
	test	al,01h			; bit 0 set if horizontal retrace
	jz	.write_no_retrace_wait2	; jump if no retrace

.do_write_char_only:
	mov	al,bl			; AL = character to write
	stosb				; write it to video memory
	sti
	inc	di			; skip attribute
	loop	.text_write_char_only	; repeat CX times
	pop	ds
	ret

.graphics:
	cmp	byte [bp+int_10_ah],08h	; check for read character function
	jz	.graphics_read

	mov	ax,word [video_cur_pos]	; Get cursor position
	call	vid_gfx_pos_to_offset	;  ...convert (row,col) -> col
	mov	di,ax			; Save in displacement register
	push	ds
	mov	al,byte [bp+int_10_al]	; Get character to write
	mov	ah,0
	or	al,al			; Is it user character set?
	js	.CG9_02			;  ...skip if so
	mov	dx,cs			; Else use ROM character set
	mov	si,gfx_font		; load graphics font offset
	jmp	.CG9_03

.CG9_02:
	and	al,7Fh			; Origin to zero
	xor	bx,bx			;  ...then go load
	mov	ds,bx			;  ...user graphics
	lds	si,[7Ch]		;  ...vector, offset in si
	mov	dx,ds			;  ...segment into dx

.CG9_03:
	pop	ds			; Restore data segment
	mov	cl,3			;  ...char 8 pixels wide
	shl	ax,cl
	add	si,ax			; Add regen. buffer base addr.
	mov	cx,word [bp+int_10_cx]	;  ...load char. count
	cmp	byte [video_mode],6	; Is the mode 640 x 200 b/w?
	push	ds
	mov	ds,dx
	jz	.CG8_02			;  ...skip if so
	shl	di,1
	mov	al,byte [bp+int_10_bl]	; Get char. attribute
	and	ax,3
	mov	bx,5555h
	mul	bx
	mov	dx,ax
	mov	bl,byte [bp+int_10_bl]	; XXXX - why again

.CG9_04:
	mov	bh,8			; Char 8 pixels wide
	push	di
	push	si

.CG9_05:
	lodsb				; Read the screen
	push	cx
	push	bx
	xor	bx,bx
	mov	cx,8

.CG9_06:
	shr	al,1			; Shift bits thru byte
	rcr	bx,1
	sar	bx,1
	loop	.CG9_06

	mov	ax,bx			; Result into ax
	pop	bx
	pop	cx
	and	ax,dx
	xchg	ah,al
	or	bl,bl
	jns	.CG9_07
    es	xor	ax,word [di]

.CG9_07:
    es	mov	word [di],ax		; Write new word
	xor	di,2000h
	test	di,2000h		; Is this other plane?
	jnz	.CG9_08			;  ...nope
	add	di,50h			; Else advance character

.CG9_08:
	dec	bh			; Show another char written
	jnz	.CG9_05			;  ...more to go
	pop	si
	pop	di
	inc	di
	inc	di
	loop	.CG9_04
	pop	ds
	ret

.CG8_02:
	mov	bl,byte [bp+int_10_bl]	; Get display page
	mov	dx,2000h		;  ...size of graphics plane

.CG8_03:
	mov	bh,8			; Pixel count to write
	push	di
	push	si

.CG8_04:
	lodsb				; Read from one plane
	or	bl,bl			;  ...done both planes?
	jns	.CG8_05			;  ...skip if not
    es	xor	al,byte [di]		; Else load attribute

.CG8_05:
    es	mov	byte [di],al		; Write out attribute
	xor	di,dx			;  ...get other plane
	test	di,dx			; Done both planes?
	jnz	.CG8_06			;  ...skip if not
	add	di,50h			; Else position for now char

.CG8_06:
	dec	bh			; Show row of pixels read
	jnz	.CG8_04			;  ...not done all of them
	pop	si
	pop	di
	inc	di
	loop	.CG8_03
	pop	ds
	ret

.graphics_read:
	cld				; Increment upwards
	mov	ax,word [video_cur_pos]	;  ...get cursor position
	call	vid_gfx_pos_to_offset	; Convert (row,col) -> columns
	mov	si,ax			;  ...save in si
	sub	sp,8			; Grab 8 bytes temp storage
	mov	di,sp			;  ...save base in di
	cmp	byte [video_mode],6	; Mode 640 x 200 b/w?
	mov	ax,es
	push	ds
	push	di
	mov	ds,ax			; load video segment to DS
	jz	CGR_06			; Mode is 640 x 200 b/w - skip
	mov	dh,8			; Eight pixels high/char
	shl	si,1
	mov	bx,2000h		; Bytes per video plane

CGR_02:
	mov	ax,word [si] 		; Read existing word
	xchg	ah,al
	mov	cx,0C000h		; Attributes to scan for
	mov	dl,0

CGR_03:
	test	ax,cx			; Look for attributes
	clc
	jz	CGR_04			;  ...set, skip
	stc				; Else show not set

CGR_04:
	rcl	dl,1
	shr	cx,1
	shr	cx,1
	jnb	CGR_03			;  ...more shifts to go
    ss	mov	byte [di],dl
	inc	di
	xor	si,bx			; Do other video plane
	test	si,bx			;  ...done both planes?
	jnz	CGR_05			;  ...no, skip
	add	si,50h			; Else advance pointer

CGR_05:
	dec	dh			; Show another pixel row done
	jnz	CGR_02			;  ...more rows to do
	jmp	short	CGR_08

CGR_06:
	mov	dh,4			; Mode 640 x 200 b/w - special

CGR_07:
	mov	ah,byte [si] 		; Read pixels from one plane
    ss	mov	byte [di],ah		;  ...save on stack
	inc	di			;  ...advance
	mov	ah,byte [si+2000h]	; Read pixels from other plane
    ss	mov	byte [di],ah		; Save pixels on stack
	inc	di			;  ...advance
	add	si,50h			; Total pixels in char
	dec	dh			;  ...another row processed
	jnz	CGR_07			;  ...more to do

CGR_08:
	mov	dx,cs			; Load segment of graphics font 
	mov	di,gfx_font		;  ...and offset
	mov	es,dx			;  ...save offset in es
	mov	dx,ss
	mov	ds,dx
	pop	si
	mov	al,0

CGR_09:
	mov	dx,80h			; Number of char. in graphics set

CGR_10:
	push	si
	push	di
	mov	cx,8			; Bytes to compare for char
	repz	cmpsb			;  ...do compare
	pop	di
	pop	si
	jz	CGR_11			; Found graphics character
	inc	al			;  ...else show another char
	add	di,8			;  ...advance one row
	dec	dx			;  ...one less char  to scan
	jnz	CGR_10			; Loop if more char left

	or	al,al			; User graphics character set?
	jz	CGR_11			;  ...no, not found
	xor	bx,bx
	mov	ds,bx			; XXX - where ES is going
	les	di,[7Ch]		; Else load user graphics char
	mov	bx,es
	or	bx,di
	jz	CGR_11			;  ...not found
	jmp	short	CGR_09		; Try using user graphics char

CGR_11:
	mov	byte [bp+int_10_al],al	; Return char in user al
	pop	ds
	add	sp,8			;  ...return temp storage
	ret

;=========================================================================
; int_10_fn0B - Set background color or palette
; Input:
; 	AH - 0Bh
;	BH = 00h - set background / border color
;		BL - background (graphics modes) or border (text modes)
;	BH = 01h - set palette (320x200 graphics mode)
;		BL - palette ID:
;			00h - background, green, red, and yellow (brown)
;			01h - background, cyan, magenta, and white
; Output:
;	none
;-------------------------------------------------------------------------
int_10_fn0B:
	mov	al,byte [video_palet_reg] ; AL = current palette register
	mov	ah,byte [bp+int_10_bl]	; AH = color / palette ID
	cmp	byte [bp+int_10_bh],00h	; check function
	jnz	.set_palette		; jump to set palette if BH != 0

	and	al,0E0h			; clear color bits - bits 0-5
	and	ah,1Fh			; clear non-color bits in input
	or	al,ah			; apply new color
	jmp	.write_palet_reg

.set_palette:
	and	al,0DFh			; clear palette bit - bit 6
	test	ah,01h
	jz	.write_palet_reg
	or	al,20h			; set palette bit for BL = 01h

.write_palet_reg:
	mov	byte [video_palet_reg],al ; save new palette reg in BIOS data
	mov	dx,word [video_port]
	add	dx,5			; CRTC color select register
	out	dx,al			; send it to CRTC
	ret

;=========================================================================
; int_10_fn0C - Write graphics pixel
; Input:
;	AH = 0Ch
;	AL = pixel color, if bit 7 set, pixel is XOR'ed onto screen
;	CX = column
;	DX = row
; Output:
;	none
;-------------------------------------------------------------------------
int_10_fn0C:
	call	vid_pixel_address	; calculate pixel address	
	jnz	.mode_320x200		; jump if 320x200 mode
	mov	al,byte [bp+int_10_al]	; AL - color
	mov	bl,al			; copy color to BL
	and	al,1			; one bit per pixel
	ror	al,1			; make color MSB instead of LSB
	mov	ah,7Fh			; AH = pixel mask
	jmp	.prepare_mask

.mode_320x200:
	shl	cl,1
	mov	al,byte [bp+int_10_al]	; AL - color
	mov	bl,al			; copy color to BL
	and	al,3			; two bit per pixel
	ror	al,1			; make color MSB instead of LSB
	ror	al,1
	mov	ah,3Fh			; AH = pixel mask

.prepare_mask:
	ror	ah,cl			; position pixel mask correctly
	shr	al,cl			; position color bits correctly
    es	mov	cl,byte [si]		; read the byte containing the pixel
	or	bl,bl			; check if bit 7 set
	jns	.set_color		; bit 7 not set - new color
	xor	cl,al			; else XOR with existing color
	jmp	.write_pixel

.set_color:
	and	cl,ah			; clear existing color bits
	or	cl,al			; set new color bits

.write_pixel:
    es	mov	[si],cl			; write the byte with the new pixel
	ret

;=========================================================================
; int_10_fn0D - Read graphics pixel
; Input:
;	AH = 0Dh
;	CX = column
;	DX = row
; Output:
;	AL = pixel color 
;-------------------------------------------------------------------------
int_10_fn0D:
	call	vid_pixel_address	; calculate pixel address
    es	mov	al,byte [si]		; read byte containing the pixel
	jnz	.mode_320x200		; jump if 320x200 mode
	shl	al,cl			; shift pixel to bit 7
	rol	al,1			; shift pixel from bit 7 to bit 0
	and	al,1			; one bit per pixel
	jmp	.exit

.mode_320x200:
	shl	cl,1			; update position for two bits per pixel
	shl	al,cl			; shift pixel to bits 7-6
	rol	al,1			; shift pixel to bits 1-0
	rol	al,1
	and	al,3			; two bits per pixel

.exit:
	mov	byte [bp+int_10_al],al	; return pixel color in AL
	ret

;=========================================================================
; int_10_fn0E - Teletype output
; Input:
;	AH = 0Eh
;	AL = character to write
;	BL = foreground color (graphics modes only)
; Output:
;	none
; Notes:
;	- writes character to the active video page
;	- support following control characters: BEL, BS, LF, CR
;-------------------------------------------------------------------------
int_10_fn0E:
	mov	bl,byte [video_page]	; BL = active video page
	mov	bh,0
	shl	bl,1			; word index
	mov	dx,word [bx+video_cur_pos] ; DX = cursor position

	mov	al,byte [bp+int_10_al]	; AL = character to write
	cmp	al,bs
	jz	.bs			; jump if backspace (BS)
	cmp	al,lf
	jz	.lf			; jump if line feed (LF)
	cmp	al,bel
	jz	.bel			; jump if beep (BEL)
	cmp	al,cr
	jz	.cr			; jump if carriage return (CR)
	mov	bl,byte [bp+int_10_bl]	; BL = attribute for graphics mode
	mov	ah,0Ah			; INT 10h, function 0Ah - write char
	mov	cx,1			; one character
	int	10h			; write character
	inc	dl			; move cursor to the next column
	cmp	dl,byte [video_columns]	; compare position to number of columns
	jnz	.set_cursor_pos		; jump if not past the last column
	mov	dl,0			; move to the first position

.lf:
	cmp	dh,24			; on the last row?
	jz	.scroll			; jump if on the last row - scroll
	inc	dh			; move cursor to the next row
	jnz	.set_cursor_pos		; set new cursor position

.bs:
	cmp	dl,0			; on the first column?
	jz	.set_cursor_pos		; jump if yes - nothing to do
	dec	dl			; move cursor to the previous position
	jmp	.set_cursor_pos		; set new cursor position

.cr:
	mov	dl,0			; set cursor to the first column

.set_cursor_pos:
	mov	bl,byte [video_page]	; BL = active video page
	jmp	set_cur_pos		; set new cursor position

.bel:
	mov	bl,2			; 0.2 second beep
	call	beep
	ret

.scroll:
	mov	ah,02h
	int	10h			; set new cursor position
	call	vid_check_mode
	mov	bh,0
	jb	.do_scroll		; jump if text mode - do scroll
	mov	ah,08h			; INT 10h, function 08h - read char
	int	10h			; read attirbute at current position
	mov	bh,ah

.do_scroll:
	mov	ah,06h			; INT 10h, function 06h - Scroll up
	mov	al,1			; scroll one line
	xor	cx,cx			; top right corner is 0,0
	mov	dh,24			; bottom row is 24
	mov	dl,byte [video_columns] ; right column is the last column
	dec	dl
	int	10h			; scroll page up
	ret

;=========================================================================
; int_10_fn0F - Get current video mode
; Input:
;	AH = 0Fh
; Output:
;	AL = video mode
;	AH = characters per column
;	BH = active video page
;-------------------------------------------------------------------------
int_10_fn0F:
	mov	al,byte [video_columns]
	mov	byte [bp+int_10_ah],al
	mov	al,byte [video_mode]
	mov	byte [bp+int_10_al],al
	mov	al,byte [video_page]
	mov	byte [bp+int_10_bh],al
	ret

;=========================================================================
; vid_check_mode - Check current video mode
; Input:
;	none
; Output:
;	ZF set if monochrome mode (mode 07h)
;	CF set if graphics modes (modes 04h - 06h)
;-------------------------------------------------------------------------
vid_check_mode:
	push	ax
	mov	al,byte [video_mode]
	cmp	al,07h			; set ZF if monochrome mode
	jz	.exit			; jump if monochrome
	cmp	al,04h			; clears CF if graphics mode
	cmc				; invert CF flag (CF = 1 - graphics)
	jnb	.exit			; jump if not graphics
	sbb	al,al			; XXX - what for? clear ZF?
	stc				; XXX

.exit:
	pop	ax
	ret

;=========================================================================
; vid_crtc_writew - Write a word to two consecutive CRTC registers
; Input:
;	AH = register number
;	CX = word to write
; Output:
;	AX trashed
; Note:
;	Writes CH to register number AH, and CL to register number AH+1
;-------------------------------------------------------------------------
vid_crtc_writew:
	mov	al,ch
	call	vid_crtc_writeb		; write CH to CRTC register AH
	inc	ah			; point AH to the next register
	mov	al,cl			; prepare AL for vid_crtc_writeb

; fall through to vid_crtc_writeb (writting to AH+1)

;=========================================================================
; vid_crtc_writew - Write a word to two consecutive CRTC registers
; Input:
;	AH = register number
;	AL = byte to write
; Output:
;	none
;-------------------------------------------------------------------------
vid_crtc_writeb:
	push	dx
	mov	dx,word [video_port]	; DX = CRTC index port
	xchg	al,ah			; AH = byte, AL = register number
	out	dx,al			; write register number
	xchg	al,ah			; AH = register numbet, AL = byte
	inc	dl			; DX = CRTC data port
	out	dx,al			; write byte
	pop	dx
	ret

;=========================================================================
; vid_pixel_address - calculate pixel address and mask
; Input:
;	CX - column
;	DX - row
; Output:
;	SI - pixel address
;	CH - pixel mask
;	CL - pixel position in the byte
;	ZF - mode
;		0 = 320x200
;		1 = 640x200
;-------------------------------------------------------------------------
vid_pixel_address:
	xor	si,si			; SI = 0
	shr	dl,1			; divide row by two
	jnb	.even			; jump if on even row 
	mov	si,2000h		; odd row - second video plane

.even:
	mov	al,50h			; bytes in each row
	mul	dl			; AX - address of the row

	add	si,ax			; add row address to SI
	mov	dx,cx			; DX - column
	mov	cx,0302h 		; CH - pixel pos mask, CL - shift
	cmp	byte [video_mode],6	; 640x200 mode?
	pushf				; save ZF (and other flags
	jnz	.1			; skip if not 640x200
	mov	cx,0703h 		; pixel pos mask and shift for 640x200

.1:
	and	ch,dl			; CH = pixel position in the byte
	shr	dx,cl			; DX = address of the column
	add	si,dx			; add column address to SI
	xchg	cl,ch			; CH = pixel mask, CL = pixel position
	popf
	ret

;=========================================================================
; vid_current_offset - convert current cursor position to offset
;		       relative to page starting address
; Input:
;	BL = page
; Output:
;	AX = offset
;-------------------------------------------------------------------------
vid_current_offset:
	mov	bh,0
	shl	bx,1				; word index
	mov	ax,word [bx+video_cur_pos]	; AX = current cursor position

; fall through to vid_position_to_offset

;=========================================================================
; vid_position_to_offset - convert position (row and column) to offset
;			   relative to page starting address
; Input:
;	AH = row
;	AL = column
; Output:
;	AX = offset
;-------------------------------------------------------------------------
vid_position_to_offset:
	push	bx
	mov	bl,al			; BL = column
	mov	al,ah			; AL = row
	mul	byte [video_columns] 	; AX = row * video_columns
	mov	bh,0			;
	add	ax,bx			; AX = row * video_columns + column
	shl	ax,1			; multiply by two (char + attribute)
	pop	bx
	ret

;=========================================================================
; vid_gfx_pos_to_offset - convert position (row and column) to offset
; Input:
;	AH = row
;	AL = column
; Output:
;	AX = offset
;-------------------------------------------------------------------------
vid_gfx_pos_to_offset:
	push	bx
	mov	bl,al			; BL = column
	mov	al,ah			; AL = row
	mul	byte [video_columns] 	; AX = row * video_columns
	shl	ax,1			; multiply by four: one character takes
	shl	ax,1			;   four bytes in each plane
	mov	bh,0
	add	ax,bx			; AX = row * video_columns * 4 + column
	pop	bx
	ret
