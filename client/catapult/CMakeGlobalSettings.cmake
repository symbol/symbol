
### define target helper functions

# used to define a catapult target (library, executable) and automatically enables PCH for clang
function(catapult_target TARGET_NAME)

	target_link_libraries(${TARGET_NAME} build.defaults ${Boost_LIBRARIES})
	string(REGEX MATCH "\.(plugins|tools)" _folder "${TARGET_NAME}")
	if(_folder)
		set_property(TARGET ${TARGET_NAME} PROPERTY FOLDER "${_folder}") 
	endif()

endfunction()

# combines catapult_library and catapult_target
function(catapult_library_target TARGET_NAME)
	add_target(${TARGET_NAME} LIBRARY LINK SOURCES ${ARGN})
endfunction()

# used to define a catapult shared library target, creating an appropriate source group and adding a library
function(catapult_shared_library_target TARGET_NAME)
	add_target(${TARGET_NAME} LIBRARY TYPE SHARED LINK SOURCES ${ARGN})
endfunction()

# used to define a catapult executable, creating an appropriate source group and adding an executable
function(catapult_executable TARGET_NAME)
	add_target(${TARGET_NAME} EXECUTABLE LINK SOURCES ${ARGN})
endfunction()

# used to define a catapult header only target, creating an appropriate source group in order to allow VS to create an appropriate folder
function(catapult_header_only_target TARGET_NAME)
	if(MSVC)
		add_target(${TARGET_NAME} CUSTOM SOURCES ${ARGN})
		#add_custom_target(${TARGET_NAME})
		#add_target_sources(${TARGET_NAME} DIRS ${ARGN})
	endif()
endfunction()

# used to define a catapult test executable
function(catapult_test_executable TARGET_NAME)
	add_target(${TARGET_NAME} TEST LINK SOURCES ${ARGN})
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
	catapult_target(${TARGET_NAME})

	target_link_libraries(${TARGET_NAME} catapult.tools)
	add_dependencies(${TARGET_NAME} catapult_sdk_publish)
	add_dependencies(tools ${TARGET_NAME})

	install(TARGETS ${TARGET_NAME})
endfunction()
