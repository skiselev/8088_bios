# Makefile - GNU Makefile
# 
# Copyright (C) 2011 - 2012 Sergey Kiselev.
# Provided for hobbyist use on the Xi 8088 board.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY# without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

SOURCES=bios.asm macro.inc atkbd.inc config.inc errno.inc floppy1.inc floppy2.inc kbc.inc misc.inc printer1.inc printer2.inc ps2aux.inc scancode.inc serial1.inc serial2.inc sound.inc time1.inc time2.inc video.inc cpu.inc messages.inc inttrace.inc rtc.inc fnt00-7F.inc fnt80-FF.inc

all: $(SOURCES) bios128k-1.0.bin bios128k-xtide-1.0.bin bios128k-2.0.bin bios128k-xtide-2.0.bin

bios128k-1.0.bin: bios.bin ff-64k.bin ff-32k.bin
	cat ff-64k.bin ff-32k.bin bios.bin > bios128k-1.0.bin

bios128k-xtide-1.0.bin: bios.bin ff-64k.bin ff-24k.bin ide_xt.bin
	cat ff-64k.bin ide_xt.bin ff-24k.bin bios.bin > bios128k-xtide-1.0.bin

bios128k-2.0.bin: bios.bin ff-64k.bin ff-32k.bin
	cat ff-32k.bin bios.bin ff-64k.bin > bios128k-2.0.bin

bios128k-xtide-2.0.bin: bios.bin ff-64k.bin ff-24k.bin ide_xt.bin
	cat ide_xt.bin ff-24k.bin bios.bin ff-64k.bin > bios128k-xtide-2.0.bin

bios.bin: $(SOURCES)
bios.bin: $(SOURCES)
	nasm -O9 -f bin -o bios.bin -l bios.lst bios.asm

clean:
	rm -f bios.bin bios-8.bin bios128k-1.0.bin bios128k-xtide-1.0.bin bios128k-2.0.bin bios128k-xtide-2.0.bin bios.lst
