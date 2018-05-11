cmake_minimum_required(VERSION 3.2)

message("--- locating mongo dependencies ---")
find_package(LIBMONGOCXX REQUIRED)
find_package(LIBBSONCXX REQUIRED)

if(MSVC)
	find_library(MONGOCXX_LIB ${LIBMONGOCXX_LIBRARIES} ${LIBMONGOCXX_LIBRARY_DIRS})
	find_library(BSONCXX_LIB ${LIBBSONCXX_LIBRARIES} ${LIBBSONCXX_LIBRARY_DIRS})

	find_library(MONGOC_LIB mongoc-1.0 ${LIBMONGOCXX_LIBRARY_DIRS})
	find_library(BSONC_LIB bson-1.0 ${LIBMONGOCXX_LIBRARY_DIRS})

	set(MONGO_ADDITIONAL_LIBS bcrypt crypt32 secur32 ws2_32)
else()
	find_library(MONGOCXX_LIB ${LIBMONGOCXX_LIBRARIES} ${LIBMONGOCXX_LIBRARY_DIRS})
	find_library(BSONCXX_LIB ${LIBBSONCXX_LIBRARIES} ${LIBBSONCXX_LIBRARY_DIRS})

	find_library(MONGOC_LIB mongoc-1.0 ${LIBMONGOCXX_LIBRARY_DIRS})
	find_library(BSONC_LIB bson-1.0 ${LIBMONGOCXX_LIBRARY_DIRS})
	set(MONGO_ADDITIONAL_LIBS)
endif()

message("  mongo c    lib: ${MONGOC_LIB}")
message("   bson c    lib: ${BSONC_LIB}")
message("  mongo cxx dirs: ${MONGOCXX_LIB} ${LIBMONGOCXX_INCLUDE_DIRS} ${LIBMONGOCXX_LIBRARY_DIRS} ${LIBMONGOCXX_LIBRARIES}")
message("   bson cxx dirs: ${BSONCXX_LIB} ${LIBBSONCXX_INCLUDE_DIRS} ${LIBBSONCXX_LIBRARY_DIRS} ${LIBBSONCXX_LIBRARIES}")

### find and set mongo includes and libraries
function(catapult_add_mongo_dependencies TARGET_NAME)
	add_definitions(-DBSONCXX_STATIC -DMONGOCXX_STATIC)
	include_directories(SYSTEM ${LIBMONGOCXX_INCLUDE_DIRS} ${LIBBSONCXX_INCLUDE_DIRS})
	target_link_libraries(${TARGET_NAME} ${MONGOCXX_LIB} ${BSONCXX_LIB} ${MONGOC_LIB} ${BSONC_LIB} ${MONGO_ADDITIONAL_LIBS})
endfunction()

# used to define a catapult mongo int test executable by combining catapult_test_executable and catapult_target
function(catapult_int_test_executable_target TARGET_NAME)
	catapult_test_executable(${TARGET_NAME} ${ARGN})
	catapult_target(${TARGET_NAME})

	set_property(TEST ${TARGET_NAME} PROPERTY LABELS Database)

	# customize and export compiler options for gtest
	catapult_set_test_compiler_options()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" PARENT_SCOPE)
endfunction()

add_subdirectory(plugins)

catapult_define_extension(mongo)
catapult_add_mongo_dependencies(extension.mongo)
