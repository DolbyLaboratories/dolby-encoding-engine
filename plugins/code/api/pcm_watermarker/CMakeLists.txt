add_library(pcm_watermarker_api INTERFACE)
add_library(dee_plugins::pcm_watermarker_api ALIAS pcm_watermarker_api)

target_sources(pcm_watermarker_api
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS .
        FILES
        pcm_watermarker_api.h
)

target_link_libraries(pcm_watermarker_api
    INTERFACE
        dee_plugins::plugins_common
)

include(GNUInstallDirs)
install(TARGETS pcm_watermarker_api
    EXPORT pcm_watermarker_apiTargets
    FILE_SET HEADERS
)

install(EXPORT pcm_watermarker_apiTargets
    NAMESPACE dee_plugins::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/dee_plugins"
)
