;=========================================================================
; fnt00-7F.inc - Font for graphics modes
;       Characters from 00h to 7Fh
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
	setloc	0FA6Eh			; IBM graphics char set entry
gfx_font:
	db	 00h, 00h,   00h,  00h,  00h,  00h,  00h,  00h
	db	 7Eh, 81h,  0A5h,  81h, 0BDh,  99h,  81h,  7Eh
	db	 7Eh, 0FFh, 0DBh, 0FFh, 0C3h, 0E7h, 0FFh,  7Eh
	db	 6Ch, 0FEh, 0FEh, 0FEh,  7Ch,  38h,  10h,  00h
	db	 10h,  38h,  7Ch, 0FEh,  7Ch,  38h,  10h,  00h
	db	 38h,  7Ch,  38h, 0FEh, 0FEh,  7Ch,  38h,  7Ch
	db	 10h,  10h,  38h,  7Ch, 0FEh,  7Ch,  38h,  7Ch
	db	 00h,  00h,  18h,  3Ch,  3Ch,  18h,  00h,  00h
	db	0FFh, 0FFh, 0E7h, 0C3h, 0C3h, 0E7h, 0FFh, 0FFh
	db	 00h,  3Ch,  66h,  42h,  42h,  66h,  3Ch,  00h
	db	0FFh, 0C3h,  99h, 0BDh, 0BDh,  99h, 0C3h, 0FFh
	db	 0Fh,  07h,  0Fh,  7Dh, 0CCh, 0CCh, 0CCh,  78h
	db	 3Ch,  66h,  66h,  66h,  3Ch,  18h,  7Eh,  18h
	db	 3Fh,  33h,  3Fh,  30h,  30h,  70h, 0F0h, 0E0h
	db	 7Fh,  63h,  7Fh,  63h,  63h,  67h, 0E6h, 0C0h
	db	 99h,  5Ah,  3Ch, 0E7h, 0E7h,  3Ch,  5Ah,  99h
	db	 80h, 0E0h, 0F8h, 0FEh, 0F8h, 0E0h,  80h,  00h
	db	 02h,  0Eh,  3Eh, 0FEh,  3Eh,  0Eh,  02h,  00h
	db	 18h,  3Ch,  7Eh,  18h,  18h,  7Eh,  3Ch,  18h
	db	 66h,  66h,  66h,  66h,  66h,  00h,  66h,  00h
	db	 7Fh, 0DBh, 0DBh,  7Bh,  1Bh,  1Bh,  1Bh,  00h
	db	 3Eh,  63h,  38h,  6Ch,  6Ch,  38h, 0CCh,  78h
	db	 00h,  00h,  00h,  00h,  7Eh,  7Eh,  7Eh,  00h
	db	 18h,  3Ch,  7Eh,  18h,  7Eh,  3Ch,  18h, 0FFh
	db	 18h,  3Ch,  7Eh,  18h,  18h,  18h,  18h,  00h
	db	 18h,  18h,  18h,  18h,  7Eh,  3Ch,  18h,  00h
	db	 00h,  18h,  0Ch, 0FEh,  0Ch,  18h,  00h,  00h
	db	 00h,  30h,  60h, 0FEh,  60h,  30h,  00h,  00h
	db	 00h,  00h, 0C0h, 0C0h, 0C0h, 0FEh,  00h,  00h
	db	 00h,  24h,  66h, 0FFh,  66h,  24h,  00h,  00h
	db	 00h,  18h,  3Ch,  7Eh,  0FFh, 0FFh, 00h,  00h
	db	 00h, 0FFh, 0FFh,  7Eh,  3Ch,  18h,  00h,  00h
	db	 00h,  00h,  00h,  00h,  00h,  00h,  00h,  00h
	db	 30h,  78h,  78h,  30h,  30h,  00h,  30h,  00h
	db	 6Ch,  6Ch,  6Ch,  00h,  00h,  00h,  00h,  00h
	db	 6Ch,  6Ch, 0FEh,  6Ch, 0FEh,  6Ch,  6Ch,  00h
	db	 30h,  7Ch, 0C0h,  78h,  0Ch, 0F8h,  30h,  00h
	db	 00h, 0C6h, 0CCh,  18h,  30h,  66h, 0C6h,  00h
	db	 38h,  6Ch,  38h,  76h, 0DCh, 0CCh,  76h,  00h
	db	 60h,  60h, 0C0h,  00h,  00h,  00h,  00h,  00h
	db	 18h,  30h,  60h,  60h,  60h,  30h,  18h,  00h
	db	 60h,  30h,  18h,  18h,  18h,  30h,  60h,  00h
	db	 00h,  66h,  3Ch, 0FFh,  3Ch,  66h,  00h,  00h
	db	 00h,  30h,  30h, 0FCh,  30h,  30h,  00h,  00h
	db	 00h,  00h,  00h,  00h,  00h,  30h,  30h,  60h
	db	 00h,  00h,  00h, 0FCh,  00h,  00h,  00h,  00h
	db	 00h,  00h,  00h,  00h,  00h,  30h,  30h,  00h
	db	 06h,  0Ch,  18h,  30h,  60h, 0C0h,  80h,  00h
	db	 7Ch, 0C6h, 0CEh, 0DEh, 0F6h, 0E6h,  7Ch,  00h
	db	 30h,  70h,  30h,  30h,  30h,  30h, 0FCh,  00h
	db	 78h, 0CCh,  0Ch,  38h,  60h, 0CCh, 0FCh,  00h
	db	 78h, 0CCh,  0Ch,  38h,  0Ch, 0CCh,  78h,  00h
	db	 1Ch,  3Ch,  6Ch, 0CCh, 0FEh,  0Ch,  1Eh,  00h
	db	0FCh, 0C0h, 0F8h,  0Ch,  0Ch, 0CCh,  78h,  00h
	db	 38h,  60h, 0C0h, 0F8h, 0CCh, 0CCh,  78h,  00h
	db	0FCh, 0CCh,  0Ch,  18h,  30h,  30h,  30h,  00h
	db	 78h, 0CCh, 0CCh,  78h, 0CCh, 0CCh,  78h,  00h
	db	 78h, 0CCh, 0CCh,  7Ch,  0Ch,  18h,  70h,  00h
	db	 00h,  30h,  30h,  00h,  00h,  30h,  30h,  00h
	db	 00h,  30h,  30h,  00h,  00h,  30h,  30h,  60h
	db	 18h,  30h,  60h, 0C0h,  60h,  30h,  18h,  00h
	db	 00h,  00h, 0FCh,  00h,  00h, 0FCh,  00h,  00h
	db	 60h,  30h,  18h,  0Ch,  18h,  30h,  60h,  00h
	db	 78h, 0CCh,  0Ch,  18h,  30h,  00h,  30h,  00h
	db	 7Ch, 0C6h, 0DEh, 0DEh, 0DEh, 0C0h,  78h,  00h
	db	 30h,  78h, 0CCh, 0CCh, 0FCh, 0CCh, 0CCh,  00h
	db	0FCh,  66h,  66h,  7Ch,  66h,  66h, 0FCh,  00h
	db	 3Ch,  66h, 0C0h, 0C0h, 0C0h,  66h,  3Ch,  00h
	db	0F8h,  6Ch,  66h,  66h,  66h,  6Ch, 0F8h,  00h
	db	0FEh,  62h,  68h,  78h,  68h,  62h, 0FEh,  00h
	db	0FEh,  62h,  68h,  78h,  68h,  60h, 0F0h,  00h
	db	 3Ch,  66h, 0C0h, 0C0h, 0CEh,  66h,  3Eh,  00h
	db	0CCh, 0CCh, 0CCh, 0FCh, 0CCh, 0CCh, 0CCh,  00h
	db	 78h,  30h,  30h,  30h,  30h,  30h,  78h,  00h
	db	 1Eh,  0Ch,  0Ch,  0Ch, 0CCh, 0CCh,  78h,  00h
	db	0E6h,  66h,  6Ch,  78h,  6Ch,  66h, 0E6h,  00h
	db	0F0h,  60h,  60h,  60h,  62h,  66h, 0FEh,  00h
	db	0C6h, 0EEh, 0FEh, 0FEh, 0D6h, 0C6h, 0C6h,  00h
	db	0C6h, 0E6h, 0F6h, 0DEh, 0CEh, 0C6h, 0C6h,  00h
	db	 38h,  6Ch, 0C6h, 0C6h, 0C6h,  6Ch,  38h,  00h
	db	0FCh,  66h,  66h,  7Ch,  60h,  60h, 0F0h,  00h
	db	 78h, 0CCh, 0CCh, 0CCh, 0DCh,  78h,  1Ch,  00h
	db	0FCh,  66h,  66h,  7Ch,  6Ch,  66h, 0E6h,  00h
	db	 78h, 0CCh, 0E0h,  70h,  1Ch, 0CCh,  78h,  00h
	db	0FCh, 0B4h,  30h,  30h,  30h,  30h,  78h,  00h
	db	0CCh, 0CCh, 0CCh, 0CCh, 0CCh, 0CCh, 0FCh,  00h
	db	0CCh, 0CCh, 0CCh, 0CCh, 0CCh,  78h,  30h,  00h
	db	0C6h, 0C6h, 0C6h, 0D6h, 0FEh, 0EEh, 0C6h,  00h
	db	0C6h,  44h,  6Ch,  38h,  38h,  6Ch, 0C6h,  00h
	db	0CCh, 0CCh, 0CCh,  78h,  30h,  30h,  78h,  00h
	db	0FEh, 0C6h,  8Ch,  18h,  32h,  66h, 0FEh,  00h
	db	 78h,  60h,  60h,  60h,  60h,  60h,  78h,  00h
	db	0C0h,  60h,  30h,  18h,  0Ch,  06h,  02h,  00h
	db	 78h,  18h,  18h,  18h,  18h,  18h,  78h,  00h
	db	 10h,  38h,  6Ch, 0C6h,  00h,  00h,  00h,  00h
	db	 00h,  00h,  00h,  00h,  00h,  00h,  00h, 0FFh
	db	 30h,  30h,  18h,  00h,  00h,  00h,  00h,  00h
	db	 00h,  00h,  78h,  0Ch,  7Ch, 0CCh,  7Ch,  00h
	db	 60h,  60h,  60h,  7Ch,  66h,  66h,  7Ch,  00h
	db	 00h,  00h,  78h, 0CCh, 0C0h, 0CCh,  78h,  00h
	db	 0Ch,  0Ch,  0Ch,  7Ch, 0CCh, 0CCh,  7Ch,  00h
	db	 00h,  00h,  78h, 0CCh, 0FCh, 0C0h,  78h,  00h
	db	 38h,  6Ch,  60h, 0F0h,  60h,  60h, 0F0h,  00h
	db	 00h,  00h,  7Ch, 0CCh, 0CCh,  7Ch,  0Ch, 0F8h
	db	 60h,  60h,  7Ch,  66h,  66h,  66h,  66h,  00h
	db	 30h,  00h,  70h,  30h,  30h,  30h,  78h,  00h
	db	 0Ch,  00h,  0Ch,  0Ch,  0Ch,  0Ch,  6Ch,  38h
	db	 60h,  60h,  66h,  6Ch,  78h,  6Ch,  66h,  00h
	db	 70h,  30h,  30h,  30h,  30h,  30h,  78h,  00h
	db	 00h,  00h, 0CCh, 0FEh, 0FEh, 0D6h, 0C6h,  00h
	db	 00h,  00h, 0F8h, 0CCh, 0CCh, 0CCh, 0CCh,  00h
	db	 00h,  00h,  78h, 0CCh, 0CCh, 0CCh,  78h,  00h
	db	 00h,  00h,  7Ch,  66h,  66h,  7Ch,  60h,  60h
	db	 00h,  00h,  7Ch, 0CCh, 0CCh,  7Ch,  0Ch,  0Ch
	db	 00h,  00h, 0DCh,  76h,  66h,  60h, 0F0h,  00h
	db	 00h,  00h,  7Ch, 0C0h,  78h,  0Ch, 0F8h,  00h
	db	 10h,  30h,  7Ch,  30h,  30h,  34h,  18h,  00h
	db	 00h,  00h, 0CCh, 0CCh, 0CCh, 0CCh,  7Ch,  00h
	db	 00h,  00h, 0CCh, 0CCh, 0CCh,  78h,  30h,  00h
	db	 00h,  00h, 0C6h, 0D6h, 0FEh, 0FEh,  6Ch,  00h
	db	 00h,  00h, 0C6h,  6Ch,  38h,  6Ch, 0C6h,  00h
	db	 00h,  00h, 0CCh, 0CCh, 0CCh,  7Ch,  0Ch, 0F8h
	db	 00h,  00h, 0FCh,  98h,  30h,  64h, 0FCh,  00h
	db	 1Ch,  30h,  30h, 0E0h,  30h,  30h,  1Ch,  00h
	db	 18h,  18h,  18h,  00h,  18h,  18h,  18h,  00h
	db	0E0h,  30h,  30h,  1Ch,  30h,  30h, 0E0h,  00h
	db	 76h, 0DCh,  00h,  00h,  00h,  00h,  00h,  00h
	db	 00h,  10h,  38h,  6Ch, 0C6h, 0C6h, 0FEh,  00h
