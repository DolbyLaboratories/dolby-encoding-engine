include(CMakeFindDependencyMacro)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    if(CMAKE_OSX_ARCHITECTURES)
        if ("x86_64" IN_LIST CMAKE_OSX_ARCHITECTURES AND "arm64" IN_LIST CMAKE_OSX_ARCHITECTURES)
            message(FATAL_ERROR "Building macOS universal binaries is not supported")
        endif()
    endif()

    set(X265_LIBNAME "libx265.dylib")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(X265_LIBNAME "libx265.dll")
    set(X265_IMPLIB "libx265.lib")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(X265_LIBNAME "libx265.so")
else()
    message(FATAL_ERROR "Not supported OS/architecture")
endif()

add_library(x265::x265 SHARED IMPORTED)

set_target_properties(x265::x265
    PROPERTIES
        IMPORTED_LOCATION "$ENV{X265ROOT}/lib/${X265_LIBNAME}"
        INTERFACE_INCLUDE_DIRECTORIES "$ENV{X265ROOT}/include"
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_target_properties(x265::x265 PROPERTIES
        IMPORTED_IMPLIB "$ENV{X265ROOT}/lib/${X265_IMPLIB}"
    )
endif()