Dolby Encoding Engine plugin for x265 HEVC encoder.
Provided source code was built and tested using x265 library version 2.5.

Build tools: 
- Visual Studio 2013
- gcc 4.8.5

Prerequisites:
- Libraries "libx265.lib" and "libx265.dll" on Windows
- Library "libx265.so" on Linux
- Folder "include" with headers: "x265.h" and "x265_config.h"

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
1. Set environment variable X265ROOT pointing to folder with prerequisites
2. Build using make\windows_amd64_msv\hevc_enc_x265_2013.sln
3. Copy .dll library with plugin  and "libx265.dll" to DEE installation folder

Build instructions, Linux:
1. Set environment variable X265ROOT pointing to folder with prerequisites
2. Build using make/hevc_enc_x265/linux_amd64_gnu/Makefile
    - make hevc_enc_x265_release.so
3. Copy .so library with plugin and "libx265.so" to DEE installation folder
4. Rename "libx265.so" (or create symbolic link) according to used x265 API version, e.g. "libx265.so.130"
    -  To check what is expected name use command: ldd hevc_enc_x265_release.so
