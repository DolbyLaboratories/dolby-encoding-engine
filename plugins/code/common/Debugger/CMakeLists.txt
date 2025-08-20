add_library(plugins_debugger INTERFACE)
add_library(dee_plugins::plugins_debugger ALIAS plugins_debugger)

target_sources(plugins_debugger
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS .
        FILES
            plugins_debugger.h
)

include(GNUInstallDirs)
install(TARGETS plugins_debugger
    EXPORT plugins_debuggerTargets
    FILE_SET HEADERS
)

install(EXPORT plugins_debuggerTargets
    NAMESPACE dee_plugins::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/dee_plugins"
)
