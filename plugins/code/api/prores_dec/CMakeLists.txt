add_library(prores_dec_api INTERFACE)
add_library(dee_plugins::prores_dec_api ALIAS prores_dec_api)

target_sources(prores_dec_api
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS .
        FILES
            prores_dec_api.h
)

target_link_libraries(prores_dec_api
    INTERFACE
        dee_plugins::plugins_common
)

include(GNUInstallDirs)
install(TARGETS prores_dec_api
    EXPORT prores_dec_apiTargets
    FILE_SET HEADERS
)

install(EXPORT prores_dec_apiTargets
    NAMESPACE dee_plugins::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/dee_plugins"
)
