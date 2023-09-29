# 8088 BIOS

Open source BIOS for [Micro 8088], [NuXT][] ([Git repo][NuXT-git]),
and [Xi 8088] systems

[Micro 8088]: https://github.com/skiselev/micro_8088
[NuXT]: https://www.vogonswiki.com/index.php/NuXT
[NuXT-git]: https://github.com/monotech/NuXT
[Xi 8088]: http://www.malinov.com/Home/sergeys-projects/xi-8088

## Acknowledgements

Following folks contributed to this project. Thank you!

* [640-KB](https://github.com/640-KB)
  * XT-related fixes
  * Delay implementation using the 8253 PIT
* [Aitor Gómez](https://github.com/spark2k06)
  * RTC add-on board support
  * CPU clock frequency configuration setting
* [horkthane](https://github.com/horkthane)
  * Keyboard code fixes
  * VGA/EGA initialization fixes
* [Curt J. Sampson](https://github.com/0cjs)
  * Documentation enhancements
* [Rink](https://github.com/zhmu)
  * Video code update that improves compatibility with FreeDOS utilties
* Multiple other individuals that helped this project by testing the code, submitting bug reports, and contributing their ideas for improvements

## BIOS Images

|File name                                                                                                       | Description                                                       |
|----------------------------------------------------------------------------------------------------------------|-------------------------------------------------------------------|
|[bios-micro8088.bin](https://github.com/skiselev/8088_bios/blob/master/binaries/bios-micro8088.bin)             | BIOS image for Micro 8088 Version 1.1 to use with xiflash utility |
|[bios-micro8088-noide.rom](https://github.com/skiselev/8088_bios/blob/master/binaries/bios-micro8088-noide.rom) | BIOS image for Micro 8088 Version 1.1                             |
|[bios-micro8088-xtide.rom](https://github.com/skiselev/8088_bios/blob/master/binaries/bios-micro8088-xtide.rom) | BIOS image for Micro 8088 Version 1.1 with XT-IDE                 |
|[bios-xi8088.bin](https://github.com/skiselev/8088_bios/blob/master/binaries/bios-xi8088.bin)                   | BIOS image for Xi 8088 Version 2.0 to use with xiflash utility    |
|[bios-xi8088-noide.rom](https://github.com/skiselev/8088_bios/blob/master/binaries/bios-xi8088-noide.rom)       | BIOS image for Xi 8088 Version 2.0                                |
|[bios-xi8088-xtide.rom](https://github.com/skiselev/8088_bios/blob/master/binaries/bios-xi8088-xtide.rom)       | BIOS image for Xi 8088 Version 2.0 with XT-IDE                    |
|[bios-sergey-xt-noide.rom](https://github.com/skiselev/8088_bios/blob/master/binaries/bios-sergey-xt-noide.rom) | BIOS image for Sergey's XT Version 1.0                            |
|[bios-sergey-xt-xtide.rom](https://github.com/skiselev/8088_bios/blob/master/binaries/bios-sergey-xt-xtide.rom) | BIOS image for Sergey's XT Version 1.0 with XT-IDE                |
|[bios-book8088-xtide.rom](https://github.com/skiselev/8088_bios/blob/master/binaries/bios-book8088-xtide.rom)   | BIOS image for Book8088 with XT-IDE                               |
|[bios-book8088-xtide-v20.rom](https://github.com/skiselev/8088_bios/blob/master/binaries/bios-book8088-xtide-v20.rom)   | BIOS image for Book8088 with XT-IDE optimized for NEC V20 |
|[bios-xt.bin](https://github.com/skiselev/8088_bios/blob/master/binaries/bios-xt.bin)                           | BIOS image for IBM XT (not tested, 16 KiB ROM)                    |

## Build Instructions

* [Linux Build Instructions](Build_Instructions-Linux.md)
* [Windows Build Instructions](Build_Instructions-Windows.md)

## Release Notes

### Changes

* Version 1.0.0
  * Add support for Book8088
  * Use PIT for delays on XT-compatible systems
  * Implement several fixes for IBM XT
  * Update XT-IDE Extension BIOS to r625
  * Use CMake and build ROM images for all supported machines and configurations

* Version 0.9.9
  * Port floppy BIOS from [Multi-Floppy BIOS extension](https://github.com/skiselev/floppy_bios)
  * Add chipset detection for Faraday FE2010A and Proton PT8010AF chipsets. Disable memory refresh on these chipsets
  * Add a setup option for configuring wait states for Faraday FE2010A and Proton PT8010AF chipsets
  * Add a setup configuration option to disable power-on memory test. Disabling the memory test speeds up the boot process
  * Add support for SST39SF020A and SST39SF040 Flash ROMs. Note that unused address lines on these chips need to be connected to the ground
  * Update XT-IDE BIOS ROM Extension to r624. Add corresponding xtidecfg.com utility. The XT-IDE BIOS ROM image provided with 8088 BIOS is configured for XT-CF-Lite at port 0x320. Use the xtidecfg.com utility to reconfigure that if needed prior to building the BIOS
  * Fix keyboard scan codes for several key combinations: Ctrl-b, Ctrl-v, Ctrl-q, Alt-q, Ctrl-+
  * Fix an issue with BIOS resetting equipment list after VGA BIOS extension initialization, which resulted in a missing or incorrect video output then VGA is connected to a monochrome display

* Version 0.9.8
  * Merge Micro 8088 and Xi 8088 to master branch (use same code base)
  * Implement BIOS setup option for F0000-F7FFF area extension ROM scan
  * Fix FDC data rate setting on systems with 360KB/720KB drives
  * Update the number of floppies in equipment_word when saving configuration
  * Revise POST codes. Display POST code prior to an action, such as device test or subsystem initialization
  * Micro 8088: RTC autodetect fixes
  * Micro 8088: Fix floppy drives configuration
  * Xi 8088: AT / PS/2 keyboard controller initialization code improved

* Version 0.9.7
  * Micro 8088: Implement RTC support and autodetect (based on the code contributed by Aitor Gomez)
  * Micro 8088: Implement setting 40x25 CGA mode using DIP switches
  * Xi 8088: Implement CPU clock frequency configuration in the BIOS setup

* Version 0.9.6
  * Micro 8088: Implement CPU clock frequency configuration in the BIOS setup utility (contributed by Aitor Gomez)
  * Fix the issue where BIOS would activate RTS when sending a character
  * Fix the issue with 24 hours clock rollover

* Version 0.9.5
  * Micro 8088: Fix reading serial port status
  * Micro 8088: Implement checksum for the Flash ROM configuration space
  * Fix booting without XTIDE Universal BIOS

* Version 0.9.4
  * Micro 8088: Implement BIOS setup utility including saving configuration to the BIOS Flash ROM
  * Improve build configurability

* Version 0.9.3
  * Implement turbo mode switching using keyboard on Micro 8088
  * Fix the issue where the number of serial and printer ports was not updated in the equipment word
  * Update XT keyboard reset and buffer flush code
  * Fix the issue where the BIOS would hang when one of the Lock keys is pressed
  * Increase I/O delay in RTC code to solve 'FF' displayed in the year issue

* Version 0.9.2
  * Update configuration mechanism to enable support of multiple target platforms
  * Add initial support for Micro 8088 board using Faraday FE2010A chipset

* Version 0.9.0
  * No updates except of BIOS date and version

* Version 0.8.2
  * Fix keypad '*' interpreted as print screen
  * Output '00' POST code to Port 80h when booting OS
  * Use OKI-designed 80C88 and Harris-designed 80C88 instead older/newer 80C88
  * Add date and time setup to RTC setup utility
  * NVRAM setup utility - print help only if requested
  * Minor bug fixes and readability improvements in floppy code
  * Fix compilation errors with AT_COMPAT and PS2_MOUSE disabled

* Changes - Version 0.8.1
  * Fix BIOS extension ROM scan procedure. Previously in some cases it was failing to initialize more than one BIOS extension ROM.

* Changes - Version 0.8
  * Add serial port (INT 14h) support
  * Add printer (INT 17h) support
  * Add print screen (INT 5) support
  * Print BIOS extension ROM addresses on ROM initialization
  * Add more POST checkpoints, update POST codes
  * Rename Sergey's XT references to Xi 8088

* Version 0.7e
  * Set DS to the BIOS data segment after calling extension ROM initialization routines. Fixes the bug where POST would stuck following initialization of an extension ROM that doesn't preserve DS. (Reported by Bill Lewis)

* Version 0.7d
  * Extension ROM scan
    * Include 0F0000h - 0F7FFFh area in scan, so that extensions can be added to the system's flash.
  * POST
    * Reset IOCHK trigger, disable turbo mode
	
* Version 0.7c
  * IPL
    * Fix boot sector signature address
    * Fix error when booting from floppy (call INT 13h AH=08h before boot)
  * POST
    * Add DMA initialization
    * Skip memory test if ESC pressed
    * Skip memory test on warm reboot
  * Keyboard / INT 09h
    * Add support for Ctrl-Break
    * Add support for Pause
  * Video / INT 10h
    * Functions 06h and 07h: Improve scrolling implementation
    * Function 00h: Fix bug with clearing display in graphics modes
    * Use free font for graphics modes
    * Add graphics font for characters 80h-0FFh
  * Floppy / INT 13h:
    * Use 2.88M settings for the default disk parameter table. Previosly 160K settings were used and BIOS was failing to boot from disks with more than 8 sectors per track.
    * More clean and effective fdc_get_result implementation, fdc_wait_input removed as it no longer needed

* Version 0.7b
  * Initial public release


## TODO

* **(high)** Add technical documentation
* **(med)** Finalize extended keyboard support - full extended keyboard support
* **(med)** Beep if no video, install dummy handler
* **(low)** Xi 8088: Debug mouse issue with Intel 8242
* **(low)** Xi 8088: Debug issues with Microsoft and Logitech mouse drivers
* **(low)** Keyboard - sound on buffer overflow
* **(low)** More tests - RTC, memory, DMA
* **(low)** Init display before keyboard, so KBD errors can be displayed. Alternatively store non-fatal errors and display them after display is initialized
* **(low)** Check possibility of using same EBDA for XT-IDE BIOS and system BIOS PS/2 mouse functions
* **(low)** BIOS checksum
* **(enh)** Add PnP extension


## Switches and jumpers settings - Xi 8088

* SW2-8: Display adapter type:
  * Off = CGA
  * On = MDA or Hercules
  * Ignored if Video BIOS is present (EGA / VGA cards)
