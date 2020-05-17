;=========================================================================
; errno.inc - POST checkpoint codes (sent to port 80h during POST)
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

e_boot		equ	00h		; Boot the OS
e_cpu_test	equ	01h		; Start of BIOS POST, CPU test
e_init_cfg	equ	02h		; Initial chipset configuration:
					; initialize PPI, disable NMI
					; disable turbo mode, disable display
e_init_dmac	equ	03h		; Initialize DMAC initialized
e_low_ram_test	equ	04h		; Test low 32 KiB of RAM
e_int_table	equ	05h		; Initialize interrupt table
e_pit_init	equ	06h		; Initialize PIT (timer)
					; play power on melody
e_pic_init	equ	07h		; Initialize PIC
e_kbd_init	equ	08h		; Initialize KBC and keyboard
e_int_ena	equ	09h		; Enable interrupts
e_vid_bios_scan	equ	10h		; Locate video BIOS
e_vid_bios_init	equ	11h		; Initialize video BIOS
e_vid_no_bios	equ	12h		; No video BIOS, using MDA/CGA
e_rtc_init	equ	20h		; Initialize RTC
e_cpu_detect	equ	21h		; Detect CPU type
e_fpu_detect	equ	22h		; Detect FPU
e_serial_scan	equ	24h		; Detect serial ports
e_parallel_scan	equ	25h		; Detect parallel ports
e_ram_start	equ	30h		; Start RAM test
e_ram_complete	equ	31h		; RAM test completed
e_ram_esc	equ	32h		; RAM test canceled
e_ext_start	equ	40h		; Start BIOS extension ROM scan
e_ext_detect	equ	41h		; BIOS extension ROM found, initizalize
e_ext_init_ok	equ	42h		; BIOS extension ROM initialized
e_ext_complete	equ	43h		; BIOS extension scan complete
e_cpu_fail	equ	52h		; CPU test failed
e_low_ram_fail	equ	54h		; Low 32 KiB RAM test failed
e_ram_fail	equ	55h		; RAM test failed
e_kbc_flsh_fail	equ	60h		; Unable to flush KBC output buffer
e_kbc_cmd_fail	equ	61h		; Unable to send command to KBC
e_kbc_test_fail	equ	62h		; Keyboard controller self test failed
e_kbc_int_fail	equ	63h		; Keyboard interface test failed
e_kbd_bat_fail	equ	70h		; Keyboard BAT test failed
e_kbd_dis_fail	equ	71h		; Keyboard disable command failed
e_kbd_ena_fail	equ	72h		; Keyboard enable command failed
;-------------------------------------------------------------------------
; Minor numbers for keyboard errors
e_kbd_tout_fail	equ	01h		; Keyboard timeout sending command
e_kbd_resp_fail	equ	02h		; Keyboard no response
e_kbd_nack_fail	equ	03h		; Acknowledge response was expected
					; but keyboard have sent something else
e_kbd_rsp2_fail	equ	04h		; Keyboard no response byte 2 for BAT
e_kbd_nbat_fail	equ	05h		; BAT OK response was expected
					; but keyboard have sent something else
e_kbd_test_fail	equ	06h		; Keyboard controller test failed
e_kbd_int_fail	equ	07h		; Keyboard interface test failed
