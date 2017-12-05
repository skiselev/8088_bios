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

flash_floppy	equ	8000h

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
	ret

;=========================================================================
; flash_save - Save configuration to Flash ROM
; Input:
;	CL - CMOS floppy drive type
;	     7-4 - first floppy drive type
;	     3-0 - second floppy drive type
;-------------------------------------------------------------------------

; TODO:
; 1. Copy itself to RAM
; 2. Call far to the code in RAM
; 3. Disable interrupts
; 4. Determine Flash type
; 5. Copy existing flash sector content to RAM
; 6. If needed - erase the sector with the configuration
; 7. Program new content to the sector or byte
; 8. Verify that it has been programmed successfully
; 9. Enable interrupts
; 10. Return far
flash_save:
	push	ax
	push	ds

	mov	ax,0F000h		; ROM segment
	mov	ds,ax
	
	pop	ds
	pop	ax
	ret
