Dolby Encoding Engine plugin for LibTIFF decoder

The provided source code was built and tested using the LibTIFF version 4.0.9.

Build tools:
- Visual Studio 2013 or 2015
- gcc 4.8.5

Prerequisites:
- The "tiff.lib" and "tiff.dll" libraries on Windows
- The "libtiff.so" library on Linux
- The "include" folder with SDK headers:
    - t4.h
    - tif_config.h
    - tif_config.vc.h
    - tif_config.wince.h
    - tif_dir.h
    - tif_fax3.h
    - tiffconf.h
    - tiffconf.vc.h
    - tiffconf.wince.h
    - tiff.h
    - tiffio.h
    - tiffio.hxx
    - tiffiop.h
    - tiffvers.h
    - tif_predict.h
    - uvcode.h

    LIBTIFFROOT
    ├── include
    │   ├── t4.h
    │   ├── tif_config.h
    │   ├── tif_config.vc.h
    │   ├── tif_config.wince.h
    │   ├── tif_dir.h
    │   ├── tif_fax3.h
    │   ├── tiffconf.h
    │   ├── tiffconf.vc.h
    │   ├── tiffconf.wince.h
    │   ├── tiff.h
    │   ├── tiffio.h
    │   ├── tiffio.hxx
    │   ├── tiffiop.h
    │   ├── tiffvers.h
    │   ├── tif_predict.h
    │   └── uvcode.h
    └── lib
        ├── linux64
        │   └── libtiff.so
        └── windows64
            ├── tiff.dll
            └── tiff.lib

Build instructions, Windows:
1. Set the LIBTIFFROOT environment variable pointing to the folder with prerequisites.
2. Build using make\tiff_dec_libtiff\windows_amd64_msvs\tiff_dec_libtiff_2015.sln
2. Copy the .dll library with the plugin and "tiff.dll" to the Dolby Encoding Engine installation folder.

Build instructions, Linux:
1. Set the LIBTIFFROOT environment variable pointing to the folder with prerequisites.
2. Build using make/tiff_dec_libtiff/linux_amd64_gnu/Makefile
    - make tiff_dec_libtiff_release.so
3. Copy the .so library with the plugin and "libtiff.so" to the Dolby Encoding Engine installation folder.
