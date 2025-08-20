include(CMakeFindDependencyMacro)

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

    set(KAKADU_LIBNAME "libkdu_v84R.so")
    if(MACOS_ARCH STREQUAL "arm64")
        set(KAKADU_LIBDIR "lib/Mac-arm-64-gcc")
    elseif(MACOS_ARCH STREQUAL "x86_64")
        set(KAKADU_LIBDIR "lib/Mac-x86-64-gcc")
    else()
        message(FATAL_ERROR "Unsupported macOS architecture")
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(KAKADU_LIBDIR "../bin_x64")
    set(KAKADU_LIBNAME "kdu_v84R.dll")
    set(KAKADU_IMPLIBDIR "../lib_x64")
    set(KAKADU_IMPLIBNAME "kdu_v84R.lib")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(KAKADU_LIBDIR "lib/Linux-x86-64-gcc")
    set(KAKADU_LIBNAME "libkdu_v84R.so")
else()
    message(FATAL_ERROR "Not supported OS/architecture")
endif()

add_library(kakadu::kdu SHARED IMPORTED)
set_target_properties(kakadu::kdu
    PROPERTIES
        IMPORTED_LOCATION "$ENV{KDUROOT}/${KAKADU_LIBDIR}/${KAKADU_LIBNAME}"
        INTERFACE_INCLUDE_DIRECTORIES "$ENV{KDUROOT}/coresys/common"
)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_target_properties(kakadu::kdu
        PROPERTIES
            IMPORTED_IMPLIB "$ENV{KDUROOT}/${KAKADU_IMPLIBDIR}/${KAKADU_IMPLIBNAME}"
    )
endif()

add_library(support STATIC)
add_library(kakadu::support ALIAS support)
target_include_directories(support
    PUBLIC
        "$ENV{KDUROOT}/apps/support"
)
target_sources(support
    PRIVATE
        "$ENV{KDUROOT}/apps/support/kdu_stripe_decompressor.cpp"
        "$ENV{KDUROOT}/apps/support/avx2_stripe_transfer.cpp"
        "$ENV{KDUROOT}/apps/support/ssse3_stripe_transfer.cpp"
        "$ENV{KDUROOT}/apps/support/neon_stripe_transfer.cpp"
        "$ENV{KDUROOT}/apps/support/supp_local.cpp"
)
target_link_libraries(support
    PRIVATE
        kakadu::kdu
)

add_library(kakadu::kakadu INTERFACE IMPORTED)
target_link_libraries(kakadu::kakadu
    INTERFACE
        kakadu::kdu
        kakadu::support
)