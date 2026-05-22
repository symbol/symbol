
if(MSVC)
	function(set_win_version_definitions WIN_TARGET_NAME WIN_FILETYPE)
		add_definitions(-DCATAPULT_VERSION_DESCRIPTION="${CATAPULT_VERSION_DESCRIPTION}")
		add_definitions(-DWIN_FILETYPE=${WIN_FILETYPE})

		if(CATAPULT_BUILD_RELEASE)
			add_definitions(-DCATAPULT_BUILD_RELEASE=1)
		endif()
	endfunction()

	# embed the version rc file
	set(VERSION_RESOURCES ${CMAKE_SOURCE_DIR}/src/catapult/version/win/win_version.rc)
else()
	# since strings are not referenced, in order for the linker to include them, they must be forcibly linked via an object file
	set(VERSION_RESOURCES $<TARGET_OBJECTS:catapult.version.nix>)
endif()

### define target helper functions

# used to define a catapult target (library, executable) and automatically enables PCH for clang
function(catapult_target TARGET_NAME)

	# indicate boost as a dependency
	target_link_libraries(${TARGET_NAME} build.defaults ${Boost_LIBRARIES})

	# put both plugins and plugins tests in same 'folder'
	if(TARGET_NAME MATCHES "\.plugins")
		set_property(TARGET ${TARGET_NAME} PROPERTY FOLDER "plugins")
	endif()

	if(TARGET_NAME MATCHES "\.tools")
		set_property(TARGET ${TARGET_NAME} PROPERTY FOLDER "tools")
	endif()
endfunction()

# finds all files comprising a target
function(catapult_find_all_target_files TARGET_TYPE TARGET_NAME)
	if(CMAKE_VERBOSE_MAKEFILE)
		message(TRACE "processing ${TARGET_TYPE} '${TARGET_NAME}'")
	endif()

	file(GLOB ${TARGET_NAME}_INCLUDE_SRC "*.h")
	file(GLOB ${TARGET_NAME}_SRC "*.cpp")

	set(CURRENT_FILES ${${TARGET_NAME}_INCLUDE_SRC} ${${TARGET_NAME}_SRC})
	SOURCE_GROUP("src" FILES ${CURRENT_FILES})
	set(TARGET_FILES ${CURRENT_FILES})

	# add any (optional) subdirectories
	foreach(arg ${ARGN})
		set(SUBDIR ${arg})
		if(CMAKE_VERBOSE_MAKEFILE)
			message(TRACE "+ processing subdirectory '${arg}'")
		endif()

		file(GLOB ${TARGET_NAME}_${SUBDIR}_INCLUDE_SRC "${SUBDIR}/*.h")
		file(GLOB ${TARGET_NAME}_${SUBDIR}_SRC "${SUBDIR}/*.cpp")

		set(CURRENT_FILES ${${TARGET_NAME}_${SUBDIR}_INCLUDE_SRC} ${${TARGET_NAME}_${SUBDIR}_SRC})
		SOURCE_GROUP("${SUBDIR}" FILES ${CURRENT_FILES})
		set(TARGET_FILES ${TARGET_FILES} ${CURRENT_FILES})
	endforeach()

	set(${TARGET_NAME}_FILES ${TARGET_FILES} PARENT_SCOPE)
endfunction()

# used to define a catapult object library
function(catapult_object_library TARGET_NAME)
	add_library(${TARGET_NAME} OBJECT ${ARGN})
	set_property(TARGET ${TARGET_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
endfunction()

# used to define a catapult library, creating an appropriate source group and adding a library
function(catapult_library TARGET_NAME)
	catapult_find_all_target_files("lib" ${TARGET_NAME} ${ARGN})
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
	catapult_find_all_target_files("shared lib" ${TARGET_NAME} ${ARGN})

	add_definitions(-DDLL_EXPORTS)

	if(MSVC)
		set_win_version_definitions(${TARGET_NAME} VFT_DLL)
	endif()

	add_library(${TARGET_NAME} SHARED ${${TARGET_NAME}_FILES} ${VERSION_RESOURCES})
endfunction()

# combines catapult_shared_library and catapult_target
function(catapult_shared_library_target TARGET_NAME)
	catapult_shared_library(${TARGET_NAME} ${ARGN})
	catapult_target(${TARGET_NAME})

	install(TARGETS ${TARGET_NAME})
endfunction()

# used to define a catapult executable, creating an appropriate source group and adding an executable
function(catapult_executable TARGET_NAME)
	catapult_find_all_target_files("exe" ${TARGET_NAME} ${ARGN})

	if(MSVC)
		set_win_version_definitions(${TARGET_NAME} VFT_APP)
	endif()

	add_executable(${TARGET_NAME} ${${TARGET_NAME}_FILES} ${VERSION_RESOURCES})

	if(WIN32 AND MINGW)
		target_link_libraries(${TARGET_NAME} wsock32 ws2_32)
	endif()
endfunction()

# used to define a catapult test executable
function(catapult_test_executable TARGET_NAME)
	catapult_executable(${TARGET_NAME} ${ARGN})
	target_link_libraries(${TARGET_NAME} build.defaults build.tests)
	add_test(NAME ${TARGET_NAME} WORKING_DIRECTORY ${CMAKE_BINARY_DIR} COMMAND ${TARGET_NAME})
endfunction()

# used to define a catapult test executable for a catapult library by combining catapult_test_executable and
# catapult_target and adding some library dependencies
function(catapult_test_executable_target TARGET_NAME TEST_DEPENDENCY_NAME)
	catapult_test_executable(${TARGET_NAME} ${ARGN})

	# customize and export compiler options for gtest
	target_link_libraries (${TARGET_NAME} build.tests)

	# test libraries are in the form test.xyz, so add xyz as a dependency (the library under test)
	string(FIND ${TARGET_NAME} "." TEST_END_INDEX)
	MATH(EXPR TEST_END_INDEX "${TEST_END_INDEX}+1")
	string(SUBSTRING ${TARGET_NAME} ${TEST_END_INDEX} -1 LIBRARY_UNDER_TEST)

	target_link_libraries(${TARGET_NAME} tests.catapult.test.${TEST_DEPENDENCY_NAME} ${LIBRARY_UNDER_TEST})
	catapult_target(${TARGET_NAME})
endfunction()

# used to define a catapult test executable for a header only catapult library by combining catapult_test_executable and
# catapult_target and adding some library dependencies
# also used when the library under test should not be automatically added because it's included by the test dependency library
function(catapult_test_executable_target_no_lib TARGET_NAME TEST_DEPENDENCY_NAME)
	catapult_test_executable(${TARGET_NAME} ${ARGN})

	# customize and export compiler options for gtest
	target_link_libraries(${TARGET_NAME} build.tests)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" PARENT_SCOPE)

	target_link_libraries(${TARGET_NAME} tests.catapult.test.${TEST_DEPENDENCY_NAME})
	catapult_target(${TARGET_NAME})
endfunction()

