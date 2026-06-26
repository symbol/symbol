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
option(W-NO-CCACHE "Disable ccache even if available" OFF)
option(W-CODE-COVERAGE "Enable code coverage" OFF)
option(W-TESTS "Enable tests" ON)
option(W-CATAPULT-DIAGNOSTICS "Enable custom diagnostics" OFF)
option(W-FUZZ-BUILD "Enable fuzz testing build" OFF)
option(W-HARDENING "Enable compiler hardening flags" OFF)

if(W-FUZZ-BUILD)
	set(W-SANITIZER "undefined" CACHE STRING "Sanitizer to use (address, undefined, etc.)")
else()
	set(W-SANITIZER "" CACHE STRING "Sanitizer to use (address, undefined, etc.)")
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

# cmake grouping targets
add_custom_target(extensions)
add_custom_target(mongo)
add_custom_target(plugins)
add_custom_target(server)
add_custom_target(tools)

# add source directories
# order matters here ?
add_subdirs(
	external
	src
	tests
	sdk
	plugins
	extensions
	resources
	internal
	tools
)

if(USE_CONAN)
	install(DIRECTORY ${CMAKE_BINARY_DIR}/deps/ DESTINATION deps)
endif()

# Print summary of configuration

if(IS_MULTI_CONFIG)
	set(BUILD_TYPE_DISPLAY "Multi [${CMAKE_CONFIGURATION_TYPES}]")
else()
	set(BUILD_TYPE_DISPLAY "${CMAKE_BUILD_TYPE}")
endif()

if(W-NO-CCACHE)
	set(_ccache_display "disabled")
elseif(CCACHE_BIN)
	set(_ccache_display "${CCACHE_BIN}")
else()
	set(_ccache_display "not found")
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
		"   Ccache         ${_ccache_display}\n"
		"   Coverage       ${W-CODE-COVERAGE}\n"
		"   Tests          ${W-TESTS}\n"
		"   Diagnostics    ${W-CATAPULT-DIAGNOSTICS}\n"
		"   Fuzz Build     ${W-FUZZ-BUILD}\n"
		"   Hardening      ${W-HARDENING}\n"
		"   Sanitizer      ${W-SANITIZER}\n"
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
