Dolby Encoding Engine plugin for HEVC encoding via ffmpeg.

Build tools: 
- Visual Studio 2013
- gcc 4.8.5

Build instructions, Windows:
1. Build using make\windows_amd64_msv\hevc_enc_ffmpeg_2013.sln
2. Copy .dll library with plugin to DEE installation folder

Build instructions, Linux:
1. Build using make/hevc_enc_ffmpeg/linux_amd64_gnu/Makefile
    - make hevc_enc_ffmpeg_release.so
2. Copy .so library with plugin to DEE installation folder

