# Kakadu JPEG2000 decoder plugin for Dolby Encoding Engine

The provided source code was built and tested using the Kakadu SDK version 8.4.

## Build tools:
- Visual Studio 2022
- gcc 11.5

## Prerequisites:

To build the plugin, you must first build the Kakadu `coresys` shared library. The build files for `coresys` are typically located in the `coresys/make` directory (for Linux and macOS Makefiles), or directly in the `coresys` folder (for Windows Visual Studio `.sln` files). Once built, define `KDUROOT` environment variable pointing to the Kakadu source tree.

A typical Kakadu SDK directory structure is shown below. Note that, in addition to the `coresys` library, the plugin requires additional source files located in the `apps/support` directory, as well as headers from the `coresys/common` folder.

```bash
├── KDUROOT
│   ├── apps
│   │   ├── support
│   │   │  ├── avx2_stripe_transfer.cpp
│   │   │  ├── kdu_region_animator.h
│   │   │  ├── kdu_region_compositor.h
│   │   │  ├── kdu_region_decompressor.h
│   │   │  ├── kdu_stripe_compressor.h
│   │   │  ├── kdu_stripe_decompressor.cpp
│   │   │  ├── kdu_stripe_decompressor.h
│   │   │  ├── neon_region_compositor_local.h
│   │   │  ├── neon_region_decompressor_local.h
│   │   │  ├── neon_stripe_transfer_local.h
│   │   │  ├── neon_stripe_transfer.cpp
│   │   │  ├── region_animator_local.h
│   │   │  ├── region_compositor_local.h
│   │   │  ├── region_decompressor_local.h
│   │   │  ├── ssse3_stripe_transfer.cpp
│   │   │  ├── stripe_compressor_local.h
│   │   │  ├── stripe_decompressor_local.h
│   │   │  ├── supp_local.cpp
│   │   │  ├── supp_local.h
│   │   │  ├── x86_region_compositor_local.h
│   │   │  ├── x86_region_decompressor_local.h
│   │   │  ├── x86_stripe_transfer_local.h
│   │   │  └── ...
│   │   └── ...
│   ├── coresys
│   │   ├── common
│   │   │  ├── arch_masm64.asm
│   │   │  ├── kdu_arch.cpp
│   │   │  ├── kdu_arch.h
│   │   │  ├── kdu_block_coding.h
│   │   │  ├── kdu_compressed.h
│   │   │  ├── kdu_elementary.h
│   │   │  ├── kdu_kernels.h
│   │   │  ├── kdu_messaging.h
│   │   │  ├── kdu_params.h
│   │   │  ├── kdu_roi_processing.h
│   │   │  ├── kdu_sample_processing.h
│   │   │  ├── kdu_threads.h
│   │   │  ├── kdu_ubiquitous.h
│   │   │  └── kdu_utils.h
│   │   ├── make
│   │   │  ├── Makefile-Linux-x86-64-gcc
│   │   │  ├── Makefile-Mac-x86-64-gcc
│   │   │  ├── Makefile-Mac-arm-64-gcc
│   │   │  └── ...
│   │   └── ...
│   ├── lib
│   │   ├── Linux-x86-64-gcc
│   │   │   └── libkdu_v84R.so
│   │   ├── Mac-x86-64-gcc
│   │   │   └── libkdu_v84R.so
│   │   └── ...
│   └── ...
├── bin_x64
│   └── kdu_v84R.dll
└── lib_x64
    └── kdu_v84R.lib
```

## Build instructions

Set up the Kakadu directory as described above, and set the `KDUROOT` environment variable to point to that folder. Note that on Windows, the binaries are located outside the `KDUROOT` folder, as this is where Kakadu build files place them by default.

Build the plugin (see [BUILDING.md](../../../BUILDING.md)), then copy the Kakadu shared libraries (e.g., `libkdu_v84R.so` or `kdu_v84R.dll`) and the plugin library (e.g., `libdee_plugin_j2k_dec_kakadu.so` or `dee_plugin_j2k_dec_kakadu.dll`) to the DEE installation folder. The plugin library file may be renamed, but its file extension must remain unchanged.
