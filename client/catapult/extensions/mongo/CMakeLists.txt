cmake_minimum_required(VERSION 3.2)

message("--- locating mongo dependencies ---")
find_package(LIBMONGOCXX REQUIRED)
find_package(LIBBSONCXX REQUIRED)

message("mongocxx  ver: ${LIBMONGOCXX_PACKAGE_VERSION}")

if(LIBMONGOCXX_PACKAGE_VERSION LESS 3.3)
	find_library(MONGOCXX_LIB ${LIBMONGOCXX_LIBRARIES} ${LIBMONGOCXX_LIBRARY_DIRS})
	find_library(BSONCXX_LIB ${LIBBSONCXX_LIBRARIES} ${LIBBSONCXX_LIBRARY_DIRS})
	set(LIBMONGOCXX_LIBRARIES ${MONGOCXX_LIB} ${BSONCXX_LIB})
endif()

find_library(MONGOC_LIB mongoc-1.0 ${LIBMONGOCXX_LIBRARY_DIRS})
find_library(BSONC_LIB bson-1.0 ${LIBMONGOCXX_LIBRARY_DIRS})

if(MSVC)
	set(MONGO_ADDITIONAL_LIBS bcrypt crypt32 secur32 ws2_32)
else()
	set(MONGO_ADDITIONAL_LIBS)
endif()

message("mongocxx libs: ${LIBMONGOCXX_LIBRARIES}")
message("mongocxx  inc: ${LIBMONGOCXX_INCLUDE_DIRS}")
message("mongoc    lib: ${MONGOC_LIB}")
message("bsonc     lib: ${BSONC_LIB}")

### find and set mongo includes and libraries
function(catapult_add_mongo_dependencies TARGET_NAME)
	include_directories(SYSTEM ${LIBMONGOCXX_INCLUDE_DIRS} ${LIBBSONCXX_INCLUDE_DIRS})
	target_link_libraries(${TARGET_NAME} ${LIBMONGOCXX_LIBRARIES} ${MONGOC_LIB} ${BSONC_LIB} ${MONGO_ADDITIONAL_LIBS})
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

catapult_define_extension_no_group(mongo)
catapult_add_mongo_dependencies(extension.mongo)

# add extension to mongo group
add_dependencies(mongo extension.mongo)