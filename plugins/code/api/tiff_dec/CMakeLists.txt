add_library(tiff_dec_api INTERFACE)
add_library(dee_plugins::tiff_dec_api ALIAS tiff_dec_api)

target_sources(tiff_dec_api
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS .
        FILES
            tiff_dec_api.h
)

target_link_libraries(tiff_dec_api
    INTERFACE
        dee_plugins::plugins_common
)

include(GNUInstallDirs)
install(TARGETS tiff_dec_api
    EXPORT tiff_dec_apiTargets
    FILE_SET HEADERS
)

install(EXPORT tiff_dec_apiTargets
    NAMESPACE dee_plugins::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/dee_plugins"
)
