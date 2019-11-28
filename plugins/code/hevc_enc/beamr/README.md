# Beamr HEVC encoder plugin for Dolby Encoding Engine

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

Copy `hevc_enc_beamr_release.so` to DEE installation folder. File can be renamed, but extension must be `.so`.

Additionally, copy `$BEAMR_SDK/lib64/libtbb.so.2` to DEE installation folder. Name of that file cannot be changed.

## Build instructions (Windows)

Extract the kit folder from the archive.

Set environment variable BEAMR_SDK.

Build the plugin using Visual Studio solution in `make\windows_amd64_msvs\hevc_enc_beamr_2015.sln`.

Copy `hevc_enc_beamr.dll` to DEE installation folder. File can be renamed, but extension must be `.dll`.

Additionally, copy `$BEAMR_SDK/bin/Intel_IPP_Win64/tbb.dll` to DEE installation folder. Name of that file cannot be changed.

## Tuning encoder settings

Provided XML examples present how Beamr plugin shall be used via DEE.
Fine-tuning fo encoder configuration depends on processed content, thus it shall be consulted with Beamr Imaging.

Presented XML interface contains only basic encoder parameters. Other parameters can be set using following XML elements:

- `native_config_file`
- `param`

These elements allow to configure encoder using native Beamr parameters, which can be found in Beamr SDK documentation and sample configs in `$BEAMR_SDK\samples\*.cfg`.

## Known issues

- When encoding Enhancement layer for Dolby Vision profile 7, encoder forces tier Main instead fo High.
  - This issue shall be fixed with next release of Beamr SDK.
