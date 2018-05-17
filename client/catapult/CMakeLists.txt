cmake_minimum_required(VERSION 3.2)
project(catapult_server)
include(CMakeGlobalSettings.cmake)
option(ENABLE_CODE_COVERAGE "Enable code coverage" OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

message(STATUS "CATAPULT_VERSION_DESCRIPTION: ${CATAPULT_VERSION_DESCRIPTION}")
MESSAGE(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
MESSAGE(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
MESSAGE(STATUS "BOOST_ROOT: $ENV{BOOST_ROOT}")

set(CORE_CATAPULT_LIBS catapult.io catapult.ionet catapult.model catapult.thread catapult.utils)

### setup boost
find_package(Boost COMPONENTS atomic system date_time regex timer chrono log thread filesystem program_options REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIR})

### setup rocksdb
message("--- locating rocksdb dependencies ---")
if (WIN32)
	find_package(RocksDB REQUIRED)
else()
	find_path(ROCKSDB_ROOT_DIR NAMES include/rocksdb/db.h)
	find_library(ROCKSDB_LIBRARIES NAMES rocksdb HINTS ${ROCKSDB_ROOT_DIR})
	find_path(ROCKSDB_INCLUDE_DIR NAMES rocksdb/db.h HINTS ${ROCKSDB_ROOT_DIR}/include)

	message("rocksdb   lib: ${ROCKSDB_LIBRARIES}")
	message("rocksdb   inc: ${ROCKSDB_INCLUDE_DIR}")
endif()

# used to add rocksdb dependencies to a target
function(catapult_add_rocksdb_dependencies TARGET_NAME)
	if (WIN32)
		target_link_libraries(${TARGET_NAME} RocksDB::rocksdb)
	else()
		include_directories(SYSTEM ${ROCKSDB_INCLUDE_DIR})
		target_link_libraries(${TARGET_NAME} ${ROCKSDB_LIBRARIES})
	endif()
endfunction()

### add source directories
add_subdirectory(external)

# run catapult tests first
add_subdirectory(src)
add_subdirectory(tests)

# run other tests second
add_subdirectory(sdk)
add_subdirectory(plugins)
add_subdirectory(extensions)

add_subdirectory(resources)

if (EXISTS "${CMAKE_SOURCE_DIR}/internal/CMakeLists.txt")
	message("--- adding internal directory ---")
	add_subdirectory(internal)
endif()

add_subdirectory(tools)
