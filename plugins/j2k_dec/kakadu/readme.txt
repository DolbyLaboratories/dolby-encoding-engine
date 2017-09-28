Dolby Encoding Engine plugin for Kakadu JPEG2000 decoder.
Provided source code was built and tested using Kakadu SDK version 7.9.

Build tools: 
- Visual Studio 2013
- gcc 4.8.5

Prerequisites:
- Libraries "kdu_v79R.lib" and "kdu_v79R.dll" on Windows
- Library "libkdu_v79R.so" on Linux
- Folder "common" with SDK headers:
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
- Folder "support" with SDK support code:
    - avx2_stripe_transfer.cpp
    - kdu_stripe_decompressor.cpp
    - kdu_stripe_decompressor.h
    - ssse3_stripe_transfer.cpp
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
    │   │   └── libkdu_v79R.so
    │   └── windows64
    │       ├── kdu_v79R.dll
    │       └── kdu_v79R.lib
    └── support
        ├── avx2_stripe_transfer.cpp
        ├── kdu_stripe_decompressor.cpp
        ├── kdu_stripe_decompressor.h
        ├── ssse3_stripe_transfer.cpp
        ├── stripe_decompressor_local.h
        └── x86_stripe_transfer_local.h

Build instructions, Windows:
1. Set environment variable KDUROOT pointing to folder with prerequisites
2. Build using make\j2k_dec_kakadu\windows_amd64_msvs\j2k_dec_kakadu_2013.sln
2. Copy .dll library with plugin  and "libkdu_v79R.dll" to DEE installation folder

Build instructions, Linux:
1. Set environment variable KDUROOT pointing to folder with prerequisites
2. Build using make/j2k_dec_kakadu/linux_amd64_gnu/Makefile
    - make j2k_dec_kakadu_release.so
3. Copy .so library with plugin and "libkdu_v79R.so" to DEE installation folder
