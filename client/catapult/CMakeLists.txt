cmake_minimum_required(VERSION 3.14)
project(catapult_server)
include(CMakeGlobalSettings.cmake)
option(ENABLE_CODE_COVERAGE "Enable code coverage" OFF)
option(ENABLE_TESTS "Enable tests" ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

message(STATUS "CATAPULT_VERSION_DESCRIPTION: ${CATAPULT_VERSION_DESCRIPTION}")
message(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "BOOST_ROOT: $ENV{BOOST_ROOT}")

set(CORE_CATAPULT_LIBS catapult.io catapult.ionet catapult.model catapult.thread catapult.utils)

### setup boost
find_package(Boost COMPONENTS atomic system date_time regex timer chrono log thread filesystem program_options REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIR})

### setup openssl
message("--- locating openssl dependencies ---")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	# https://gitlab.kitware.com/cmake/cmake/issues/16885
	set(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include)
	list(APPEND OPENSSL_LIBS ${OPENSSL_ROOT_DIR}/lib/libssl.dylib)
	list(APPEND OPENSSL_LIBS ${OPENSSL_ROOT_DIR}/lib/libcrypto.dylib)

	message("OpenSSL root: ${OPENSSL_ROOT_DIR}")
else()
	find_package(OpenSSL REQUIRED)
	list(APPEND OPENSSL_LIBS OpenSSL::SSL)

	message("OpenSSL  ver: ${OPENSSL_VERSION}")
	message("OpenSSL libs: ${OPENSSL_LIBRARIES}")
endif()

message("OpenSSL  inc: ${OPENSSL_INCLUDE_DIR}")
message("OpenSSL libs: ${OPENSSL_LIBS}")

# used to add openssl dependencies to a target
function(catapult_add_openssl_dependencies TARGET_NAME)
	include_directories(${OPENSSL_INCLUDE_DIR})
	target_link_libraries(${TARGET_NAME} ${OPENSSL_LIBS})
endfunction()

### setup rocksdb
message("--- locating rocksdb dependencies ---")
if(WIN32)
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
	if(WIN32)
		target_link_libraries(${TARGET_NAME} RocksDB::rocksdb)
	else()
		include_directories(SYSTEM ${ROCKSDB_INCLUDE_DIR})
		target_link_libraries(${TARGET_NAME} ${ROCKSDB_LIBRARIES})
	endif()
endfunction()

# cmake grouping targets
add_custom_target(extensions)
add_custom_target(mongo)
add_custom_target(plugins)
add_custom_target(server)
add_custom_target(tools)

### add source directories
add_subdirectory(external)

# run catapult tests first
add_subdirectory(src)
catapult_add_tests_subdirectory(tests)

# run other tests second
add_subdirectory(sdk)
add_subdirectory(plugins)
add_subdirectory(extensions)

add_subdirectory(resources)

if(EXISTS "${CMAKE_SOURCE_DIR}/internal/CMakeLists.txt")
	message("--- adding internal directory ---")
	add_subdirectory(internal)
endif()

add_subdirectory(tools)
