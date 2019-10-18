Dolby Encoding Engine plugin example for noise filtering

Build tools: 
- Visual Studio 2013 or 2015
- gcc 4.8.5

Build instructions, Windows:
1. Build using make\noise_example\windows_amd64_msvs\noise_example_2015.sln
2. Copy the .dll library with the plugin to the Dolby Encoding Engine installation folder.

Build instructions, Linux:
1. Build using make/noise_example/linux_amd64_gnu/Makefile
    - make noise_example_release.so
2. Copy the .so library with the plugin to Dolby Encoding Engine installation folder.
