add_library(j2k_dec_api INTERFACE)
add_library(dee_plugins::j2k_dec_api ALIAS j2k_dec_api)

target_sources(j2k_dec_api
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS .
        FILES
            j2k_dec_api.h
)

target_link_libraries(j2k_dec_api
    INTERFACE
        dee_plugins::plugins_common
)

include(GNUInstallDirs)
install(TARGETS j2k_dec_api
    EXPORT j2k_dec_apiTargets
    FILE_SET HEADERS
)

install(EXPORT j2k_dec_apiTargets
    NAMESPACE dee_plugins::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/dee_plugins"
)
