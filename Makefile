# Makefile - GNU Makefile
# 
# Copyright (C) 2010 - 2019 Sergey Kiselev.
# Provided for hobbyist use on the Xi 8088 and Micro 8088 boards.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Target machine type is defined below
# Xi 8088 Board
#MACHINE=MACHINE_XI8088
# Micro 8088 Board
MACHINE=MACHINE_FE2010A
# IBM PC/XT or highly compatible board (FIXME: not implemented yet)
#MACHINE=MACHINE_XT

# Flash ROM IC type (as supported by minipro programmer)
FLASH_ROM=SST39SF010A
#FLASH_ROM=SST29EE010
#FLASH_ROM=AT29C010A
#FLASH_ROM=W29EE011
#FLASH_ROM="AM29F010 @DIP32"

SOURCES=bios.asm macro.inc at_kbc.inc config.inc errno.inc flash.inc floppy1.inc floppy2.inc keyboard.inc misc.inc printer1.inc printer2.inc ps2aux.inc scancode.inc serial1.inc serial2.inc setup.inc sound.inc time1.inc time2.inc video.inc cpu.inc messages.inc inttrace.inc rtc.inc fnt00-7F.inc fnt80-FF.inc

ifeq "$(MACHINE)" "MACHINE_XI8088"
IMAGES=bios-sergey-xt.bin bios-sergey-xt-xtide.bin bios-xi8088.bin bios-xi8088-xtide.bin
FLASH_IMAGE=bios-xi8088.bin
else
ifeq "$(MACHINE)" "MACHINE_FE2010A"
IMAGES=bios-micro8088.bin bios-micro8088-xtide.bin
FLASH_IMAGE=bios-micro8088.bin
else
IMAGES=bios.bin
endif
endif

all: $(SOURCES) $(IMAGES)

bios.bin: $(SOURCES)
	nasm -D$(MACHINE) -O9 -f bin -o bios.bin -l bios.lst bios.asm

bios-micro8088.bin: bios.bin
	dd if=/dev/zero ibs=1k count=40 | tr "\000" "\377" > bios-micro8088.bin
	cat bios.bin >> bios-micro8088.bin
	dd if=/dev/zero ibs=1k count=64 | tr "\000" "\377" >> bios-micro8088.bin

bios-micro8088-xtide.bin: bios.bin ide_xt.bin
	cat ide_xt.bin > bios-micro8088-xtide.bin
	dd if=/dev/zero ibs=1k count=32 | tr "\000" "\377" >> bios-micro8088-xtide.bin
	cat bios.bin >> bios-micro8088-xtide.bin
	dd if=/dev/zero ibs=1k count=64 | tr "\000" "\377" >> bios-micro8088-xtide.bin

bios-sergey-xt.bin: bios.bin
	dd if=/dev/zero ibs=1k count=96 | tr "\000" "\377" > bios-sergey-xt.bin
	cat bios.bin >> bios-sergey-xt.bin

bios-sergey-xt-xtide.bin: bios.bin ide_xt.bin
	dd if=/dev/zero ibs=1k count=64 | tr "\000" "\377" > bios-sergey-xt-xtide.bin
	cat ide_xt.bin >> bios-sergey-xt-xtide.bin
	dd if=/dev/zero ibs=1k count=24 | tr "\000" "\377" >> bios-sergey-xt-xtide.bin
	cat bios.bin >> bios-sergey-xt-xtide.bin

bios-xi8088.bin: bios.bin
	dd if=/dev/zero ibs=1k count=32 | tr "\000" "\377" > bios-xi8088.bin
	cat bios.bin >> bios-xi8088.bin
	dd if=/dev/zero ibs=1k count=64 | tr "\000" "\377" >> bios-xi8088.bin

bios-xi8088-xtide.bin: bios.bin ide_xt.bin
	cat ide_xt.bin > bios-xi8088-xtide.bin
	dd if=/dev/zero ibs=1k count=24 | tr "\000" "\377" >> bios-xi8088-xtide.bin
	cat bios.bin >> bios-xi8088-xtide.bin
	dd if=/dev/zero ibs=1k count=64 | tr "\000" "\377" >> bios-xi8088-xtide.bin

clean:
	rm -f bios.lst bios.bin bios-micro8088.bin bios-micro8088-xtide.bin bios-sergey-xt.bin bios-sergey-xt-xtide.bin bios-xi8088.bin bios-xi8088-xtide.bin

flash:
	minipro -p $(FLASH_ROM) -w $(FLASH_IMAGE)
