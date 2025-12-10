# libtiff TIFF decoder plugin for Dolby Encoding Engine

The provided source code was built and tested using the libtiff version 4.7.0.

## Build tools:
- Visual Studio 2022
- gcc 11.5

## Prerequisites:

To build the plugin, you must first build and install the [libtiff 4.7.0](https://gitlab.com/libtiff/libtiff/-/releases/v4.7.0) `tiff` shared library.
Use `cmake` to build and install libtiff libraries. Navigate to the extracted directory in terminal and invoke:
- See all available configuration options:
  - `cmake -h` 
- Generate project with specific install prefix (and other options):
  - `cmake -B build -S . --install-prefix <INSTALL_PATH> [other options]`
- Build project:
  - `cmake --build build --config Release -j`
- Install project to `INSTALL_PATH`:
  - `cmake --install build`

Once built and installed, define `LIBTIFFROOT` environment variable to point to `INSTALL_PATH` defined during configuration step.
Example `LIBTIFFROOT` directory structure (for Windows) is presented below.

```bash
LIBTIFFROOT
├───bin
│       fax2ps.exe
│       fax2tiff.exe
│       pal2rgb.exe
│       ...
│       tiff.dll
├───include
│       tiff.h
│       tiffconf.h
│       tiffio.h
│       tiffio.hxx
│       tiffvers.h
│
└───lib
    │   tiff.lib
    │   tiffxx.lib
    ...
```

## Build instructions

Build the plugin (see [BUILDING.md](../../BUILDING.md)), then copy the `tiff` shared library and plugin library:
- on Linux: `LIBTIFFROOT/lib/libtiff.so` and `libdee_plugin_tiff_dec_libtiff.so`
- on Windows: `LIBTIFFROOT/bin/tiff.dll` and `libdee_plugin_tiff_dec_libtiff.dll`

to the DEE installation folder. The plugin library file may be renamed, but its file extension must remain unchanged.
