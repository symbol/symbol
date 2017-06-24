### enable testing
enable_testing()

### enable ccache if available
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
	set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
	set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

### set general cmake settings
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

### set boost settings
add_definitions(-DBOOST_ALL_DYN_LINK)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

### set compiler settings
if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /WX /EHsc")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MD /Zi")
	set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /DEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd /D_SCL_SECURE_NO_WARNINGS")

	# disable: "marked as __forceinline not inlined"
	add_definitions(-D_WIN32_WINNT=0x0601 /wd4714 /w44287 /w44388)

	# explicitly disable linking against static boost libs
	add_definitions(-DBOOST_ALL_NO_LIB)
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
	# basically equivalent to MSVC W4
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-invalid-offsetof -Werror")

	# add memset_s
	add_definitions(-D_STDC_WANT_LIB_EXT1_=1)
	add_definitions(-D__STDC_WANT_LIB_EXT1__=1)

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
	# basically equivalent to MSVC W4
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
		-Weverything \
		-Werror \
		-Wno-c++98-compat \
		-Wno-c++98-compat-pedantic \
		-Wno-disabled-macro-expansion \
		-Wno-global-constructors \
		-Wno-invalid-offsetof \
		-Wno-newline-eof \
		-Wno-padded \
		-Wno-switch-enum \
		-Wno-weak-vtables")

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
endif()

# set runpath for built binaries on linux
if(("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU") OR ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" AND "${CMAKE_SYSTEM_NAME}" MATCHES "Linux"))
	file(MAKE_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/boost")
	set(CMAKE_SKIP_BUILD_RPATH FALSE)

	# $origin - to load plugins when running the server
	# $origin/boost - same, use our boost libs
	set(CMAKE_INSTALL_RPATH "$ORIGIN:$ORIGIN/boost${CMAKE_INSTALL_RPATH}")
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

	# use rpath for executables (executable rpath will be used for loading indirect libs, this is needed because boost libs
	#   do not set runpath)
	# use newer runpath for shared libs
	set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--enable-new-dtags")
endif()

### find and set mongo libraries
function(catapult_add_mongo_dependencies TARGET_NAME)
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

	message("mongo c    lib: ${MONGOC_LIB}")
	message(" bson c    lib: ${BSONC_LIB}")
	message("mongo cxx dirs: ${MONGOCXX_LIB} ${LIBMONGOCXX_INCLUDE_DIRS} ${LIBMONGOCXX_LIBRARY_DIRS} ${LIBMONGOCXX_LIBRARIES}")
	message(" bson cxx dirs: ${BSONCXX_LIB} ${LIBBSONCXX_INCLUDE_DIRS} ${LIBBSONCXX_LIBRARY_DIRS} ${LIBBSONCXX_LIBRARIES}")

	add_definitions(-DBSONCXX_STATIC -DMONGOCXX_STATIC)
	include_directories(SYSTEM ${LIBMONGOCXX_INCLUDE_DIRS} ${LIBBSONCXX_INCLUDE_DIRS})
	target_link_libraries(${TARGET_NAME} ${MONGOCXX_LIB} ${BSONCXX_LIB} ${MONGOC_LIB} ${BSONC_LIB} ${MONGO_ADDITIONAL_LIBS})
endfunction(catapult_add_mongo_dependencies)

### define helper functions

# used to define a catapult target (library, executable) and automatically enables PCH for clang
function(catapult_target TARGET_NAME)
	set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 14)

	# indicate boost as a dependency
	target_link_libraries(${TARGET_NAME} ${Boost_LIBRARIES})

	# copy boost shared libraries
	foreach(BOOST_COMPONENT ATOMIC SYSTEM DATE_TIME REGEX TIMER CHRONO LOG THREAD FILESYSTEM PROGRAM_OPTIONS)
		if(MSVC)
			# copy into ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(Configuration)
			string(REPLACE ".lib" ".dll" BOOSTDLLNAME ${Boost_${BOOST_COMPONENT}_LIBRARY_RELEASE})
			add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_if_different
				"${BOOSTDLLNAME}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(Configuration)")
		elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
			# copy into ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/boost
			set(BOOSTDLLNAME ${Boost_${BOOST_COMPONENT}_LIBRARY_RELEASE})
			set(BOOSTVERSION "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")
			get_filename_component(BOOSTFILENAME ${BOOSTDLLNAME} NAME)
			add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_if_different
				"${BOOSTDLLNAME}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/boost")
		endif()
	endforeach()

	# put both plugins and plugins tests in same 'folder'
	if(TARGET_NAME MATCHES "\.plugins")
		set_property(TARGET ${TARGET_NAME} PROPERTY FOLDER "plugins")
	endif()

	if(TARGET_NAME MATCHES "\.tools")
		set_property(TARGET ${TARGET_NAME} PROPERTY FOLDER "tools")
	endif()
endfunction(catapult_target)

function(find_all_target_files TARGET_NAME)
	file(GLOB ${TARGET_NAME}_INCLUDE_SRC "*.h")
	file(GLOB ${TARGET_NAME}_SRC "*.cpp")

	set(CURRENT_FILES ${${TARGET_NAME}_INCLUDE_SRC} ${${TARGET_NAME}_SRC})
	SOURCE_GROUP("src" FILES ${CURRENT_FILES})
	set(TARGET_FILES ${CURRENT_FILES})

	# add any (optional) subdirectories
	foreach(arg ${ARGN})
		set(SUBDIR ${arg})
		message("+ processing subdirectory '${arg}'")
		file(GLOB ${TARGET_NAME}_${SUBDIR}_INCLUDE_SRC "${SUBDIR}/*.h")
		file(GLOB ${TARGET_NAME}_${SUBDIR}_SRC "${SUBDIR}/*.cpp")

		set(CURRENT_FILES ${${TARGET_NAME}_${SUBDIR}_INCLUDE_SRC} ${${TARGET_NAME}_${SUBDIR}_SRC})
		SOURCE_GROUP("${SUBDIR}" FILES ${CURRENT_FILES})
		set(TARGET_FILES ${TARGET_FILES} ${CURRENT_FILES})
	endforeach()

	set(${TARGET_NAME}_FILES ${TARGET_FILES} PARENT_SCOPE)
endfunction(find_all_target_files)

# used to define a catapult library, creating an appropriate source group and adding a library
function(catapult_library TARGET_NAME)
	message("processing lib '${TARGET_NAME}'")
	find_all_target_files(${TARGET_NAME} ${ARGN})

	if (ENABLE_STRESS)
		if (NOT STRESS_COUNT)
			set(STRESS_COUNT 100)
		endif()
		MESSAGE(STATUS "Enabling stress test for ${TARGET_NAME} (${STRESS_COUNT})")
		add_definitions(-DSTRESS=${STRESS_COUNT})
	endif()

	add_library(${TARGET_NAME} ${${TARGET_NAME}_FILES})
endfunction()

# combines catapult_library and catapult_target
function(catapult_library_target TARGET_NAME)
	catapult_library(${TARGET_NAME} ${ARGN})
	set_property(TARGET ${TARGET_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
	catapult_target(${TARGET_NAME})
endfunction()

# used to define a catapult shared library, creating an appropriate source group and adding a library
function(catapult_shared_library TARGET_NAME)
	message("processing shared lib '${TARGET_NAME}'")
	find_all_target_files(${TARGET_NAME} ${ARGN})

	add_definitions(-DDLL_EXPORTS)

	add_library(${TARGET_NAME} SHARED ${${TARGET_NAME}_FILES})
endfunction()

# combines catapult_shared_library and catapult_target
function(catapult_shared_library_target TARGET_NAME)
	catapult_shared_library(${TARGET_NAME} ${ARGN})
	catapult_target(${TARGET_NAME})
endfunction()

# used to define a catapult executable, creating an appropriate source group and adding an executable
function(catapult_executable TARGET_NAME)
	message("processing exe '${TARGET_NAME}'")
	find_all_target_files(${TARGET_NAME} ${ARGN})

	add_executable(${TARGET_NAME} ${${TARGET_NAME}_FILES})

	if(WIN32 AND MINGW)
		target_link_libraries(${TARGET_NAME} wsock32 ws2_32)
	endif()
endfunction()

function(catapult_header_only_target TARGET_NAME)
	if(MSVC)
		message("processing hdr '${TARGET_NAME}'")
		find_all_target_files(${TARGET_NAME} ${ARGN})

		# unfortunately add_library INTERFACE doesn't seem to work
		# http://stackoverflow.com/questions/5957134/how-to-setup-cmake-to-generate-header-only-projects
		add_library(${TARGET_NAME} STATIC ${${TARGET_NAME}_FILES})
		set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)
	endif()
endfunction()

# used to define a catapult test executable
function(catapult_test_executable TARGET_NAME)
	catapult_executable(${TARGET_NAME} ${ARGN})
	add_test(NAME ${TARGET_NAME} WORKING_DIRECTORY ${CMAKE_BINARY_DIR} COMMAND ${TARGET_NAME})

	target_link_libraries(${TARGET_NAME} ${GTEST_LIBRARIES})

	if (ENABLE_CODE_COVERAGE)
		MESSAGE(STATUS "Enabling code coverage for ${TARGET_NAME}")
		set_target_properties(${TARGET_NAME} PROPERTIES COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
		target_link_libraries(${TARGET_NAME} gcov)
	endif()
	if (ENABLE_STRESS)
		if (NOT STRESS_COUNT)
			set(STRESS_COUNT 100)
		endif()
		MESSAGE(STATUS "Enabling stress test for ${TARGET_NAME} (${STRESS_COUNT})")
		add_definitions(-DSTRESS=${STRESS_COUNT})
	endif()
endfunction()

# used to define a catapult test executable for a catapult library by combining catapult_test_executable and
# catapult_target and adding some library dependencies
function(catapult_test_executable_target TARGET_NAME TEST_DEPENDENCY_NAME)
	catapult_test_executable(${TARGET_NAME} ${ARGN})

	# test libraries are in the form test.xyz, so add xyz as a dependency (the library under test)
	string(FIND ${TARGET_NAME} "." TEST_END_INDEX)
	MATH(EXPR TEST_END_INDEX "${TEST_END_INDEX}+1")
	string(SUBSTRING ${TARGET_NAME} ${TEST_END_INDEX} -1 LIBRARY_UNDER_TEST)

	target_link_libraries(${TARGET_NAME} tests.catapult.test.${TEST_DEPENDENCY_NAME} ${LIBRARY_UNDER_TEST})
	catapult_target(${TARGET_NAME})
endfunction()

# used to define a catapult test executable for a header only catapult library by combining catapult_test_executable and
# catapult_target and adding some library dependencies
function(catapult_test_executable_target_header_only TARGET_NAME TEST_DEPENDENCY_NAME)
	catapult_test_executable(${TARGET_NAME} ${ARGN})

	target_link_libraries(${TARGET_NAME} tests.catapult.test.${TEST_DEPENDENCY_NAME})
	catapult_target(${TARGET_NAME})
endfunction()
