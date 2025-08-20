cmake_minimum_required(VERSION 3.24)

project(dee-plugins)

option(DEE_PLUGINS_ENABLE_BEAMR_HEVC_ENCODER "Enables beamr HEVC encoder" ON)
option(DEE_PLUGINS_ENABLE_X265_HEVC_ENCODER "Enables x265 HEVC encoder" ON)
option(DEE_PLUGINS_ENABLE_KAKADU_J2K_DECODER "Enables kakadu J2K decoder" ON)
option(DEE_PLUGINS_ENABLE_LIBTIFF_TIFF_DECODER "Enables libtiff TIFF decoder" ON)
option(DEE_PLUGINS_ENABLE_DUMMY_IMAGE_TRANSFORMER "Enables dummy image transformer" ON)

include(GNUInstallDirs)

add_subdirectory(code)