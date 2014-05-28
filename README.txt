README file for Xi 8088 / Sergey's XT BIOS
==========================================

BIOS Images
-----------

bios.bin		- BIOS image for use with xiflash utility
bios128k-2.0.bin	- BIOS image for Xi 8088 - Version 2.0
bios128k-xtide-2.0.bin	- BIOS image for Xi 8088 - Version 2.0 with XT-IDE
bios128k-1.0.bin	- BIOS image for Sergey's XT - Version 1.0
bios128k-xtide-1.0.bin	- BIOS image for Sergey's XT - Version 1.0 with XT-IDE

TODO:
- Investigate 'FF' displayed in the year
- Debug mouse issue with Intel 8242
- Debug issues with Microsoft and Logitech mouse drivers

Changes - Version 0.9.0
-----------------------
- No updates except of BIOS date and version

Changes - Version 0.8.2
-----------------------
- Fix keypad '*' interpreted as print screen
- Output '00' POST code to Port 80h when booting OS
- Use OKI-designed 80C88 and Harris-designed 80C88 instead older/newer 80C88
- Add date and time setup to RTC setup utility
- NVRAM setup utility - print help only if requested
- Minor bug fixes and readability improvements in floppy code
- Fix compilation errors with AT_COMPAT and PS2_MOUSE disabled

Changes - Version 0.8.1
-----------------------
- Fix BIOS extension ROM scan procedure. Previously in some cases it was
  failing to initialize more than one BIOS extension ROM.

Changes - Version 0.8
---------------------
- Add serial port (INT 14h) support
- Add printer (INT 17h) support
- Add print screen (INT 5) support
- Print BIOS extension ROM addresses on ROM initialization
- Add more POST checkpoints, update POST codes
- Rename Sergey's XT references to Xi 8088

Changes - Version 0.7e
----------------------

- Set DS to the BIOS data segment after calling extension ROM initialization
  routines. Fixes the bug where POST would stuck following initialization
  of an extension ROM that doesn't preserve DS. (Reported by Bill Lewis)

Changes - Version 0.7d
----------------------

- Extension ROM scan
	- Include 0F0000h - 0F7FFFh area in scan, so that extensions
	  can be added to the system's flash.

- POST
	- Reset IOCHK trigger, disable turbo mode
	

Changes - Version 0.7c
----------------------

- IPL
	- Fix boot sector signature address
	- Fix error when booting from floppy (call INT 13h AH=08h before boot)

- POST
	- Add DMA initialization
	- Skip memory test if ESC pressed
	- Skip memory test on warm reboot

- Keyboard / INT 09h
	- Add support for Ctrl-Break
	- Add support for Pause

- Video / INT 10h
	- Functions 06h and 07h: Improve scrolling implementation
	- Function 00h: Fix bug with clearing display in graphics modes
	- Use free font for graphics modes
	- Add graphics font for characters 80h-0FFh

- Floppy / INT 13h:
	- Use 2.88M settings for the default disk parameter table.
          Previosly 160K settings were used and BIOS was failing to boot
          from disks with more than 8 sectors per track.
	- More clean and effective fdc_get_result implementation,
	  fdc_wait_input removed as it no longer needed

Changes - Version 0.7b
----------------------

- Initial public release


TODO
----

- [crit] Remove XXX comments
- [high] Add technical documentation
- [med] Finalize extended keyboard support - full extended keyboard support
- [med] Beep if no video, install dummy handler
- [low] Keyboard - sound on buffer overflow
- [low] More tests - RTC, memory, DMA
- [low] Pause if something in POST fails, so user can read messages
- [low] Reset turbo bit during boot
- [low] Init display before keyboard, so KBD errors can be displayed
        Alternatively store non-fatal errors and display them after display
        is initialized
- [low] Add XT-IDE to system's flash (@ 0F0000h), change ROM scan to scan
        that area
- [low] Check possibility of using same EBDA for XT-IDE BIOS and system
        BIOS PS/2 mouse functions
- [low] BIOS checksum
- [enh] Add PnP extension


Switches and jumpers settings
-----------------------------

SW2-8: Display adapter type:
	Off = CGA
	On = MDA or Hercules
	Ignored if Video BIOS is present (EGA / VGA cards)
