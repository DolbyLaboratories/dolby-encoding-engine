add_library(plugins_common INTERFACE)
add_library(dee_plugins::plugins_common ALIAS plugins_common)

target_sources(plugins_common
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS .
        FILES
            plugins_common.h
)

include(GNUInstallDirs)
install(TARGETS plugins_common
    EXPORT plugins_commonTargets
    FILE_SET HEADERS
)

install(EXPORT plugins_commonTargets
    NAMESPACE dee_plugins::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/dee_plugins"
)
