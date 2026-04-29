
if(USE_CONAN)
	# only set rpath when running conan, which copies dependencies to `@executable_path/../deps`
	# when not using conan, rpath is set to link paths by default
	if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		set(ENABLE_RPATHS ON)
		set(USE_EXPLICIT_RPATHS ON)
	endif()
endif()

if(ENABLE_RPATHS)
	if(USE_EXPLICIT_RPATHS)
		if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
			# $origin - to load plugins when running the server
			set(CMAKE_INSTALL_RPATH "$ORIGIN/../deps:$ORIGIN/../lib")
			set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
			set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

			# use rpath for executables
			# (executable rpath will be used for loading indirect libs, this is needed because boost libs do not set runpath)
			# use newer runpath for shared libs
			set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--enable-new-dtags")
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--disable-new-dtags")
		endif()
		if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
			set(CMAKE_INSTALL_RPATH "@executable_path/../deps;@executable_path/../lib")
			set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
			set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
		endif()
	endif()
else()
	set(CMAKE_SKIP_BUILD_RPATH TRUE)
endif()

### define gtest helper functions

# sets additional compiler options for test projects in order to quiet GTEST warnings while allowing source warning checks to be stricter
function(catapult_set_test_compiler_options)
	# some gtest workarounds for gcc + clang
	if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
		set(CMAKE_CXX_FLAGS_LOCAL "-Wno-dangling-else")

		# - Wno-dangling-else: workaround for GTEST ambiguous else blocker not working https://github.com/google/googletest/issues/1119
		# disable dangling reference for tests - https://gcc.gnu.org/bugzilla/show_bug.cgi?id=108165#c9
		if(${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER "13")
			set(CMAKE_CXX_FLAGS_LOCAL "${CMAKE_CXX_FLAGS_LOCAL} -Wno-dangling-reference")
		endif()

		# - Wno-free-nonheap-object: bug should be fix in gcc 16 - https://gcc.gnu.org/bugzilla/show_bug.cgi?id=115016
		if (${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER_EQUAL "14" AND ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS "16")
			set(CMAKE_CXX_FLAGS_LOCAL "${CMAKE_CXX_FLAGS_LOCAL} -Wno-free-nonheap-object")
		endif()

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_LOCAL}" PARENT_SCOPE)
	elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
		# - Wno-global-constructors: required for GTEST test definition macros
		# - Wno-zero-as-null-pointer-constant: workaround for GTEST NULL/nullptr mismatch https://github.com/google/googletest/issues/1323
		# - Wno-missing-noreturn: some test functions do not return a value
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
			-Wno-global-constructors \
			-Wno-zero-as-null-pointer-constant \
			-Wno-missing-noreturn"
			PARENT_SCOPE)
	endif()
endfunction()

### define version helpers


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

	target_link_libraries(${TARGET_NAME} build.defaults ${Boost_LIBRARIES})
	string(REGEX MATCH "\.(plugins|tools)" _TARGET_FOLDER "${TARGET_NAME}")
	if(_TARGET_FOLDER)
		set_property(TARGET ${TARGET_NAME} PROPERTY FOLDER "${_TARGET_FOLDER}") 
	endif()

endfunction()

# finds all files comprising a target
function(catapult_find_all_target_files TARGET_TYPE TARGET_NAME)

	message(TRACE "processing ${TARGET_TYPE} '${TARGET_NAME}'")

	file(GLOB TARGET_FILES CONFIGURE_DEPENDS "*.h" "*.cpp")
	SOURCE_GROUP("src" FILES ${TARGET_FILES})

	# add any (optional) subdirectories
	foreach(SUBDIR ${ARGN})
		if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${SUBDIR})
			message(TRACE "+ processing subdirectory '${SUBDIR}'")
			file(GLOB SUBDIR_FILES CONFIGURE_DEPENDS "${SUBDIR}/*.h" "${SUBDIR}/*.cpp")
			SOURCE_GROUP("${SUBDIR}" FILES ${SUBDIR_FILES})
			list(APPEND TARGET_FILES ${SUBDIR_FILES})
		else()
			message(TRACE "!! subdirectory '${SUBDIR}' does not exist in ${CMAKE_CURRENT_SOURCE_DIR}")
		endif()
	endforeach()

	set(${TARGET_NAME}_FILES ${TARGET_FILES} PARENT_SCOPE)

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

# used to define a catapult header only target, creating an appropriate source group in order to allow VS to create an appropriate folder
function(catapult_header_only_target TARGET_NAME)
	if(MSVC)
		catapult_find_all_target_files("hdr" ${TARGET_NAME} ${ARGN})

		if(CMAKE_VERBOSE_MAKEFILE)
			foreach(arg ${ARGN})
				message("adding subdirectory '${arg}'")
			endforeach()
		endif()

		# https://stackoverflow.com/questions/39887352/how-to-create-a-cmake-header-only-library-that-depends-on-external-header-files
		# target_sources doesn't work with interface libraries, but we can use custom_target (with empty action)
		add_custom_target(${TARGET_NAME} SOURCES ${${TARGET_NAME}_FILES})
	endif()
endfunction()

# used to define a catapult test executable
function(catapult_test_executable TARGET_NAME)
	catapult_executable(${TARGET_NAME} ${ARGN})
	add_test(NAME ${TARGET_NAME} WORKING_DIRECTORY ${CMAKE_BINARY_DIR} COMMAND ${TARGET_NAME})
endfunction()

# used to define a catapult test executable for a catapult library by combining catapult_test_executable and
# catapult_target and adding some library dependencies
function(catapult_test_executable_target TARGET_NAME TEST_DEPENDENCY_NAME)
	catapult_test_executable(${TARGET_NAME} ${ARGN})

	# customize and export compiler options for gtest
	catapult_set_test_compiler_options()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" PARENT_SCOPE)

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
	catapult_set_test_compiler_options()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" PARENT_SCOPE)

	target_link_libraries(${TARGET_NAME} tests.catapult.test.${TEST_DEPENDENCY_NAME})
	catapult_target(${TARGET_NAME})
endfunction()

# used to define a catapult tool executable
function(catapult_define_tool TOOL_NAME)
	set(TARGET_NAME catapult.tools.${TOOL_NAME})

	catapult_executable(${TARGET_NAME})
	target_link_libraries(${TARGET_NAME} catapult.tools)
	catapult_target(${TARGET_NAME})

	add_dependencies(${TARGET_NAME} catapult_sdk_publish)
	add_dependencies(tools ${TARGET_NAME})

	install(TARGETS ${TARGET_NAME})
endfunction()
