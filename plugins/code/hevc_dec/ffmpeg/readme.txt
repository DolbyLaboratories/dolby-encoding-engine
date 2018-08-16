Dolby Encoding Engine plugin for HEVC decoding via FFmpeg

Build tools: 
- Visual Studio 2013
- gcc 4.8.5

Build instructions, Windows:
1. Build using make\windows_amd64_msv\hevc_dec_ffmpeg_2013.sln
2. Copy the .dll library with the plugin to the Dolby Encoding Engine installation folder.

Build instructions, Linux:
1. Build using make/hevc_dec_ffmpeg/linux_amd64_gnu/Makefile
    - make hevc_dec_ffmpeg_release.so
2. Copy the .so library with plugin to the Dolby Encoding Engine installation folder.

The minimum required FFmpeg version is 3.4.2. Previous versions
are built without the necessary 10-bit support on Windows.