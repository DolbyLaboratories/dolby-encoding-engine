Dolby Encoding Engine plugin for the x265 HEVC encoder

The provided source code was built and tested using the x265 library version 2.8.

Build tools: 
- Visual Studio 2013
- gcc 4.8.5

x265 compilation and prerequisities:

- NASM:
    x265 shall be built with NASM (verified with 2.13.03) in order to achieve maximum performance.
    NASM Release Builds are currently stored at www.nasm.us/pub/nasm/releasebuilds/ 
    Linux:
    - Download:
      wget www.nasm.us/pub/nasm/releasebuilds/2.13.03/nasm-2.13.03.tar.xz
    - Extract:
      tar -xvf nasm-2.13.03.tar.xz
    - Go to the extracted nasm-2.13.03 directory, build and install:
      ./configure
      make
      sudo make install  
    Windows:
    - Download the installer: 
      https://www.nasm.us/pub/nasm/releasebuilds/2.13.03/win64/nasm-2.13.03-installer-x64.exe
    - Execute the installer and follow the instructions
    - Add the NASM installation directory to PATH

- Building x265:
    It is mandatory to build the x265 library with at least 10-bit API.
    The recommended way is to build an x265 library containing 8/10/12-bit API.
    The simplest way is to use the x265 multilib build scripts.
    
    Linux:
    ./multilib.sh (in x265_2.8/build/linux directory)
    
    Windows:
    .\multilib.bat (in x265_2.8\vc12-x86_65)
    
    If you set up NASM properly, at the start of execution, the following message displays:
    Found nasm: C:/Users/mgaik/NASM/nasm.exe (found version "2.13.03")
    Found Nasm 2.13.03 to build assembly primitives
    x265 version 2.8

    Visit https://bitbucket.org/multicoreware/x265/wiki/Home for more details.

Plugin prerequisities:
- The "libx265.lib" and "libx265.dll" libraries on Windows
- The "libx265.so" library on Linux
- The "include" folder with the following headers: "x265.h" and "x265_config.h"

    X265ROOT
    ├── include
    │   ├── x265_config.h
    │   └── x265.h
    └── lib
        ├── linux64
        │   └── libx265.so
        └── windows64
            ├── libx265.dll
            └── libx265.lib

Build instructions, Windows:
1. Set an X265ROOT environment variable pointing to the folder with prerequisites
2. Build using make\windows_amd64_msv\hevc_enc_x265_2013.sln
3. Copy the .dll library with the plugin and "libx265.dll" to the Dolby Encoding Engine installation folder

Build instructions, Linux:
1. Set an X265ROOT environment variable pointing to the folder with prerequisites
2. Build using make/hevc_enc_x265/linux_amd64_gnu/Makefile
    - make hevc_enc_x265_release.so
3. Copy the .so library with the plugin and "libx265.so" to the Dolby Encoding Engine installation folder
4. Rename "libx265.so" (or create a symbolic link) according to the used x265 API version, e.g. "libx265.so.160"
    -  To check what the expected name is, use the command: ldd hevc_enc_x265_release.so
