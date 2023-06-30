# Windows Build Instructions - Building BIOS'es for Fun and Profit

Contributed by [SQLServerIO](https://github.com/SQLServerIO), the original text is [here](https://github.com/skiselev/8088_bios/issues/13)

## Download and Install

1. [NASM](https://www.nasm.us/): The [latest stable version 2.15.05](https://www.nasm.us/pub/nasm/releasebuilds/2.15.05/win64/nasm-2.15.05-installer-x64.exe)
2. [GNUWin32 Make](http://gnuwin32.sourceforge.net/packages/make.htm): [binary package download](http://gnuwin32.sourceforge.net/downlinks/make.php)
3. [GNUWin32 CoreUtils](http://gnuwin32.sourceforge.net/packages/coreutils.htm): [binary package download](http://gnuwin32.sourceforge.net/downlinks/coreutils.php). This package contains the **tr** utility
4. John Newbigin's [dd for windows](http://www.chrysocome.net/dd): The [latest version 0.6beta3](http://www.chrysocome.net/downloads/dd-0.6beta3.zip). This version implements /dev/zero, while GNUWin32 does not
  * Rename the original **C:\Program Files (x86)\GnuWin32\bin\dd.exe** to **dd_gnu.exe**
  * Download and then unzip **dd.exe** from this package to the **C:\Program Files (x86)\GnuWin32\bin** folder
5. Download and unpack 8088 BIOS source from [this](https://github.com/skiselev/8088_bios) repository

## Add NASM and GNUWin32 to PATH

Add NASM and GNUWin32 to the Windows PATH under Settings->Advanced settings->Environment Variables->Path (either local or system)

* NASM default installation path is **C:\Program Files\NASM**
* GNUWin32 default installation path is **C:\Program Files (x86)\GnuWin32\bin**

Alternatively you can temporarily add NASM and GNUWin32 to the PATH environment variable before running make. See [Building the BIOS](#building-the-bios) section below

## Building the BIOS

* Open a CMD console
* Change current directory to the folder where 8088 BIOS was previously unpacked using **cd <8088_bios_path>\src** command
* If needed, add NASM and GNUWin32 to the PATH environment variable:
  * **set PATH=%PATH%;C:\Program Files (x86)\GnuWin32\bin;C:\Program Files\NASM**
* Run **make -f Makefile.win** 

## Programming the BIOS to the Flash ROM
  
Use MiniPro GUI to program the resulting *.rom images to the Flash ROM
