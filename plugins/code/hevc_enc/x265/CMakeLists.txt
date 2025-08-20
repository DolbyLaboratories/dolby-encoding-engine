list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/")
find_package(x265 REQUIRED)

add_library(dee_plugin_hevc_enc_x265 SHARED)
add_library(dee_plugins::dee_plugin_hevc_enc_x265 ALIAS dee_plugin_hevc_enc_x265)

target_compile_features(dee_plugin_hevc_enc_x265
    PRIVATE
        cxx_std_11
)

target_link_libraries(dee_plugin_hevc_enc_x265
    PRIVATE
        dee_plugins::hevc_enc_api
        x265::x265
)

install(TARGETS dee_plugin_hevc_enc_x265
    EXPORT dee_plugin_hevc_enc_x265Targets
)

install(EXPORT dee_plugin_hevc_enc_x265Targets
    NAMESPACE dee_plugins::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/dee_plugins"
)

add_subdirectory(src)
