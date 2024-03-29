Dolby Encoding Engine plugin for the x265 HEVC encoder

The provided source code was built and tested using the x265 library version 3.5.

Build tools: 
- Visual Studio 2015
- gcc 4.8.5
- clang 12.0.0

x265 compilation and prerequisities:
- CMake
    Visit: https://cmake.org/download/ or https://cmake.org/install/ for more details
    On Ubuntu: sudo apt-get install cmake
    Make sure cmake is added to PATH

- NASM:
    x265 shall be built with NASM (verified with 2.15.05) in order to achieve maximum performance.
    NASM Release Builds are currently stored at www.nasm.us/pub/nasm/releasebuilds/ 
    Linux & Mac:
    - Download:
      wget www.nasm.us/pub/nasm/releasebuilds/2.15.05/nasm-2.13.05.tar.xz
    - Extract:
      tar -xvf nasm-2.15.05.tar.xz
    - Go to the extracted nasm-2.15.05 directory, build and install:
      ./configure
      make
      sudo make install  
    Windows:
    - Download the installer: 
      https://www.nasm.us/pub/nasm/releasebuilds/2.15.05/win64/nasm-2.15.05-installer-x64.exe
    - Execute the installer and follow the instructions
    - Add the NASM installation directory to PATH

- Building x265:
    It is mandatory to build the x265 library with at least 10-bit API.
    The recommended way is to build an x265 library containing 8/10/12-bit API.
    The simplest way is to use the x265 multilib build scripts.
    Linux & Mac:
    ./make-Makefiles.bash (If building for Mac, set CMAKE_OSX_DEPLOYMENT_TARGET value to 10.12)

    ./multilib.sh (in x265_3.5/build/linux directory)
    
    Windows:
    .\multilib.bat (in x265_3.5\vc12-x86_64 if building in VS2013, in x265_3.5\vc15-x86_64 if VS2015 or later)
    If builing with VS2015, change %VS150COMNTOOLS% in build-all.bat and multilib.bat to %VS140COMNTOOLS%
    and change occurances of -G "Visual Studio 15 Win64" to -G "Visual Studio 14 Win64" in build-all.bat, make-solutions.bat, multilib.bat
    or refer to resources/engineering_docs/x265_build_and_install/BuildAndInstallx265Plugin.md
    
    If you set up NASM properly, at the start of execution, the following message displays:
    Found nasm: C:/Users/[USER]/NASM/nasm.exe (found version "2.15.05")
    Found Nasm 2.15.05 to build assembly primitives
    x265 version 3.5

    Generated libraries can be found in /8bit (/8bit/Release if Windows) directory

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
4. Rename "libx265.so" (or create a symbolic link) according to the used x265 API version, e.g. "libx265.so.199"
    -  To check what the expected name is, use the command: ldd hevc_enc_x265_release.so, or check the number in x265_config.h file
