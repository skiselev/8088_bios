cmake_minimum_required(VERSION 3.16)

set(CMAKE_ASM_NASM_OBJECT_FORMAT bin)
set(CMAKE_ASM_NASM_FLAGS -O9)
set(CMAKE_ASM_NASM_LINK_EXECUTABLE "mv <OBJECTS> <TARGET>")
project(8088_BIOS LANGUAGES ASM_NASM)

include_directories(src)

set(SOURCES src/bios.asm src/at_kbc.inc src/delay.inc src/floppy1.inc src/fnt80-FF.inc src/macro.inc src/printer1.inc src/rtc.inc src/serial2.inc src/time1.inc src/config.inc src/errno.inc src/floppy2.inc src/inttrace.inc src/messages.inc src/printer2.inc src/scancode.inc src/setup.inc src/time2.inc src/cpu.inc src/flash.inc src/fnt00-7F.inc src/keyboard.inc src/misc.inc src/ps2aux.inc src/serial1.inc src/sound.inc src/video.inc)

set(XTIDE_300 xtide_r625/ide_xt-cf-lite_300h.bin)
set(XTIDE_V20_300 xtide_r625/ide_xtp-cf-lite_300h.bin)
set(XTIDE_320 xtide_r625/ide_xt-cf-lite_320h.bin)
cmake_path(ABSOLUTE_PATH XTIDE_300)
cmake_path(ABSOLUTE_PATH XTIDE_V20_300)
cmake_path(ABSOLUTE_PATH XTIDE_320)

set(CMAKE_ASM_NASM_OBJECT_FORMAT bin)

add_executable(bios-micro8088.bin ${SOURCES})
target_compile_definitions(bios-micro8088.bin PRIVATE MACHINE_FE2010A)
add_custom_command(
	TARGET bios-micro8088.bin
	POST_BUILD
	COMMAND dd if=/dev/zero ibs=1k count=40 | LANG=C tr \\\\000 \\\\377 > bios-micro8088-noide.rom
	COMMAND cat bios-micro8088.bin >> bios-micro8088-noide.rom
	COMMAND dd if=/dev/zero ibs=1k count=64 | LANG=C tr \\\\000 \\\\377 >> bios-micro8088-noide.rom
	COMMAND cat ${XTIDE_320} > bios-micro8088-xtide.rom
	COMMAND dd if=/dev/zero ibs=1k count=32 | LANG=C tr \\\\000 \\\\377 >> bios-micro8088-xtide.rom
	COMMAND cat bios-micro8088.bin >> bios-micro8088-xtide.rom
	COMMAND dd if=/dev/zero ibs=1k count=64 | LANG=C tr \\\\000 \\\\377 >> bios-micro8088-xtide.rom
	COMMENT "Creating ROM images for Micro 8088"
)

add_executable(bios-xi8088.bin ${SOURCES})
target_compile_definitions(bios-xi8088.bin PRIVATE MACHINE_XI8088)
add_custom_command(
	TARGET bios-xi8088.bin
	POST_BUILD
	COMMAND dd if=/dev/zero ibs=1k count=96 | LANG=C tr \\\\000 \\\\377 > bios-sergey-xt-noide.rom
	COMMAND cat bios-xi8088.bin >> bios-sergey-xt-noide.rom
	COMMAND dd if=/dev/zero ibs=1k count=64 | LANG=C tr \\\\000 \\\\377 > bios-sergey-xt-xtide.rom
	COMMAND cat ${XTIDE_320} >> bios-sergey-xt-xtide.rom
	COMMAND dd if=/dev/zero ibs=1k count=24 | LANG=C tr \\\\000 \\\\377 >> bios-sergey-xt-xtide.rom
	COMMAND cat bios-xi8088.bin >> bios-sergey-xt-xtide.rom
	COMMAND dd if=/dev/zero ibs=1k count=32 | LANG=C tr \\\\000 \\\\377 > bios-xi8088-noide.rom
	COMMAND cat bios-xi8088.bin >> bios-xi8088-noide.rom
	COMMAND dd if=/dev/zero ibs=1k count=64 | LANG=C tr \\\\000 \\\\377 >> bios-xi8088-noide.rom
	COMMAND cat ${XTIDE_320} > bios-xi8088-xtide.rom
	COMMAND dd if=/dev/zero ibs=1k count=24 | LANG=C tr \\\\000 \\\\377 >> bios-xi8088-xtide.rom
	COMMAND cat bios-xi8088.bin >> bios-xi8088-xtide.rom
	COMMAND dd if=/dev/zero ibs=1k count=64 | LANG=C tr \\\\000 \\\\377 >> bios-xi8088-xtide.rom
	COMMENT "Creating ROM images for Xi 8088 and Sergey's XT"
)

add_executable(bios-book8088.bin ${SOURCES})
target_compile_definitions(bios-book8088.bin PRIVATE MACHINE_BOOK8088)
add_custom_command(
	TARGET bios-book8088.bin
	POST_BUILD
	COMMAND cat ${XTIDE_300} > bios-book8088-xtide.rom
	COMMAND dd if=/dev/zero ibs=1k count=40 | LANG=C tr \\\\000 \\\\377 >> bios-book8088-xtide.rom
	COMMAND cat bios-book8088.bin >> bios-book8088-xtide.rom
	COMMAND cat ${XTIDE_V20_300} > bios-book8088-xtide-v20.rom
	COMMAND dd if=/dev/zero ibs=1k count=40 | LANG=C tr \\\\000 \\\\377 >> bios-book8088-xtide-v20.rom
	COMMAND cat bios-book8088.bin >> bios-book8088-xtide-v20.rom
	COMMENT "Creating ROM image for Book8088"
)

add_executable(bios-xt.bin ${SOURCES})
target_compile_definitions(bios-xt.bin PRIVATE MACHINE_XT)
