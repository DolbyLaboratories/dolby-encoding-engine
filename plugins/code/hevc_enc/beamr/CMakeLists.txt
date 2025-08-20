list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/")
find_package(beamr REQUIRED COMPONENTS encoder common_primitives)

add_library(dee_plugin_hevc_enc_beamr SHARED)
add_library(dee_plugins::dee_plugin_hevc_enc_beamr ALIAS dee_plugin_hevc_enc_beamr)

target_compile_features(dee_plugin_hevc_enc_beamr
    PRIVATE
        cxx_std_11
)

target_link_libraries(dee_plugin_hevc_enc_beamr
    PRIVATE
        dee_plugins::plugins_debugger
        dee_plugins::hevc_enc_api
        beamr::encoder
        beamr::common_primitives
)

install(TARGETS dee_plugin_hevc_enc_beamr
    EXPORT dee_plugin_hevc_enc_beamrTargets
)

install(EXPORT dee_plugin_hevc_enc_beamrTargets
    NAMESPACE dee_plugins::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/dee_plugins"
)

add_subdirectory(src)