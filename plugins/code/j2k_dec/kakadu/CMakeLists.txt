list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/")
find_package(kakadu REQUIRED)

add_library(dee_plugin_j2k_dec_kakadu_base STATIC)
add_library(dee_plugins::dee_plugin_j2k_dec_kakadu_base ALIAS dee_plugin_j2k_dec_kakadu_base)

target_sources(dee_plugin_j2k_dec_kakadu_base
    PUBLIC
        FILE_SET HEADERS
        TYPE HEADERS
        BASE_DIRS src
)

target_link_libraries(dee_plugin_j2k_dec_kakadu_base
    PUBLIC
        dee_plugins::j2k_dec_api
    PRIVATE
        kakadu::kakadu
)

install(TARGETS dee_plugin_j2k_dec_kakadu_base
    EXPORT dee_plugin_j2k_dec_kakadu_baseTargets
    FILE_SET HEADERS
)

install(EXPORT dee_plugin_j2k_dec_kakadu_baseTargets
    NAMESPACE dee_plugins::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/dee_plugins"
)

add_library(dee_plugin_j2k_dec_kakadu SHARED)
add_library(dee_plugins::dee_plugin_j2k_dec_kakadu ALIAS dee_plugin_j2k_dec_kakadu)

target_link_libraries(dee_plugin_j2k_dec_kakadu
    PRIVATE
        dee_plugins::dee_plugin_j2k_dec_kakadu_base
)

install(TARGETS dee_plugin_j2k_dec_kakadu
    EXPORT dee_plugin_j2k_dec_kakaduTargets
)

install(EXPORT dee_plugin_j2k_dec_kakaduTargets
    NAMESPACE dee_plugins::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/dee_plugins"
)

add_subdirectory(src)
