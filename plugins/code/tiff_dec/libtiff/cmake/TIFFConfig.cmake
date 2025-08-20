include(CMakeFindDependencyMacro)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    if(CMAKE_OSX_ARCHITECTURES)
        if ("x86_64" IN_LIST CMAKE_OSX_ARCHITECTURES AND "arm64" IN_LIST CMAKE_OSX_ARCHITECTURES)
            message(FATAL_ERROR "Building macOS universal binaries is not supported")
        endif()
    endif()

    set(LIBTIFF_NAME "libtiff.dylib")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(LIBTIFF_NAME "tiff.dll")
    set(LIBTIFF_IMPLIB "tiff.lib")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LIBTIFF_NAME "libtiff.so")
else()
    message(FATAL_ERROR "Not supported OS/architecture")
endif()


add_library(TIFF::TIFF SHARED IMPORTED)
set_target_properties(TIFF::TIFF
    PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "$ENV{LIBTIFFROOT}/include"
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_target_properties(TIFF::TIFF
        PROPERTIES
            IMPORTED_LOCATION "$ENV{LIBTIFFROOT}/bin/${LIBTIFF_NAME}"
            IMPORTED_IMPLIB "$ENV{LIBTIFFROOT}/lib/${LIBTIFF_IMPLIB}"
    )
else()
    set_target_properties(TIFF::TIFF
        PROPERTIES
            IMPORTED_LOCATION "$ENV{LIBTIFFROOT}/lib/${LIBTIFF_NAME}"
    )
endif()




