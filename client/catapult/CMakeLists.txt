cmake_minimum_required(VERSION 3.25)

find_package(Git 2.25.0 REQUIRED)

if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../../.git)
	message(FATAL_ERROR "It seems symbol repository has not been properly cloned or copied (missing .git directory)")
endif ()

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
	message(FATAL_ERROR "In-source builds are not allowed.\nPlease create a separate build directory and run CMake from there.")
endif()

include(cmake/cxx17.cmake)													# C++17 is required for the project, so set it up globally
include(cmake/toolchain.cmake)												# setup toolchain and related options (e.g. USE_CONAN, USE_VCPKG)
project(catapult_server LANGUAGES C CXX)									# setup project
set_property(GLOBAL PROPERTY USE_FOLDERS ON)								# use FOLDER target property
get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

# setup configurable options
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_CCACHE_ON_WINDOWS "Enable ccache on Windows (requires ccache executable in PATH)" OFF [[CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows"]] OFF) # Only supported on Windows host
option(ENABLE_CODE_COVERAGE "Enable code coverage" OFF)
option(ENABLE_TESTS "Enable tests" ON)
option(ENABLE_CATAPULT_DIAGNOSTICS "Enable custom diagnostics" OFF)
option(ENABLE_FUZZ_BUILD "Enable fuzz testing build" OFF)
option(ENABLE_HARDENING "Enable compiler hardening flags" OFF)
option(ENABLE_CODE_COVERAGE "Enable code coverage" OFF)

if(ENABLE_FUZZ_BUILD)
	set(USE_SANITIZER "undefined" CACHE STRING "Sanitizer to use (address, undefined, etc.)")
else()
    set(USE_SANITIZER "" CACHE STRING "Sanitizer to use (address, undefined, etc.)")
endif()

include(cmake/packages.cmake)                                               # setup dependencies and related helper functions
include(cmake/functions.cmake)                                              # include custom CMake functions/helpers
include(cmake/settings_all.cmake)                                           # setup cross platform/compiler build settings (e.g. compiler flags, sanitizers, hardening, etc.)

# include platform/compiler specific settings
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    if(MSVC OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"))
    include(cmake/settings_win_msvc.cmake)
    else()
        message(FATAL_ERROR "Unsupported compiler on Windows")
    endif()
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        include(cmake/settings_nix_gnu.cmake)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        include(cmake/settings_nix_clang.cmake)
    else()
        message(FATAL_ERROR "Unsupported compiler on Unix")
    endif()
else()
    message(FATAL_ERROR "Unsupported host system: ${CMAKE_HOST_SYSTEM_NAME}")
endif()

include(CMakeGlobalSettings.cmake)


# used to add openssl dependencies to a target
function(catapult_add_openssl_dependencies TARGET_NAME)
    target_include_directories(${TARGET_NAME} PRIVATE ${OPENSSL_INCLUDE_DIR})
	target_link_libraries(${TARGET_NAME} OpenSSL::SSL)
endfunction()

# used to add rocksdb dependencies to a target
function(catapult_add_rocksdb_dependencies TARGET_NAME)
	target_link_libraries(${TARGET_NAME} ${RocksDB_LIBRARY})
endfunction()

# cmake grouping targets
add_custom_target(extensions)
add_custom_target(mongo)
add_custom_target(plugins)
add_custom_target(server)
add_custom_target(tools)

# add source directories
subdirectories(ADD BASE . DIRS external src tests sdk plugins extensions resources internal tools)

if(USE_CONAN)
	install(DIRECTORY ${CMAKE_BINARY_DIR}/deps/ DESTINATION deps)
endif()

# Print summary of configuration

if(IS_MULTI_CONFIG)
    set(BUILD_TYPE_DISPLAY "Multi [${CMAKE_CONFIGURATION_TYPES}]")
else()
    set(BUILD_TYPE_DISPLAY "${CMAKE_BUILD_TYPE}")
endif()

message(STATUS 
        "\n============================================================================\n"
        "   Summary :\n"
        "   --------------\n"
        "   Project        ${CMAKE_PROJECT_NAME}\n"
        "   Source         ${PROJECT_SOURCE_DIR}\n"
        "   Cmake          ${CMAKE_VERSION}\n"
        "   Git            ${GIT_EXECUTABLE} (${GIT_VERSION_STRING})\n"
        "   Toolchain      ${CMAKE_TOOLCHAIN_FILE}\n"
        "   Host           ${CMAKE_HOST_SYSTEM_NAME} ${CMAKE_HOST_SYSTEM_PROCESSOR}\n"
        "   Compiler       ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}\n"
        "   Config Type    ${BUILD_TYPE_DISPLAY}\n"
        "============================================================================\n"
        "   Options :\n"
        "   --------------\n"
        "   Win Ccache     ${ENABLE_CCACHE_ON_WINDOWS}\n"
        "   Coverage       ${ENABLE_CODE_COVERAGE}\n"
        "   Tests          ${ENABLE_TESTS}\n"
        "   Diagnostics    ${ENABLE_CATAPULT_DIAGNOSTICS}\n"
        "   Fuzz Build     ${ENABLE_FUZZ_BUILD}\n"
        "   Hardening      ${ENABLE_HARDENING}\n"
        "   Sanitizer      ${USE_SANITIZER}\n"
        "============================================================================\n"
        "   Dependencies:\n"
        "   --------------\n"
        "   Boost          ${Boost_VERSION}\n"
        "   OpenSSL        ${OPENSSL_VERSION}\n"
        "   RocksDB        ${RocksDB_VERSION}\n"
        "   GTest          ${GTest_VERSION}\n"
        "   Benchmark      ${benchmark_VERSION}\n"
        "   MongoDB        ${mongocxx_VERSION}\n"
        "   MongoC         ${mongoc_VERSION}\n"
        "   cppzmq         ${cppzmq_VERSION}\n"
        "   zeromq         ${ZeroMQ_VERSION}\n"
        "============================================================================"
)
