Dolby Encoding Engine plugin for Comprimato JPEG2000 decoder

The provided source code was built and tested using the Comprimato SDK version 2.5.8-release.

Build tools:
- Visual Studio 2013
- gcc 4.8.5

Prerequisites:
- The "cmpto_j2k_dec.lib" and "cmpto_j2k_dec.dll" libraries on Windows
- The "libcmpto_j2k_dec.so" library on Linux
- The "cmpto_j2k_dec.h" API's header file.

    COMPTIMATOROOT
    ├── library files (based on platform)
    └── cmpto_j2k_dec.h

Build instructions, Windows:
1. Set the COMPRIMATOROOT environment variable pointing to the folder with prerequisites.
2. Build using make\j2k_dec_comprimato\windows_amd64_msvs\j2k_dec_comprimato_2013.sln
2. Copy the .dll library with the plugin and "cmpto_j2k_dec.dll" to the Dolby Encoding Engine installation folder.

Build instructions, Linux:
1. Set the COMPRIMATOROOT environment variable pointing to the folder with prerequisites.
2. Build using make/j2k_dec_comprimato/linux_amd64_gnu/Makefile
    - make j2k_dec_comprimato_release.so
3. Copy the .so library with the plugin and "libcmpto_j2k_dec.so" to the Dolby Encoding Engine installation folder.
