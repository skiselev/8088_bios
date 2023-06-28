# Linux Build Instructions

## Prerequisites

1. [NASM](https://www.nasm.us/)

Your distribution might already include **nasm** package. For example, on Ubuntu it can be installed using **sudo apt install nasm** command. If your distribution does not provide it, it is possible to download and build it using the link above

2. Make utiltiy

Most likely your distribution already comes with **make** utility. On Ubuntu it can be installed using **sudo apt install make** command

3. [CMake utility](https://cmake.org/)

Use your distribution package management system to install CMake. For example, on Debian based Linux distributons, e.g. on Ubuntu, it can be istalled using **sudo apt install make** command

4. Git

Git is the preferred method of acquiring the 8088 BIOS source. The alternate method, is to download the ZIP archive from GitHub using the **wget** utiltiy or a web browser. On Ubuntu git can be installed using **sudo apt install git** command

5. minipro software (optional)

If you have a MiniPro TL866xx series programmer, you can install Linux software for it from [minipro](https://gitlab.com/DavidGriffith/minipro) Git repo, or find corresponding packages for your Linux distribution

6. Clone or download and unpack 8088 BIOS source from [this](https://github.com/skiselev/8088_bios) repository

If using Git, this can be done using **git clone https://github.com/skiselev/8088_bios.git** command

## Building the BIOS

* Open a terminal
* Change current directory to the folder where 8088 BIOS was previously unpacked using **cd <path>** command
* Create a build directory and change the current directory to it, for example using **mkdir -p build && cd build** command
* Run **cmake ..** command to create Makefiles
* Run **make** to build the project. The resulting **.rom** and **.bin** files will be located in the current (build) directory

## Programming the BIOS to the Flash ROM
  
Assuming that you have TL866xx series programmer connected to your computer, you can use **minipro** command to program the resulting ***.rom** image to the Flash ROM:
* **minipro -p <FLASH_ROM_TYPE> -w <BIOS.rom>**
* For example, assuming that SST39SF010A Flash ROM is used and Micro 8088 image is to be programmed, use: **minipro -p SST39SF010A -w bios-micro8088-xtide.rom**
* Some other 128 KiB parallel Flash ROMs supported by minipro: SST29EE010, AT29C010A, WW29EE011, "AM29F010 @DIP32"
