cmake_minimum_required(VERSION 3.23)

find_package(Git QUIET)
if(NOT GIT_FOUND)
	message(FATAL_ERROR "Git not found")
else()
	message(STATUS "Git found: ${GIT_EXECUTABLE} version ${GIT_VERSION_STRING}")
	if(GIT_VERSION_STRING VERSION_LESS "2.25.0")
		message(FATAL_ERROR "Git version 2.25.0 or later required")
	endif()
endif()

if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../../.git)
	message(FATAL_ERROR "It seems symbol repository has not been properly cloned or copied (missing .git directory)")
endif ()

project(catapult_server)
option(ENABLE_CODE_COVERAGE "Enable code coverage" OFF)
option(ENABLE_TESTS "Enable tests" ON)

# ccache on windows requires real binary instead of the shims used by scoop to be in the path
option(USE_CCACHE_ON_WINDOWS "Enable use ccache on Windows" OFF)

include(CMakeGlobalSettings.cmake)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

message(STATUS "CATAPULT_VERSION_DESCRIPTION: ${CATAPULT_VERSION_DESCRIPTION}")
message(STATUS "USE_CONAN:                    ${USE_CONAN}")
message(STATUS "CMAKE_CXX_COMPILER_ID:        ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "CMAKE_CXX_FLAGS:              ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_PREFIX_PATH:            ${CMAKE_PREFIX_PATH}")
message(STATUS "CMAKE_MODULE_PATH:            ${CMAKE_MODULE_PATH}")
message(STATUS "CMAKE_BUILD_TYPE:             ${CMAKE_BUILD_TYPE}")

set(CORE_CATAPULT_LIBS catapult.io catapult.ionet catapult.model catapult.thread catapult.utils)

# finds the imported location of TARGET_NAME and assigns it to OUTPUT_LIBRARY_PATH
function(get_library_path OUTPUT_LIBRARY_PATH TARGET_NAME)
	get_property(LIBRARY_PATH TARGET ${TARGET_NAME} PROPERTY IMPORTED_LOCATION_RELEASE)
	if(NOT LIBRARY_PATH)
		get_property(LIBRARY_PATH TARGET ${TARGET_NAME} PROPERTY IMPORTED_LOCATION_RELWITHDEBINFO)
	endif()

	set("${OUTPUT_LIBRARY_PATH}" "${LIBRARY_PATH}" PARENT_SCOPE)
endfunction()

### setup boost
message("--- locating boost dependencies ---")
find_package(Boost 1.83.0 EXACT COMPONENTS ${CATAPULT_BOOST_COMPONENTS} REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIR})

message("boost     ver: ${Boost_VERSION}")
message("boost     inc: ${Boost_INCLUDE_DIR}")
message("boost    libs: ${Boost_LIBRARIES}")

### setup gtest
if(ENABLE_TESTS)
	message("--- locating gtest dependencies ---")

	find_package(GTest 1.16.0 EXACT REQUIRED)
	message("GTest   found: ${GTest_FOUND}")
	message("GTest     inc: ${GTest_INCLUDE_DIRS}")
	message("GTest     ver: ${GTest_VERSION}")

	# GTest_LIBRARIES is when using Conan 2, GTEST_LIBRARIES is set for metal builds
	if (NOT GTest_LIBRARIES AND GTEST_LIBRARIES)
		set(GTest_LIBRARIES ${GTEST_LIBRARIES})
	endif()

	message("GTest    libs: ${GTest_LIBRARIES}")
endif()

### setup openssl
message("--- locating openssl dependencies ---")

find_package(OpenSSL 3.4.1 EXACT REQUIRED)
message("OpenSSL   ver: ${OPENSSL_VERSION}")
message("OpenSSL  root: ${OPENSSL_ROOT_DIR}")
message("OpenSSL   inc: ${OPENSSL_INCLUDE_DIR}")
message("OpenSSL  libs: ${OPENSSL_LIBRARIES}")

# used to add openssl dependencies to a target
function(catapult_add_openssl_dependencies TARGET_NAME)
	include_directories(${OPENSSL_INCLUDE_DIR})
	target_link_libraries(${TARGET_NAME} OpenSSL::SSL)
endfunction()

### setup rocksdb
message("--- locating rocksdb dependencies ---")
find_package(RocksDB 9.8.4 EXACT REQUIRED)

if(WIN32)
	set(RocksDB_LIBRARY RocksDB::rocksdb)
else()
	set(RocksDB_LIBRARY RocksDB::rocksdb-shared)
endif()

message("rocksdb   ver: ${RocksDB_VERSION}")
get_property(RocksDB_INCLUDE_DIRS TARGET ${RocksDB_LIBRARY} PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
message("rocksdb   inc: ${RocksDB_INCLUDE_DIRS}")
get_library_path("RocksDB_LIBRARY_PATH" ${RocksDB_LIBRARY})
message("rocksdb   lib: ${RocksDB_LIBRARY_PATH}")

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

if(USE_CONAN)
	message("--- copy conan deps ---")
	install(DIRECTORY ${CMAKE_BINARY_DIR}/deps/ DESTINATION deps)
endif()
