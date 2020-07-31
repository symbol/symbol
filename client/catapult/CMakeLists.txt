cmake_minimum_required(VERSION 3.14)
project(catapult_server)
option(ENABLE_CODE_COVERAGE "Enable code coverage" OFF)
option(ENABLE_TESTS "Enable tests" ON)

include(CMakeGlobalSettings.cmake)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

message(STATUS "CATAPULT_VERSION_DESCRIPTION: ${CATAPULT_VERSION_DESCRIPTION}")
message(STATUS "USE_CONAN:                    ${USE_CONAN}")

message(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "CMAKE_CXX_FLAGS:       ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_PREFIX_PATH:     ${CMAKE_PREFIX_PATH}")
message(STATUS "CMAKE_MODULE_PATH:     ${CMAKE_MODULE_PATH}")
message(STATUS "CMAKE_BUILD_TYPE:      ${CMAKE_BUILD_TYPE}")

message("--- locating boost dependencies ---")
message("boost    root: ${BOOST_ROOT}")
message("boost     inc: ${Boost_INCLUDE_DIR}")
message("boost    libs: ${Boost_LIBRARIES}")

set(CORE_CATAPULT_LIBS catapult.io catapult.ionet catapult.model catapult.thread catapult.utils)

### setup boost
find_package(Boost COMPONENTS ${CATAPULT_BOOST_COMPONENTS} REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIR})

### setup openssl
message("--- locating openssl dependencies ---")

find_package(OpenSSL REQUIRED)
message("OpenSSL  root: ${OPENSSL_ROOT_DIR}")
message("OpenSSL   ver: ${OPENSSL_VERSION}")
message("OpenSSL   inc: ${OPENSSL_INCLUDE_DIR}")
message("OpenSSL  libs: ${OpenSSL_LIBRARIES}")

# used to add openssl dependencies to a target
function(catapult_add_openssl_dependencies TARGET_NAME)
	include_directories(${OPENSSL_INCLUDE_DIR})
	target_link_libraries(${TARGET_NAME} OpenSSL::SSL)
endfunction()

### setup rocksdb
message("--- locating rocksdb dependencies ---")
if(USE_CONAN)
	find_package(RocksDB REQUIRED)

	if(WIN32)
		set(ROCKSDB_LIBRARY RocksDB::rocksdb)
	else()
		set(ROCKSDB_LIBRARY RocksDB::rocksdb-shared)
	endif()

	get_property(propval TARGET ${ROCKSDB_LIBRARY} PROPERTY IMPORTED_LOCATION_RELEASE)
	message("rocksdb   lib: ${propval}")
	get_property(propval TARGET ${ROCKSDB_LIBRARY} PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
	message("rocksdb   inc: ${propval}")
else()
	if(WIN32)
		find_package(RocksDB REQUIRED)
	else()
		find_path(ROCKSDB_ROOT_DIR NAMES include/rocksdb/db.h)
		find_library(ROCKSDB_LIBRARIES NAMES rocksdb HINTS ${ROCKSDB_ROOT_DIR})
		find_path(ROCKSDB_INCLUDE_DIRS NAMES rocksdb/db.h HINTS ${ROCKSDB_ROOT_DIR}/include)
	endif()

	message("rocksdb   lib: ${ROCKSDB_LIBRARIES}")
	message("rocksdb   inc: ${ROCKSDB_INCLUDE_DIRS}")
endif()

# used to add rocksdb dependencies to a target
function(catapult_add_rocksdb_dependencies TARGET_NAME)
	if(USE_CONAN)
		if(WIN32)
			target_link_libraries(${TARGET_NAME} RocksDB::rocksdb)
		else()
			target_link_libraries(${TARGET_NAME} RocksDB::rocksdb-shared)
		endif()
	elseif(WIN32)
		target_link_libraries(${TARGET_NAME} RocksDB::rocksdb)
	else()
		include_directories(SYSTEM ${ROCKSDB_INCLUDE_DIRS})
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
