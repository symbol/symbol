# Sanitize project toolchain file if any.
# IMPORTANT ! This MUST be done before any project() or find_package() calls
# as after any of the above the CMAKE_TOOLCHAIN_FILE is locked in.

if(DEFINED CMAKE_TOOLCHAIN_FILE)

    get_filename_component(_toolchain_abs "${CMAKE_TOOLCHAIN_FILE}" ABSOLUTE)
    get_filename_component(_toolchain_leaf "${_toolchain_abs}" NAME)

    # Lower case comparison
    string(TOLOWER "${_toolchain_leaf}" _toolchain_leaf_lower)
    if(_toolchain_leaf_lower STREQUAL "vcpkg.cmake")
        set(USE_VCPKG ON CACHE BOOL "Use vcpkg")
        if(WIN32)
            set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "")
        elseif(LINUX)
            set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "")
        endif()
        if(NOT DEFINED VCPKG_OVERLAY_PORTS OR VCPKG_OVERLAY_PORTS STREQUAL "")
            set(VCPKG_OVERLAY_PORTS "${CMAKE_SOURCE_DIR}/vcpkg-ports" CACHE STRING "Vcpkg overlay ports")
        endif()
    elseif(_toolchain_leaf_lower MATCHES "^conan.*\.cmake$")
        set(USE_CONAN ON CACHE BOOL "Use Conan")
    else()
        message(FATAL_ERROR "CMAKE_TOOLCHAIN_FILE must be either a vcpkg.cmake or a Conan-generated cmake file. Provided: ${CMAKE_TOOLCHAIN_FILE}")
    endif()

    unset(_toolchain_abs)
    unset(_toolchain_leaf)
    unset(_toolchain_leaf_lower)

else()

    set(USE_METAL ON CACHE BOOL "Use Metal")

endif()

set(CONAN_SYSTEM_INCLUDES $<BOOL:${USE_CONAN}>)

### set general cmake settings
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
