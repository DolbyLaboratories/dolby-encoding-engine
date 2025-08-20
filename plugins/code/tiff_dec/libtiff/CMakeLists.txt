list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/")

find_package(TIFF CONFIG REQUIRED)
add_library(dee_plugin_tiff_dec_libtiff SHARED)
add_library(dee_plugins::dee_plugin_tiff_dec_libtiff ALIAS dee_plugin_tiff_dec_libtiff)

target_compile_features(dee_plugin_tiff_dec_libtiff
    PRIVATE
        cxx_std_11
)

target_link_libraries(dee_plugin_tiff_dec_libtiff
    PRIVATE
        dee_plugins::tiff_dec_api
        TIFF::TIFF
)

install(TARGETS dee_plugin_tiff_dec_libtiff
    EXPORT dee_plugin_tiff_dec_libtiffTargets
)

install(EXPORT dee_plugin_tiff_dec_libtiffTargets
    NAMESPACE dee_plugins::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/dee_plugins"
)

add_subdirectory(src)
