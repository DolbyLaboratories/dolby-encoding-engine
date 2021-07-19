# Beamr HEVC encoder plugin for Dolby Encoding Engine

Provided source code was built and tested using Beamr5x-4.7.2.2.

## Build tools

- Visual Studio 2015
- gcc 4.8.5 (or higher)

## Prerequisites

The build must be performed within the complete `dolby-encoding-engine` code tree. Download the whole repository, even if you plan to build only selected plugins.

To build the plugin, Beamr5x SDK is required, and the environment variable `BEAMR_SDK` pointing to the kit folder.
The typical structure of the kit folder is presented below.

```bash
BEAMR_SDK
├── Beamr_5_Installation_Notes.pdf
├── bin
├── common_primitives
│   ├── include
│   └── src
├── inc
├── lib
├── lib64
├── license
└── samples
```

The build uses files in following folders:

- $BEAMR_SDK/common_primitives/include
- $BEAMR_SDK/common_primitives/src
- $BEAMR_SDK/inc
- $BEAMR_SDK/lib
- $BEAMR_SDK/lib64

## Build instructions (Linux)

Extract the kit folder from the archive.

Set environment variable `BEAMR_SDK`.

Build the plugin:

```bash
cd make/hevc_enc_beamr/linux_amd64_gnu
make
```

Copy `hevc_enc_beamr_release.so` to DEE installation folder. The file can be renamed, but the extension must be `.so`.

## Build instructions (Windows)

Extract the kit folder from the archive.

Set environment variable BEAMR_SDK.

Build the plugin using Visual Studio solution in `make\hevc_enc_beamr\windows_amd64_msvs\hevc_enc_beamr_2015.sln`.

Copy `hevc_enc_beamr.dll` to DEE installation folder. The file can be renamed, but the extension must be `.dll`.

## Tuning encoder settings

Provided XML examples present how Beamr plugin shall be used via DEE.
Fine-tuning fo encoder configuration depends on processed content, thus it shall be consulted with Beamr Imaging.

The presented XML interface contains only basic encoder parameters. Other parameters can be set using the following XML elements:

- `native_config_file`
- `param`

These elements allow configuring encoder using native Beamr parameters, which can be found in Beamr SDK documentation and sample configs in `$BEAMR_SDK\samples\*.cfg`.

