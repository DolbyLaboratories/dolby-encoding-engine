add_library(dee_plugin_image_transformer_dummy SHARED)
add_library(dee_plugins::dee_plugin_image_transformer_dummy ALIAS dee_plugin_image_transformer_dummy)

target_compile_features(dee_plugin_image_transformer_dummy
    PRIVATE
        cxx_std_11
)

target_link_libraries(dee_plugin_image_transformer_dummy
    PRIVATE
        dee_plugins::image_transformer_api
)

install(TARGETS dee_plugin_image_transformer_dummy
    EXPORT dee_plugin_image_transformer_dummyTargets
)

install(EXPORT dee_plugin_image_transformer_dummyTargets
    NAMESPACE dee_plugins::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/dee_plugins"
)

add_subdirectory(src)
