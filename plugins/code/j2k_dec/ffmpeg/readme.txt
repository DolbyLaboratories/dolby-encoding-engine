Dolby Encoding Engine plugin for JPEG2000 decoding via ffmpeg.
Provided source code was tested using libopenjpeg decoder within ffmpeg.

Build tools: 
- Visual Studio 2013
- gcc 4.8.5

Prerequisites:
- ffmpeg (version 3.4.2) with jpeg2000 decoder

Build instructions, Windows:
1. Build using make\j2k_dec_ffmpeg\windows_amd64_msvs\j2k_dec_ffmpeg_2013.sln
2. Copy dll library with plugin to DEE installation folder

Build instructions, Linux:
1. Build using make/j2k_dec_ffmpeg/linux_amd64_gnu/Makefile
    - make j2k_dec_ffmpeg_release.so
2. Copy so library with plugin to DEE installation folder
