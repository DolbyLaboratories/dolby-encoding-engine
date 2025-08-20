target_sources(dee_plugin_j2k_dec_kakadu_base
    PUBLIC
        FILE_SET HEADERS
        FILES
            j2k_dec_kakadu.h
    PRIVATE
        j2k_dec_kakadu.cpp
)

target_sources(dee_plugin_j2k_dec_kakadu
    PRIVATE
        j2k_dec_kakadu_plugin.cpp
)
