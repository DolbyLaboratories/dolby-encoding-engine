include(CMakeFindDependencyMacro)

add_library(beamr::common_primitives INTERFACE IMPORTED)
target_include_directories(beamr::common_primitives
    INTERFACE
        "$ENV{BEAMR_SDK}/common_primitives/include"
)
target_sources(beamr::common_primitives
    INTERFACE
        "$ENV{BEAMR_SDK}/common_primitives/src/AllocationPool.cpp"
        "$ENV{BEAMR_SDK}/common_primitives/src/vh3_default_callbacks.cpp"
        "$ENV{BEAMR_SDK}/common_primitives/src/vh3_thread_manager.cpp"
)

add_library(beamr::common INTERFACE IMPORTED)
target_link_libraries(beamr::common
    INTERFACE
        beamr::hevc-cmn
        beamr::vpl
        beamr::vsl
        $<$<PLATFORM_ID:Windows>:beamr::ircmt>
        $<$<PLATFORM_ID:Windows>:beamr::svml_dispmt>
        $<$<PLATFORM_ID:Windows>:beamr::mmt>
        $<$<BOOL:${BEAMR_IRC_LIBNAME}>:beamr::irc>
        $<$<BOOL:${BEAMR_IMF_LIBNAME}>:beamr::imf>
        $<$<BOOL:${BEAMR_SVML_LIBNAME}>:beamr::svml>
        beamr::common_primitives
)

add_library(beamr::encoder INTERFACE IMPORTED)
target_link_libraries(beamr::encoder
    INTERFACE
        beamr::hevc-enc
        beamr::common
)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    if(CMAKE_OSX_ARCHITECTURES)
        if ("x86_64" IN_LIST CMAKE_OSX_ARCHITECTURES AND "arm64" IN_LIST CMAKE_OSX_ARCHITECTURES)
            message(FATAL_ERROR "Building macOS universal binaries is not supported")
        elseif(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
            set(MACOS_ARCH "arm64")
        elseif(CMAKE_OSX_ARCHITECTURES STREQUAL "x86_64")
            set(MACOS_ARCH "x86_64")
        endif()
    else()
        set(MACOS_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
    endif()

    if(MACOS_ARCH STREQUAL "arm64")
        set(BEAMR_HEVC_ENC_LIBNAME "libhevc-enc-marm64g.a")
        set(BEAMR_HEVC_CMN_LIBNAME "libhevc-cmn-marm64g.a")
        set(BEAMR_VPL_LIBNAME "libvpl-marm64g.a")
        set(BEAMR_VSL_LIBNAME "libvsl-marm64g.a")
    elseif(MACOS_ARCH STREQUAL "x86_64")
        set(BEAMR_HEVC_ENC_LIBNAME "libhevc-enc-m64g.a")
        set(BEAMR_HEVC_CMN_LIBNAME "libhevc-cmn-m64g.a")
        set(BEAMR_VPL_LIBNAME "libvpl-m64g.a")
        set(BEAMR_VSL_LIBNAME "libvsl-m64g.a")
    else()
        message(FATAL_ERROR "Unsupported macOS architecture")
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(BEAMR_HEVC_ENC_LIBNAME "libhevc-enc-w64i-MT.lib")
    set(BEAMR_HEVC_CMN_LIBNAME "libhevc-cmn-w64i-MT.lib")
    set(BEAMR_VPL_LIBNAME "libvpl-w64i-MT.lib")
    set(BEAMR_VSL_LIBNAME "libvsl-w64i-MT.lib")
    set(BEAMR_IRCMT_LIBNAME "libircmt.lib")
    set(BEAMR_SVML_DISPMT_LIBNAME "svml_dispmt.lib")
    set(BEAMR_MMT_LIBNAME "libmmt.lib")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64)")
        set(BEAMR_HEVC_ENC_LIBNAME "libhevc-enc-l64i.a")
        set(BEAMR_HEVC_CMN_LIBNAME "libhevc-cmn-l64i.a")
        set(BEAMR_VPL_LIBNAME "libvpl-l64i.a")
        set(BEAMR_VSL_LIBNAME "libvsl-l64i.a")
        set(BEAMR_IRC_LIBNAME "libirc.a")
        set(BEAMR_IMF_LIBNAME "libimf.a")
        set(BEAMR_SVML_LIBNAME "libsvml.a")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm|aarch64)")
        set(BEAMR_HEVC_ENC_LIBNAME "libhevc-enc-larm64g.a")
        set(BEAMR_HEVC_CMN_LIBNAME "libhevc-cmn-larm64g.a")
        set(BEAMR_VPL_LIBNAME "libvpl-larm64g.a")
        set(BEAMR_VSL_LIBNAME "libvsl-larm64g.a")
    else()
        message(FATAL_ERROR "Unsupported linux architecture: ${CMAKE_SYSTEM_PROCESSOR}")
    endif()
else()
    message(FATAL_ERROR "Not supported OS/architecture")
endif()

add_library(beamr::hevc-enc STATIC IMPORTED)
set_target_properties(beamr::hevc-enc
    PROPERTIES
        IMPORTED_LOCATION "$ENV{BEAMR_SDK}/lib/${BEAMR_HEVC_ENC_LIBNAME}"
        INTERFACE_INCLUDE_DIRECTORIES "$ENV{BEAMR_SDK}/inc"
)
target_link_options(beamr::hevc-enc
    INTERFACE
        "$<$<PLATFORM_ID:Windows>:/NODEFAULTLIB:libirc.lib;/NODEFAULTLIB:svml_disp.lib;/NODEFAULTLIB:libm.lib>"
)

add_library(beamr::hevc-cmn STATIC IMPORTED)
set_target_properties(beamr::hevc-cmn
    PROPERTIES
        IMPORTED_LOCATION "$ENV{BEAMR_SDK}/lib/${BEAMR_HEVC_CMN_LIBNAME}"
        INTERFACE_INCLUDE_DIRECTORIES "$ENV{BEAMR_SDK}/inc"
)
target_link_options(beamr::hevc-cmn
    INTERFACE
        "$<$<PLATFORM_ID:Windows>:/NODEFAULTLIB:libirc.lib;/NODEFAULTLIB:svml_disp.lib;/NODEFAULTLIB:libm.lib>"
)

add_library(beamr::vpl STATIC IMPORTED)
set_target_properties(beamr::vpl
    PROPERTIES
        IMPORTED_LOCATION "$ENV{BEAMR_SDK}/lib/${BEAMR_VPL_LIBNAME}"
        INTERFACE_INCLUDE_DIRECTORIES "$ENV{BEAMR_SDK}/inc"
)
target_link_options(beamr::vpl
    INTERFACE
        "$<$<PLATFORM_ID:Windows>:/NODEFAULTLIB:libirc.lib;/NODEFAULTLIB:svml_disp.lib;/NODEFAULTLIB:libm.lib>"
)

add_library(beamr::vsl STATIC IMPORTED)
set_target_properties(beamr::vsl
    PROPERTIES
        IMPORTED_LOCATION "$ENV{BEAMR_SDK}/lib/${BEAMR_VSL_LIBNAME}"
        INTERFACE_INCLUDE_DIRECTORIES "$ENV{BEAMR_SDK}/inc"
)
target_link_options(beamr::vsl
    INTERFACE
        "$<$<PLATFORM_ID:Windows>:/NODEFAULTLIB:libirc.lib;/NODEFAULTLIB:svml_disp.lib;/NODEFAULTLIB:libm.lib>"
)

if(BEAMR_IRCMT_LIBNAME)
    add_library(beamr::ircmt STATIC IMPORTED)
    set_target_properties(beamr::ircmt
        PROPERTIES
            IMPORTED_LOCATION "$ENV{BEAMR_SDK}/lib64/${BEAMR_IRCMT_LIBNAME}"
            INTERFACE_INCLUDE_DIRECTORIES "$ENV{BEAMR_SDK}/inc"
    )
endif()

if(BEAMR_SVML_DISPMT_LIBNAME)
    add_library(beamr::svml_dispmt STATIC IMPORTED)
    set_target_properties(beamr::svml_dispmt
        PROPERTIES
            IMPORTED_LOCATION "$ENV{BEAMR_SDK}/lib64/${BEAMR_SVML_DISPMT_LIBNAME}"
            INTERFACE_INCLUDE_DIRECTORIES "$ENV{BEAMR_SDK}/inc"
    )
endif()

if(BEAMR_MMT_LIBNAME)
    add_library(beamr::mmt STATIC IMPORTED)
    set_target_properties(beamr::mmt
        PROPERTIES
            IMPORTED_LOCATION "$ENV{BEAMR_SDK}/lib64/${BEAMR_MMT_LIBNAME}"
            INTERFACE_INCLUDE_DIRECTORIES "$ENV{BEAMR_SDK}/inc"
    )
endif()

if(BEAMR_IRC_LIBNAME)
    add_library(beamr::irc STATIC IMPORTED)

    set_target_properties(beamr::irc
        PROPERTIES
            IMPORTED_LOCATION "$ENV{BEAMR_SDK}/lib64/${BEAMR_IRC_LIBNAME}"
            INTERFACE_INCLUDE_DIRECTORIES "$ENV{BEAMR_SDK}/inc"
    )
endif()

if(BEAMR_IMF_LIBNAME)
    add_library(beamr::imf STATIC IMPORTED)

    set_target_properties(beamr::imf
        PROPERTIES
            IMPORTED_LOCATION "$ENV{BEAMR_SDK}/lib64/${BEAMR_IMF_LIBNAME}"
            INTERFACE_INCLUDE_DIRECTORIES "$ENV{BEAMR_SDK}/inc"
    )
endif()

if(BEAMR_SVML_LIBNAME)
    add_library(beamr::svml STATIC IMPORTED)

    set_target_properties(beamr::svml
        PROPERTIES
            IMPORTED_LOCATION "$ENV{BEAMR_SDK}/lib64/${BEAMR_SVML_LIBNAME}"
            INTERFACE_INCLUDE_DIRECTORIES "$ENV{BEAMR_SDK}/inc"
    )
endif()
