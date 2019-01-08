Dolby Encoding Engine plugin for Apple ProRes decoder

Build tools:
- Visual Studio 2015
- gcc 4.8.5

Prerequisites:
- The "ProRes64.lib" library on Windows
- The "libProRes64.a" library on Linux
- The "ProResDecoder.h" API's header file.

    APPLE_PRORES_LIB_ROOT
    ├── lib
    │   ├── linux64
    │   │   └── libProRes64.a
    │   └── windows64
    │       └── ProRes64.lib
    └── include
        └── ProResDecoder.h

Build instructions, Windows:
1. Set the APPLE_PRORES_LIB_ROOT environment variable pointing to the folder with prerequisites.
2. Build using make\prores_dec_apple\windows_amd64_msvs\prores_dec_apple_2015.sln
2. Copy the .dll library with the plugin to the Dolby Encoding Engine installation folder.

Build instructions, Linux:
1. Set the APPLE_PRORES_LIB_ROOT environment variable pointing to the folder with prerequisites.
2. Build using make/prores_dec_apple/linux_amd64_gnu/Makefile
    - make prores_dec_apple_release.so
3. Copy the .so library with the plugin to the Dolby Encoding Engine installation folder.
