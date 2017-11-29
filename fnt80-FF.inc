;=========================================================================
; fnt80-FF.inc - Font for graphics modes (Cyrillic font)
;       Characters from 80h to 0FFh
;-------------------------------------------------------------------------
;
; Compiles with NASM 2.11.08, might work with other versions
;
; This font is borrowed from kbd package (alt-8x8)
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
int_1F:
	db	 1Eh,  36h,  66h,  66h,  7Eh,  66h,  66h,  00h
	db	 7Ch,  60h,  60h,  7Ch,  66h,  66h,  7Ch,  00h
	db	 7Ch,  66h,  66h,  7Ch,  66h,  66h,  7Ch,  00h
	db	 7Eh,  60h,  60h,  60h,  60h,  60h,  60h,  00h
	db	 38h,  6Ch,  6Ch,  6Ch,  6Ch,  6Ch, 0FEh, 0C6h
	db	 7Eh,  60h,  60h,  7Ch,  60h,  60h,  7Eh,  00h
	db	0DBh, 0DBh,  7Eh,  3Ch,  7Eh, 0DBh, 0DBh,  00h
	db	 3Ch,  66h,  06h,  1Ch,  06h,  66h,  3Ch,  00h
	db	 66h,  66h,  6Eh,  7Eh,  76h,  66h,  66h,  00h
	db	 3Ch,  66h,  6Eh,  7Eh,  76h,  66h,  66h,  00h
	db	 66h,  6Ch,  78h,  70h,  78h,  6Ch,  66h,  00h
	db	 1Eh,  36h,  66h,  66h,  66h,  66h,  66h,  00h
	db	0C6h, 0EEh, 0FEh, 0FEh, 0D6h, 0C6h, 0C6h,  00h
	db	 66h,  66h,  66h,  7Eh,  66h,  66h,  66h,  00h
	db	 3Ch,  66h,  66h,  66h,  66h,  66h,  3Ch,  00h
	db	 7Eh,  66h,  66h,  66h,  66h,  66h,  66h,  00h
	db	 7Ch,  66h,  66h,  66h,  7Ch,  60h,  60h,  00h
	db	 3Ch,  66h,  60h,  60h,  60h,  66h,  3Ch,  00h
	db	 7Eh,  18h,  18h,  18h,  18h,  18h,  18h,  00h
	db	 66h,  66h,  66h,  3Eh,  06h,  66h,  3Ch,  00h
	db	 7Eh, 0DBh, 0DBh, 0DBh,  7Eh,  18h,  18h,  00h
	db	 66h,  66h,  3Ch,  18h,  3Ch,  66h,  66h,  00h
	db	 66h,  66h,  66h,  66h,  66h,  66h,  7Fh,  03h
	db	 66h,  66h,  66h,  3Eh,  06h,  06h,  06h,  00h
	db	0DBh, 0DBh, 0DBh, 0DBh, 0DBh, 0DBh, 0FFh,  00h
	db	0DBh, 0DBh, 0DBh, 0DBh, 0DBh, 0DBh, 0FFh,  03h
	db	0E0h,  60h,  60h,  7Ch,  66h,  66h,  7Ch,  00h
	db	0C6h, 0C6h, 0C6h, 0F6h, 0DEh, 0DEh, 0F6h,  00h
	db	 60h,  60h,  60h,  7Ch,  66h,  66h,  7Ch,  00h
	db	 78h,  8Ch,  06h,  3Eh,  06h,  8Ch,  78h,  00h
	db	0CEh, 0DBh, 0DBh, 0FBh, 0DBh, 0DBh, 0CEh,  00h
	db	 3Eh,  66h,  66h,  66h,  3Eh,  36h,  66h,  00h
	db	 00h,  00h,  78h,  0Ch,  7Ch, 0CCh,  76h,  00h
	db	 00h,  3Ch,  60h,  3Ch,  66h,  66h,  3Ch,  00h
	db	 00h,  00h,  7Ch,  66h,  7Ch,  66h,  7Ch,  00h
	db	 00h,  00h,  7Eh,  60h,  60h,  60h,  60h,  00h
	db	 00h,  00h,  3Ch,  6Ch,  6Ch,  6Ch, 0FEh, 0C6h
	db	 00h,  00h,  3Ch,  66h,  7Eh,  60h,  3Ch,  00h
	db	 00h,  00h, 0DBh,  7Eh,  3Ch,  7Eh, 0DBh,  00h
	db	 00h,  00h,  3Ch,  66h,  0Ch,  66h,  3Ch,  00h
	db	 00h,  00h,  66h,  6Eh,  7Eh,  76h,  66h,  00h
	db	 00h,  18h,  66h,  6Eh,  7Eh,  76h,  66h,  00h
	db	 00h,  00h,  66h,  6Ch,  78h,  6Ch,  66h,  00h
	db	 00h,  00h,  1Eh,  36h,  66h,  66h,  66h,  00h
	db	 00h,  00h, 0C6h, 0FEh, 0FEh, 0D6h, 0C6h,  00h
	db	 00h,  00h,  66h,  66h,  7Eh,  66h,  66h,  00h
	db	 00h,  00h,  3Ch,  66h,  66h,  66h,  3Ch,  00h
	db	 00h,  00h,  7Eh,  66h,  66h,  66h,  66h,  00h
	db	 11h,  44h,  11h,  44h,  11h,  44h,  11h,  44h
	db	 55h, 0AAh,  55h, 0AAh,  55h, 0AAh,  55h, 0AAh
	db	0DDh,  77h, 0DDh,  77h, 0DDh,  77h, 0DDh,  77h
	db	 18h,  18h,  18h,  18h,  18h,  18h,  18h,  18h
	db	 18h,  18h,  18h, 0F8h,  18h,  18h,  18h,  18h
	db	 18h, 0F8h,  18h, 0F8h,  18h,  18h,  18h,  18h
	db	 36h,  36h,  36h, 0F6h,  36h,  36h,  36h,  36h
	db	 00h,  00h,  00h, 0FEh,  36h,  36h,  36h,  36h
	db	 00h, 0F8h,  18h, 0F8h,  18h,  18h,  18h,  18h
	db	 36h, 0F6h,  06h, 0F6h,  36h,  36h,  36h,  36h
	db	 36h,  36h,  36h,  36h,  36h,  36h,  36h,  36h
	db	 00h, 0FEh,  06h,  0F6h, 36h,  36h,  36h,  36h
	db	 36h, 0F6h,  06h, 0FEh,  00h,  00h,  00h,  00h
	db	 36h,  36h,  36h, 0FEh,  00h,  00h,  00h,  00h
	db	 18h, 0F8h,  18h, 0F8h,  00h,  00h,  00h,  00h
	db	 00h,  00h,  00h, 0F8h,  18h,  18h,  18h,  18h
	db	 18h,  18h,  18h,  1Fh,  00h,  00h,  00h,  00h
	db	 18h,  18h,  18h, 0FFh,  00h,  00h,  00h,  00h
	db	 00h,  00h,  00h, 0FFh,  18h,  18h,  18h,  18h
	db	 18h,  18h,  18h,  1Fh,  18h,  18h,  18h,  18h
	db	 00h,  00h,  00h, 0FFh,  00h,  00h,  00h,  00h
	db	 18h,  18h,  18h,  0FFh, 18h,  18h,  18h,  18h
	db	 18h,  1Fh,  18h,  1Fh,  18h,  18h,  18h,  18h
	db	 36h,  36h,  36h,  37h,  36h,  36h,  36h,  36h
	db	 36h,  37h,  30h,  3Fh,  00h,  00h,  00h,  00h
	db	 00h,  3Fh,  30h,  37h,  36h,  36h,  36h,  36h
	db	 36h, 0F7h,  00h, 0FFh,  00h,  00h,  00h,  00h
	db	 00h, 0FFh,  00h, 0F7h,  36h,  36h,  36h,  36h
	db	 36h,  37h,  30h,  37h,  36h,  36h,  36h,  36h
	db	 00h, 0FFh,  00h, 0FFh,  00h,  00h,  00h,  00h
	db	 36h, 0F7h,  00h, 0F7h,  36h,  36h,  36h,  36h
	db	 18h, 0FFh,  00h, 0FFh,  00h,  00h,  00h,  00h
	db	 36h,  36h,  36h, 0FFh,  00h,  00h,  00h,  00h
	db	 00h, 0FFh,  00h, 0FFh,  18h,  18h,  18h,  18h
	db	 00h,  00h,  00h, 0FFh,  36h,  36h,  36h,  36h
	db	 36h,  36h,  36h,  3Fh,  00h,  00h,  00h,  00h
	db	 18h,  1Fh,  18h,  1Fh,  00h,  00h,  00h,  00h
	db	 00h,  1Fh,  18h,  1Fh,  18h,  18h,  18h,  18h
	db	 00h,  00h,  00h,  3Fh,  36h,  36h,  36h,  36h
	db	 36h,  36h,  36h, 0FFh,  36h,  36h,  36h,  36h
	db	 18h, 0FFh,  18h, 0FFh,  18h,  18h,  18h,  18h
	db	 18h,  18h,  18h, 0F8h,  00h,  00h,  00h,  00h
	db	 00h,  00h,  00h,  1Fh,  18h,  18h,  18h,  18h
	db	0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh
	db	 00h,  00h,  00h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh
	db	0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h, 0F0h
	db	 0Fh,  0Fh,  0Fh,  0Fh,  0Fh,  0Fh,  0Fh,  0Fh
	db	0FFh, 0FFh, 0FFh,  00h,  00h,  00h,  00h,  00h
	db	 00h,  00h,  7Ch,  66h,  66h,  7Ch,  60h,  00h
	db	 00h,  00h,  3Ch,  66h,  60h,  66h,  3Ch,  00h
	db	 00h,  00h,  7Eh,  18h,  18h,  18h,  18h,  00h
	db	 00h,  00h,  66h,  66h,  3Eh,  06h,  3Ch,  00h
	db	 00h,  00h,  7Eh, 0DBh, 0DBh,  7Eh,  18h,  00h
	db	 00h,  00h,  66h,  3Ch,  18h,  3Ch,  66h,  00h
	db	 00h,  00h,  66h,  66h,  66h,  66h,  7Fh,  03h
	db	 00h,  00h,  66h,  66h,  3Eh,  06h,  06h,  00h
	db	 00h,  00h, 0DBh, 0DBh, 0DBh, 0DBh, 0FFh,  00h
	db	 00h,  00h, 0DBh, 0DBh, 0DBh, 0DBh, 0FFh,  03h
	db	 00h,  00h, 0E0h,  60h,  7Ch,  66h,  7Ch,  00h
	db	 00h,  00h, 0C6h, 0C6h, 0F6h, 0DEh, 0F6h,  00h
	db	 00h,  00h,  60h,  60h,  7Ch,  66h,  7Ch,  00h
	db	 00h,  00h,  7Ch,  06h,  3Eh,  06h,  7Ch,  00h
	db	 00h,  00h, 0CEh, 0DBh, 0FBh, 0DBh, 0CEh,  00h
	db	 00h,  00h,  3Eh,  66h,  3Eh,  36h,  66h,  00h
	db	 66h,  00h,  7Eh,  60h,  7Ch,  60h,  7Eh,  00h
	db	 24h,  00h,  3Ch,  66h,  7Eh,  60h,  3Ch,  00h
	db	 00h,  30h,  18h,  0Ch,  06h,  0Ch,  18h,  30h
	db	 00h,  0Ch,  18h,  30h,  60h,  30h,  18h,  0Ch
	db	 0Eh,  1Bh,  1Bh,  18h,  18h,  18h,  18h,  18h
	db	 18h,  18h,  18h,  18h,  18h, 0D8h, 0D8h,  70h
	db	 00h,  18h,  18h,  00h,  7Eh,  00h,  18h,  18h
	db	 00h,  76h, 0DCh,  00h,  76h, 0DCh,  00h,  00h
	db	 00h,  38h,  6Ch,  6Ch,  38h,  00h,  00h,  00h
	db	 00h,  00h,  00h,  00h,  18h,  00h,  00h,  00h
	db	 00h,  00h,  00h,  38h,  38h,  00h,  00h,  00h
	db	 03h,  02h,  06h,  04h, 0CCh,  68h,  38h,  10h
	db	 3Ch,  42h,  99h, 0A1h, 0A1h,  99h,  42h,  3Ch
	db	 30h,  48h,  10h,  20h,  78h,  00h,  00h,  00h
	db	 00h,  00h,  7Ch,  7Ch,  7Ch,  7Ch,  00h,  00h
	db	 00h,  00h,  00h,  00h,  00h,  42h,  7Eh,  00h
