
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
	string(REGEX MATCH "\.(plugins|tools)" _folder "${TARGET_NAME}")
	if(_folder)
		set_property(TARGET ${TARGET_NAME} PROPERTY FOLDER "${_folder}") 
	endif()

endfunction()

# finds all files comprising a target
function(catapult_find_all_target_files TARGET_TYPE TARGET_NAME)

	message(TRACE "processing ${TARGET_TYPE} '${TARGET_NAME}'")

	file(GLOB TARGET_FILES LIST_DIRECTORIES false CONFIGURE_DEPENDS "*.h" "*.cpp")
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

# combines catapult_library and catapult_target
function(catapult_library_target TARGET_NAME)
	catapult_find_all_target_files("lib" ${TARGET_NAME} ${ARGN})
	add_library(${TARGET_NAME} ${${TARGET_NAME}_FILES})
	catapult_target(${TARGET_NAME})
endfunction()

# used to define a catapult shared library target, creating an appropriate source group and adding a library
function(catapult_shared_library_target TARGET_NAME)
	catapult_find_all_target_files("shared lib" ${TARGET_NAME} ${ARGN})
	add_definitions(-DDLL_EXPORTS)
	if(MSVC)
		set_win_version_definitions(${TARGET_NAME} VFT_DLL)
	endif()
	add_library(${TARGET_NAME} SHARED ${${TARGET_NAME}_FILES} ${VERSION_RESOURCES})
	catapult_target(${TARGET_NAME})
	install(TARGETS ${TARGET_NAME})
endfunction()

# used to define a catapult executable, creating an appropriate source group and adding an executable
function(catapult_executable TARGET_NAME)
	catapult_find_all_target_files("exe" ${TARGET_NAME} ${ARGN})

	if(MSVC_EXTENDED)
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
# catapult_target and adding some library dependencies.
# pass NOLIB as the third argument to skip linking the library under test (header-only or already pulled in transitively)
function(catapult_test_executable_target TARGET_NAME TEST_DEPENDENCY_NAME)
	cmake_parse_arguments(_ARG "NOLIB" "" "" ${ARGN})
	catapult_test_executable(${TARGET_NAME} ${_ARG_UNPARSED_ARGUMENTS})

	set(_extra_libs "")
	if(NOT _ARG_NOLIB)
		# test targets are in the form test.xyz, so derive xyz as the library under test
		string(REPLACE "." ";" _parts "${TARGET_NAME}")
		list(LENGTH _parts _num_parts)
		if(_num_parts LESS 2)
			message(FATAL_ERROR "unexpected test target name '${TARGET_NAME}'")
		endif()
		list(REMOVE_AT _parts 0)
		string(JOIN "." _extra_libs ${_parts})
	endif()

	target_link_libraries(${TARGET_NAME} build.tests tests.catapult.test.${TEST_DEPENDENCY_NAME} ${_extra_libs})
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
