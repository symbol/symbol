cmake_minimum_required(VERSION 3.2)
project(catapult_server)
include(CMakeGlobalSettings.cmake)
option(ENABLE_CODE_COVERAGE "Enable code coverage" OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

MESSAGE(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
MESSAGE(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
MESSAGE(STATUS "BOOST_ROOT: $ENV{BOOST_ROOT}")

find_package(Boost COMPONENTS atomic system date_time regex timer chrono log thread filesystem program_options REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIR})

set(CORE_CATAPULT_LIBS catapult.io catapult.ionet catapult.model catapult.thread catapult.utils)

add_subdirectory(external)

# run catapult tests first
add_subdirectory(src)
add_subdirectory(tests)

# run other tests second
add_subdirectory(sdk)
add_subdirectory(plugins)
add_subdirectory(extensions)

add_subdirectory(resources)

add_subdirectory(tools)
