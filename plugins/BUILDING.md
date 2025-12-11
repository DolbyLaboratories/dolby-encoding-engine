# Prerequisites

- CMake 3.24 or newer
- gcc 11.5 or Visual Studio 2022

# Instructions

## Configure

By default, this CMake project builds all plugins.

For detailed instructions on where each plugin expects its dependent libraries to be located, please refer to the plugin's README file (e.g. x265 HEVC encoder plugin's [README.md](code/hevc_enc/x265/README.md)).

To exclude specific plugins from the build, use the appropriate CMake option during configuration. For example, to disable the Beamr HEVC encoder plugin:

```bash
cmake -B build -S . -DDEE_PLUGINS_ENABLE_BEAMR_HEVC_ENCODER=OFF
```

Depending on the target platform, it may be necessary to specify options such as the MSVC multi-threaded statically linked runtime library using `-DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded"`, or position-independent code using `-DCMAKE_POSITION_INDEPENDENT_CODE=ON`.

Specifying an option that is unsupported on a given platform will simply be ignored, so it is fine to provide both settings:

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded" -DCMAKE_POSITION_INDEPENDENT_CODE=ON
```

Please refer to [CMakeLists.txt](CMakeLists.txt) for more build options.

## Build

To build the project, run:

```bash
cmake --build build --config Release -j
```