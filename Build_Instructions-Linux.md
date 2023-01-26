# Linux Build Instructions

## Prerequisites

1. [NASM](https://www.nasm.us/)

Your distribution might already include **nasm** package. For example, on Ubuntu it can be installed using **sudo apt install nasm** command. If your distribution does not provide it, it is possible to download and build it using the link above

2. Make utiltiy

Most likely your distribution already comes with **make** utility. On Ubuntu it can be installed using **sudo apt install make** command

3. Git

Git is the preferred method of acquiring the 8088 BIOS source. The alternate method, is to download the ZIP archive from GitHub using the **wget** utiltiy or a web browser. On Ubuntu git can be installed using **sudo apt install git** command

4. minipro software (optional)

If you have a MiniPro TL866xx series programmer, you can install Linux software for it from [minipro](https://gitlab.com/DavidGriffith/minipro) Git repo, or find corresponding packages for your Linux distribution

5. Clone or download and unpack 8088 BIOS source from [this](https://github.com/skiselev/8088_bios) repository

If using Git, this can be done using **git clone https://github.com/skiselev/8088_bios.git** command

## Building the BIOS

* Open a terminal
* Change current directory to the folder where 8088 BIOS was previously unpacked using **cd <path>** command
* Run **make**

Note: By default the Makefile is configured to build BIOS for Micro 8088. The same BIOS should also work for NuXT systems. Edit the Makefile, and uncomment **#MACHINE=MACHINE_XI8088** line if you want to build BIOS for Xi 8088

## Programming the BIOS to the Flash ROM
  
Assuming that you have TL866xx series programmer, you can run **make flash** to program the resulting *.bin image to the Flash ROM
