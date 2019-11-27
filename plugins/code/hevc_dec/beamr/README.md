# Beamr HEVC decoder plugin for Dolby Encoding Engine

The provided source code was built and tested using Beamr5x-4.5.0.3.

## Build tools

- Visual Studio 2015
- gcc 4.8.5 (or higher)

## Prerequisites

To build Beamr SDK and environment variable BEAMR_SDK pointing to the kit folder.
Typical structure of the kit folder is presented below. The build requires files in following folders:

- $BEAMR_SDK/common_primitives/include
- $BEAMR_SDK/common_primitives/src
- $BEAMR_SDK/inc
- $BEAMR_SDK/lib
- $BEAMR_SDK/lib64

```bash
BEAMR_SDK
├── Beamr_5_Installation_Notes.pdf
├── bin
├── common_primitives
│   ├── include
│   └── src
├── inc
├── lib
├── lib64
├── license
└── samples
```

## Build instructions (Linux)

Extract the kit folder from the archive.

Set environment variable BEAMR_SDK.

Build the plugin:

```bash
cd make/linux_amd64_gnu
make
```

Copy hevc_dec_beamr_release.so to DEE installation folder. File can be renamed, but extension must be `.so`.

## Build instructions (Windows)

Extract the kit folder from the archive.

Set environment variable BEAMR_SDK.

Build the plugin using Visual Studio solution in `make/windows_amd64_msvs/hevc_dec_beamr_2015.sln`.

Copy hevc_dec_beamr.dll to DEE installation folder. File can be renamed, but extension must be `.dll`.
