;=========================================================================
; config.inc - Compilation time settings and settings
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

%define DATE		'11/28/17'	; BIOS release date MM/DD/YY
%define VERSION		'0.9.3'		; BIOS version

; Machine type is defined here
; only one of the defines below needs to be uncommented
;%define MACHINE_XI8088			; Xi 8088
%define MACHINE_FE2010A 		; Faraday FE2010A
;%define MACHINE_XT			; IBM PC/XT or highly compatible clone
					; FIXME: not implemented yet

; Settings for Xi 8088
%ifdef MACHINE_XI8088
%define	START		8000h		; BIOS starts at offset 8000h
%define MODEL_BYTE	0FCh		; Pretend it is an IBM PC/AT
%define AT_RTC				; Has AT-compatible RTC
%define AT_DELAY			; Use refresh bit of port 61h for delays
%define SECOND_PIC			; Has two PICs (AT-style)
%define AT_KEYBOARD			; Has AT-compatible keyboard controller
%define PS2_MOUSE			; Enable PS/2 auxiliary device support
%define DISABLE_KBD_DURING_INTERRUPTS	; Don't disable keyboard in INT1
%define MIN_RAM_SIZE	32		; At least 32 KiB to boot the system
%define MAX_RAM_SIZE	640		; Scan this much memory during POST
%define RAM_TEST_BLOCK	16384		; block size for RAM test
%define EBDA_SIZE	1		; 1KB reserved for EBDA
					; EBDA is required for PS/2 aux support
%define SLOW_FLOPPY			; turn off turbo during floppy I/O
%endif ; MACHINE_XI8088

; Settings for FE2010A
%ifdef MACHINE_FE2010A
%define	START		8000h		; BIOS starts at offset 8000h
%define MODEL_BYTE	0FEh		; IBM PC/XT
;%define SECOND_PIC			; AT-compatible system, two PICs
;%define PS2_MOUSE			; Enable PS/2 auxiliary device support
%define DISABLE_KBD_DURING_INTERRUPTS	; Don't disable keyboard in INT1
%define MIN_RAM_SIZE	32		; At least 32 KiB to boot the system
%define MAX_RAM_SIZE	640		; Scan this much memory during POST
%define RAM_TEST_BLOCK	16384		; block size for RAM test
;%define EBDA_SIZE	1		; 1KB reserved for EBDA
					; EBDA is required for PS/2 aux support
%define SLOW_FLOPPY			; turn off turbo during floppy I/O
%endif ; MACHINE_FE2010A

; Settings for IBM PC/XT
%ifdef MACHINE_XT
%define	START		0E000h		; BIOS starts at offset 0E000h
%define MODEL_BYTE	0FEh		; IBM PC/XT
;%define SECOND_PIC			; AT-compatible system, two PICs
;%define PS2_MOUSE			; Enable PS/2 auxiliary device support
%define DISABLE_KBD_DURING_INTERRUPTS	; Don't disable keyboard in INT1
%define MIN_RAM_SIZE	32		; At least 32 KiB to boot the system
%define MAX_RAM_SIZE	640		; Scan this much memory during POST
%define RAM_TEST_BLOCK	16384		; block size for RAM test
;%define EBDA_SIZE	1		; 1KB reserved for EBDA
					; EBDA is required for PS/2 aux support
%define SLOW_FLOPPY			; turn off turbo during floppy I/O
%endif ; MACHINE_FE2010A


