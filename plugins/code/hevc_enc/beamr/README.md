# Beamr HEVC encoder plugin for Dolby Encoding Engine

Provided source code was built and tested using Beamr5x-4.7.3.4.

## Build tools

- Visual Studio 2019
- gcc 9.4.0 (or higher)

## Prerequisites

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

## Build instructions

Extract the kit folder from the archive and set the `BEAMR_SDK` environment variable.

Build the plugin (see [BUILDING.md](../../../BUILDING.md)), then copy `libdee_plugin_hevc_enc_beamr.so` or `libdee_plugin_hevc_enc_beamr.dll` (depending on your operating system) to the DEE installation folder. The file can be renamed, but the extension must remain unchanged.
