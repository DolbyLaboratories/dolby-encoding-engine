add_library(hevc_enc_api INTERFACE)
add_library(dee_plugins::hevc_enc_api ALIAS hevc_enc_api)

target_sources(hevc_enc_api
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS .
        FILES
            hevc_enc_api.h
)

target_link_libraries(hevc_enc_api
    INTERFACE
        dee_plugins::plugins_common
)

include(GNUInstallDirs)
install(TARGETS hevc_enc_api
    EXPORT hevc_enc_apiTargets
    FILE_SET HEADERS
)

install(EXPORT hevc_enc_apiTargets
    NAMESPACE dee_plugins::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/dee_plugins"
)
