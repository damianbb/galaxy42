### Instruction for Cygwin download/instalation.



#### 1. Download Cygwin installer:
###### You can download if from here:
- 32-bit installation: https://www.cygwin.com/setup-x86.exe
- 64-bit installation: https://www.cygwin.com/setup-x86_64.exe

#### 2. Install Cygwin:
1. Run downloaded file
2. -> Next
3. "Install from internet" -> Next
4. Your settings -> Next (recomended basic)
5. Your settings -> Next (recomended basic)
6. "Direct Connection" -> Next
7. -> Next
8. Choose packeges to download
9. -> Next
10. -> Finish

#### 4. Choosing packeges:

Example:
- search git
- click on Devel
- find line witch "git: Distributed version control system"
- click several time on Skip/Keep in this line and choose latest version

##### 5. Needed packeges:
###### Convention:
  search:     click on:       [line to find]
- git:        Devel:          [git: Distributed version control system]
- boost:      Devel + libs:   all
- sodium:     Devel + libs:   all
- libtool:    Devel:          [libtool: Generic library support script]
- cmake:      Devel:          [cmake: Cross-platform makefile generation system]
- wget:       Web:            all
- make:       Devel:          (make: GUI version of the 'make' utillity)
- gcc:        Devel:          mingw64-i686-gcc-g++: GCC for Win32 (i686-w64-mingw32) toolchain (C++)
- gcc:        Devel:          mingw64-x86_64-gcc-g++: GCC for Win64 toolchain (C++)
- gettext     Devel:          gettext-devel:GNUInternationalization development utilities

### Building.
#### 1. Download code from github:
```sh
$ git clone https://github.com/yedino/galaxy42.git
$ cd galaxy42
$ git submodule update --init
```
#### 2. Set compiler:
- for 32-bit
```sh
$ export CC=i686-w64-mingw32-gcc
$ export CXX=i686-w64-mingw32-g++
```
- for 64-bitst
```sh
$ export CC=x86_64-w64-mingw32-gcc
$ export CXX=x86_64-w64-mingw32-g++
```
  ###### Checking compiler
```sh
$ $CXX -v
```

  Version must be >= 5.0.0

  Thread model must be posix


### 3. Building
``` sh
$ cmake .
$ make tunserver.elf
```
For building windows service wrapper
```sh
$ cmake .
$ make windowsService
```

### 4. Running
##### Install TUN/TAP device

 https://github.com/yedino/galaxy42/blob/master/doc/msvc-instructions.md#create-tuntap-device
##### Copy dll files to binary directory

-  Source directory for 32-bit

  `/usr/i686-w64-mingw32/sys-root/mingw/bin/`

-  Source directory for 64-bit

  `/usr/x86_64-w64-mingw32/sys-root/mingw/bin/`  

##### Libraries list

- libsodium-18.dll
- libwinpthread-1.dll
- libboost_system.dll
- libboost_program_options.dll
- libboost_locale-mt.dll
- libboost_filesystem.dll
- libstdc++-6.dll
- libgcc_s_seh-1.dll
- iconv.dll
- icui18n57.dll
- icuuc57.dll
- icudata57.dll
- libboost_system-mt.dll
- libboost_thread-mt.dll

##### Run as Administrator
- Run cmd as Administrator(right mouse button -> Run as Administrator)
- Go to binary dir (e.g. `cd C:\cygwin64\home\win2\galaxy42`)
- Run tunserver (`tunserver.elf.exe`)


Tested on Cygwin 2.6.0(0.304/5/3)

mingw version 5.4.0
