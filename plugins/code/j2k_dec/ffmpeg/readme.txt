Dolby Encoding Engine plugin for JPEG2000 decoding via FFmpeg

The provided source code was tested using libopenjpeg decoder within FFmpeg.

Build tools: 
- Visual Studio 2013 or 2015
- gcc 4.8.5

Prerequisites:
- FFmpeg (version 3.4.2) with JPEG2000 decoder

Build instructions, Windows:
1. Build using make\j2k_dec_ffmpeg\windows_amd64_msvs\j2k_dec_ffmpeg_2015.sln
2. Copy the .dll library with the plugin to the Dolby Encoding Engine installation folder.

Build instructions, Linux:
1. Build using make/j2k_dec_ffmpeg/linux_amd64_gnu/Makefile
    - make j2k_dec_ffmpeg_release.so
2. Copy the .so library with the plugin to the Dolby Encoding Engine installation folder.
