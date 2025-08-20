if(DEE_PLUGINS_ENABLE_BEAMR_HEVC_ENCODER)
    add_subdirectory(beamr)
endif()

if(DEE_PLUGINS_ENABLE_X265_HEVC_ENCODER)
    add_subdirectory(x265)
endif()