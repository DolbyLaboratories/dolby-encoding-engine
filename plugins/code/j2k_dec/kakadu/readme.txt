Dolby Encoding Engine plugin for Kakadu JPEG2000 decoder

The provided source code was built and tested using the Kakadu SDK version 7.A.

Build tools: 
- Visual Studio 2013
- gcc 4.8.5

Prerequisites:
- The "kdu_v7AR.lib" and "kdu_v7AR.dll" libraries on Windows
- The "libkdu_v7AR.so" library on Linux
- The "common" folder with SDK headers:
    - arch_masm64.asm
    - kdu_arch.h
    - kdu_block_coding.h
    - kdu_compressed.h
    - kdu_elementary.h
    - kdu_kernels.h
    - kdu_messaging.h
    - kdu_params.h
    - kdu_roi_processing.h
    - kdu_sample_processing.h
    - kdu_threads.h
    - kdu_ubiquitous.h
    - kdu_utils.h
- The "support" folder with SDK support code:
    - avx2_stripe_transfer.cpp
    - kdu_stripe_decompressor.cpp
    - kdu_stripe_decompressor.h
    - ssse3_stripe_transfer.cpp
    - supp_local.cpp (not needed for library v7.9)
    - stripe_decompressor_local.h
    - x86_stripe_transfer_local.h

    KDUROOT
    ├── common
    │   ├── arch_masm64.asm
    │   ├── kdu_arch.cpp
    │   ├── kdu_arch.h
    │   ├── kdu_block_coding.h
    │   ├── kdu_compressed.h
    │   ├── kdu_elementary.h
    │   ├── kdu_kernels.h
    │   ├── kdu_messaging.h
    │   ├── kdu_params.h
    │   ├── kdu_roi_processing.h
    │   ├── kdu_sample_processing.h
    │   ├── kdu_threads.h
    │   ├── kdu_ubiquitous.h
    │   └── kdu_utils.h
    ├── lib
    │   ├── linux64
    │   │   └── libkdu_v7AR.so
    │   └── windows64
    │       ├── kdu_v7AR.dll
    │       └── kdu_v7AR.lib
    └── support
        ├── avx2_stripe_transfer.cpp
        ├── kdu_stripe_decompressor.cpp
        ├── supp_local.cpp
        ├── kdu_stripe_decompressor.h
        ├── ssse3_stripe_transfer.cpp
        ├── stripe_decompressor_local.h
        └── x86_stripe_transfer_local.h

Build instructions, Windows:
1. Set the KDUROOT environment variable pointing to the folder with prerequisites.
2. Build using make\j2k_dec_kakadu\windows_amd64_msvs\j2k_dec_kakadu_2013.sln
2. Copy the .dll library with the plugin and "libkdu_v7AR.dll" to the Dolby Encoding Engine installation folder.

Build instructions, Linux:
1. Set the KDUROOT environment variable pointing to the folder with prerequisites.
2. Build using make/j2k_dec_kakadu/linux_amd64_gnu/Makefile
    - make j2k_dec_kakadu_release.so
3. Copy the .so library with the plugin and "libkdu_v7AR.so" to the Dolby Encoding Engine installation folder.
