# x265 HEVC encoder plugin for Dolby Encoding Engine

The provided source code was built and tested using the x265 library version 4.1.

## Requirements:
- Windows:
  - Visual Studio 2022
  - [NASM](https://nasm.us/pub/nasm/releasebuilds/?C=M;O=D)
  - [CMake](https://github.com/Kitware/CMake/releases/tag/v3.31.8) 
- Linux:
  - gcc 11.5
  - [NASM](https://nasm.us/pub/nasm/releasebuilds/?C=M;O=D)
  - [CMake](https://github.com/Kitware/CMake/releases/tag/v3.31.8) 

## Building x265 library
 The x265 library has to be build with multi-bitdepth (8/10/12-bit) support and in shared format.  
 - Navigate to [x265 git repository](https://bitbucket.org/multicoreware/x265_git/src/4.1/build/) and locate `multilib` file for your platform.
 - Execute `multilib` file to build `x265` library.
 - Multi-bitdepth library will be located in newly created `8bit` directory

## Building x265 plugin
- Setup the library directory as pesented below. Copy necessary files from the [previous step](#building-x265-library):
  ```bash
  x265_root
  ├── include
  │   ├── x265_config.h
  │   └── x265.h
  └── lib
      └── libx265.so or libx265.lib
  ```

- Set `X265ROOT` environment variable pointing to `x265_root`.
- Build the plugin (see [BUILDING.md](../../../BUILDING.md))
- Copy x265 shared library (`libx265.so` or `libx265.dll`)
and plugin library (`libdee_plugin_hevc_enc_x265.so` or `dee_plugin_hevc_enc_x265.dll`) to the DEE installation folder. The plugin library file can be renamed, but the extension must remain unchanged.
