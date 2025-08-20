add_library(image_transformer_api INTERFACE)
add_library(dee_plugins::image_transformer_api ALIAS image_transformer_api)

target_sources(image_transformer_api
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS .
        FILES
            image_transformer_api.h
)

target_link_libraries(image_transformer_api
    INTERFACE
        dee_plugins::plugins_common
)

include(GNUInstallDirs)
install(TARGETS image_transformer_api
    EXPORT image_transformer_apiTargets
    FILE_SET HEADERS
)

install(EXPORT image_transformer_apiTargets
    NAMESPACE dee_plugins::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/dee_plugins"
)
