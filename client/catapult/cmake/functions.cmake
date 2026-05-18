# This file contains custom CMake functions that are used across the project.

# Description:
#   This wrap the file globbing functionality to retrieve a list of source files from a provided
#   list of directories. Sources globbing expressions are defined as *.h, *.c, *.hpp, *.cpp.
#   Note : CONFIGURE_DEPENDS is used to ensure that CMake re-evaluates the glob and updates the list of sources
#
# Syntax:
#   glob_sources(<out-variable> [RECURSE] [FOLLOW_SYMLINKS] dir1 [dir2 ...])
function(glob_sources OUT_VAR)
	
	set(_fn_options RECURSE FOLLOW_SYMLINKS)
	set(_fn_single RELATIVE)
	set(_fn_multi)

	cmake_parse_arguments(
		PARSE_ARGV 1
		_arg
		"${_fn_options}"
		"${_fn_single}"
		"${_fn_multi}"
	)

	if(_arg_KEYWORDS_MISSING_VALUES)
		message(FATAL_ERROR "glob_sources: missing values for keyword(s): ${_arg_KEYWORDS_MISSING_VALUES}")
	endif()
	if(_arg_RELATIVE)
		cmake_path(
			ABSOLUTE_PATH _arg_RELATIVE
			BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
			NORMALIZE
			OUTPUT_VARIABLE _abs_relative
		)
	endif()
	
	set(_paths)
	if(NOT _arg_UNPARSED_ARGUMENTS)
		list(APPEND _paths "${CMAKE_CURRENT_SOURCE_DIR}")
	else()
		set(_paths ${_arg_UNPARSED_ARGUMENTS})
	endif()

	set(_all_sources)
	foreach(_path IN LISTS _paths)

		cmake_path(
			ABSOLUTE_PATH _path
			BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
			NORMALIZE
			OUTPUT_VARIABLE _abs_path
		)
		cmake_path(
			RELATIVE_PATH _abs_path
			BASE_DIRECTORY "${CMAKE_SOURCE_DIR}"
			OUTPUT_VARIABLE _display_path
		)

		if(NOT IS_DIRECTORY "${_abs_path}")
			message(TRACE "[i] glob_sources: skipping '${_display_path}' since it is not a valid directory")
			continue()
		endif()

		set(_sources)
		set(_call_args)
		if(_arg_RECURSE)
			list(APPEND _call_args GLOB_RECURSE _sources)
			if(_arg_FOLLOW_SYMLINKS)
				list(APPEND _call_args FOLLOW_SYMLINKS)
			endif()
			list(APPEND _call_args LIST_DIRECTORIES false)
		else()
			list(APPEND _call_args GLOB _sources)
		endif()
		if(_arg_RELATIVE)
			list(APPEND _call_args RELATIVE "${_abs_relative}")
		endif()
		list(APPEND _call_args CONFIGURE_DEPENDS)
		
		file(${_call_args} "${_abs_path}/*.h" "${_abs_path}/*.c" "${_abs_path}/*.hpp" "${_abs_path}/*.cpp")
		list(LENGTH _sources _num_sources)
		message(TRACE "[i] glob_sources: '${_display_path}' - found ${_num_sources} source files")
		list(APPEND _all_sources ${_sources})

	endforeach()
	set(${OUT_VAR} ${_all_sources} PARENT_SCOPE)

endfunction()

# Description:
#   This is a wrapper around file(GLOB) to retrieve a list of subdirectories from provided path.
#   If not path is provided, current cmake source directory is used as the base path for globbing.
#
# Syntax:
#   glob_subdirs(<out-variable> [RECURSE] [FOLLOW_SYMLINKS] [RELATIVE <base-dir>] [WITH_CMAKELISTS] [WITH_TESTS true | false] <path>)
function(glob_subdirs OUT_VAR)
	set(_fn_options RECURSE FOLLOW_SYMLINKS WITH_CMAKELISTS)
	set(_fn_single RELATIVE WITH_TESTS)
	set(_fn_multi)

	cmake_parse_arguments(
		PARSE_ARGV 1
		_arg
		"${_fn_options}"
		"${_fn_single}"
		"${_fn_multi}"
	)


	if(_arg_KEYWORDS_MISSING_VALUES)
		message(FATAL_ERROR "glob_subdirs: missing values for keyword(s): ${_arg_KEYWORDS_MISSING_VALUES}")
	endif()
	if(NOT DEFINED _arg_WITH_TESTS)
		set(_arg_WITH_TESTS ${ENABLE_TESTS})
	endif()

	set(_path)
	if(NOT _arg_UNPARSED_ARGUMENTS)
		set(_path "${CMAKE_CURRENT_SOURCE_DIR}")
	else()
		list(LENGTH _arg_UNPARSED_ARGUMENTS _num_paths)
		if(_num_paths GREATER 1)
			message(FATAL_ERROR "glob_subdirs: too many paths provided.\nExpected only one base path for globbing, but got ${_num_paths}.")
		endif()
		list(GET _arg_UNPARSED_ARGUMENTS 0 _path)
	endif()

	cmake_path(
		ABSOLUTE_PATH _path
		BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
		NORMALIZE
		OUTPUT_VARIABLE _abs_path
	)
	if(_arg_RELATIVE)
		cmake_path(
			ABSOLUTE_PATH _arg_RELATIVE
			BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
			NORMALIZE
			OUTPUT_VARIABLE _abs_relative
		)
	endif()
	cmake_path(
		RELATIVE_PATH _abs_path
		BASE_DIRECTORY "${CMAKE_SOURCE_DIR}"
		OUTPUT_VARIABLE _display_path
	)

	if(NOT IS_DIRECTORY "${_abs_path}")
		message(TRACE "[i] glob_subdirs: skipping '${_display_path}' - not a directory")
		return(PROPAGATE ${OUT_VAR})
	endif()

	set(_items)
	set(_call_args)
	if(_arg_RECURSE)
		list(APPEND _call_args GLOB_RECURSE _items)
		if(_arg_FOLLOW_SYMLINKS)
			list(APPEND _call_args FOLLOW_SYMLINKS)
		endif()
		list(APPEND _call_args LIST_DIRECTORIES true)
	else()
		list(APPEND _call_args GLOB _items)
	endif()
	if(_arg_RELATIVE)
		list(APPEND _call_args RELATIVE "${_abs_relative}")
	endif()

	message(TRACE "[i] glob_subdirs: base '${_display_path}'")

	list(APPEND _call_args CONFIGURE_DEPENDS)
	file(${_call_args} "${_abs_path}/*")

	foreach(_item IN LISTS _items)
		if(IS_ABSOLUTE "${_item}")
			set(_abs_item "${_item}")
		elseif(_arg_RELATIVE)
			cmake_path(
				ABSOLUTE_PATH _item
				BASE_DIRECTORY "${_abs_relative}"
				NORMALIZE
				OUTPUT_VARIABLE _abs_item
			)
		else()
			cmake_path(
				ABSOLUTE_PATH _item
				BASE_DIRECTORY "${_abs_path}"
				NORMALIZE
				OUTPUT_VARIABLE _abs_item
			)
		endif()

		if(IS_DIRECTORY "${_abs_item}")
			if(_arg_WITH_CMAKELISTS AND NOT EXISTS "${_abs_item}/CMakeLists.txt")
				message(TRACE "[i] glob_subdirs: skipping '${_item}' - missing CMakeLists.txt file")
				continue()
			endif()
			if(NOT _arg_WITH_TESTS AND "${_item}" MATCHES "tests$")
				message(TRACE "[i] glob_subdirs: skipping '${_item}' - WITH_TESTS is false")
				continue()
			endif()
			list(APPEND ${OUT_VAR} "${_item}")
		endif()
	endforeach()
	return(PROPAGATE ${OUT_VAR})

endfunction()

# Description:
#   This is a wrapper around add_subdirectory that allows to add multiple subdirectories based on
#   a provided base path and optional list of subdirs. If no subdirs are provided, all subdirectories
#   from current cmake directory are added.
# 
# Syntax:
#   add_subdirs([FOLLOW_SYMLINKS] [EXCLUDE_FROM_ALL] [SYSTEM] [WITH_TESTS] [<dir1> [dir2 ...]])
function(add_subdirs)

	set(_fn_options FOLLOW_SYMLINKS EXCLUDE_FROM_ALL SYSTEM)
	set(_fn_single WITH_TESTS)
	set(_fn_multi)

	cmake_parse_arguments(
		PARSE_ARGV 0
		_arg
		"${_fn_options}"
		"${_fn_single}"
		"${_fn_multi}"
	)

	if(NOT DEFINED _arg_WITH_TESTS)
		set(_arg_WITH_TESTS ${ENABLE_TESTS})
	endif()

	set(_glob)

	if(NOT _arg_UNPARSED_ARGUMENTS)
		set(_glob_args)
		if(_arg_FOLLOW_SYMLINKS)
			list(APPEND _glob_args FOLLOW_SYMLINKS)
		endif()
		list(APPEND _glob_args WITH_TESTS ${_arg_WITH_TESTS})
		glob_subdirs(_glob ${_glob_args} WITH_CMAKELISTS "${CMAKE_CURRENT_SOURCE_DIR}")
	else()
		# Traverse each item and transform to absolute path if needed, then set to _glob
		foreach(_item IN LISTS _arg_UNPARSED_ARGUMENTS)
			cmake_path(
				ABSOLUTE_PATH _item
				BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
				NORMALIZE
				OUTPUT_VARIABLE _abs_item
			)
			list(APPEND _glob "${_abs_item}")
		endforeach()
	endif()

	foreach(_dir IN LISTS _glob)
		cmake_path(
			RELATIVE_PATH _dir
			BASE_DIRECTORY "${CMAKE_SOURCE_DIR}"
			OUTPUT_VARIABLE _display_path
		)

		if(NOT IS_DIRECTORY "${_dir}")
			message(TRACE "[i] add_subdirs: skipping '${_display_path}' - not a directory")
			continue()
		endif()
		if(NOT EXISTS "${_dir}/CMakeLists.txt")
			message(TRACE "[i] add_subdirs: skipping '${_display_path}' - missing CMakeLists.txt file")
			continue()
		endif()
		if(NOT _arg_WITH_TESTS AND "${_display_path}" MATCHES "tests$")
			message(TRACE "[i] add_subdirs: skipping '${_display_path}' - WITH_TESTS is false")
			continue()
		endif()

		message(TRACE "[+] adding subdir '${_display_path}'")
		set(_add_subdirectory_args "${_dir}")
		if(_arg_EXCLUDE_FROM_ALL)
			list(APPEND _add_subdirectory_args EXCLUDE_FROM_ALL)
		endif()
		if(_arg_SYSTEM)
			list(APPEND _add_subdirectory_args SYSTEM)
		endif()
		add_subdirectory(${_add_subdirectory_args})
	endforeach()

endfunction()

# Ancillary macro to add_target : 
# parse the arguments provided to add_target and set appropriate variables for further processing.
macro(_parse_arguments)
		if(NOT "${CMAKE_CURRENT_FUNCTION}" STREQUAL "add_target")
			message(FATAL_ERROR "_parse_arguments: must only be used from add_target().")
		endif()
		cmake_parse_arguments(
			PARSE_ARGV 2
			_arg
			"${_fn_options}"
			"${_fn_single}"
			"${_fn_multi}"
		)
		if(_arg_UNPARSED_ARGUMENTS)
			message(FATAL_ERROR "add_target ${TNAME} (${TTYPE}): unrecognized arguments: ${_arg_UNPARSED_ARGUMENTS}.")
		endif()
		if(_arg_KEYWORDS_MISSING_VALUES)
			message(FATAL_ERROR "glob_sources: missing values for keyword(s): ${_arg_KEYWORDS_MISSING_VALUES}")
		endif()

		#if("INCLUDE_DIRS" IN_LIST _arg_KEYWORDS_MISSING_VALUES)
		#	message(FATAL_ERROR "add_target ${TNAME} (${TTYPE}): missing value for INCLUDE_DIRS.")
		#endif()
endmacro()

# Ancillary macro to add_target
# handle the dependencies and dependents for the target based on the provided arguments.
macro(_handle_dependencies_dependents)
	if(NOT "${CMAKE_CURRENT_FUNCTION}" STREQUAL "add_target")
		message(FATAL_ERROR "_handle_dependencies_dependents: must only be used from add_target().")
	endif()
	foreach(_dep IN LISTS _arg_DEPENDENCIES)
		add_dependencies(${TNAME} ${_dep})
	endforeach()
	foreach(_dep IN LISTS _arg_DEPENDENTS)
		add_dependencies(${_dep} ${TNAME})
	endforeach()
endmacro()

# Ancillary macro to add_target
# handle the sources for the target based on the provided arguments.
macro(_handle_sources)
	if(NOT "${CMAKE_CURRENT_FUNCTION}" STREQUAL "add_target")
		message(FATAL_ERROR "_handle_sources: must only be used from add_target().")
	endif()
	if(DEFINED _arg_SOURCES)
		set(_sources_list)
		glob_sources(_sources_list ${_arg_SOURCES})
		if(_arg_TYPE STREQUAL "INTERFACE")
			list(FILTER _sources_list EXCLUDE REGEX "\\.c(pp)?$")
			target_sources(${TNAME} INTERFACE ${_sources_list})
		else()
			target_sources(${TNAME} PRIVATE ${_sources_list})
		endif()
	endif()
endmacro()

# Ancillary macro to add_target
# handle the include directories for the target based on the provided arguments.
macro(_handle_includes)
	if(NOT "${CMAKE_CURRENT_FUNCTION}" STREQUAL "add_target")
		message(FATAL_ERROR "_handle_includes: must only be used from add_target().")
	endif()
	if(_arg_INCLUDE_DIRS OR "INCLUDE_DIRS" IN_LIST _arg_KEYWORDS_MISSING_VALUES)
		if(_arg_TYPE STREQUAL "INTERFACE")
			target_include_directories(${TNAME} INTERFACE ${_arg_INCLUDE_DIRS})
		else()
			target_include_directories(${TNAME} PRIVATE ${_arg_INCLUDE_DIRS})
		endif()
	endif()
endmacro()

# Ancillary macro to add_target
# handle the link libraries for the target based on the provided arguments.
macro(_handle_link_libs)
	if(NOT "${CMAKE_CURRENT_FUNCTION}" STREQUAL "add_target")
		message(FATAL_ERROR "_handle_link_libs: must only be used from add_target().")
	endif()

	set(_link_libs
		build.defaults		# Default compiler and linker flags for all targets
		${Boost_LIBRARIES}	# Ubiquitous Boost libraries for all targets (if any)
		${_arg_LINK_LIBS}	# Any user-provided link libraries for this target
	)
	if(_arg_TYPE STREQUAL "INTERFACE")
		set(_link_mode INTERFACE)
	endif()
	target_link_libraries(${TNAME} ${_link_mode} ${_link_libs})

endmacro()

# Description:
#
#	Wraps the add_library, add_executable, add_test call for a target, allowing to define 
#	custom target types (e.g. header-only) without having to duplicate the logic for 
#	adding sources, setting properties, etc.
#
#   Unlike wrapped original methods where sources are expected to be a list of files, 
#   here the sources are expected to be provided as a list of directories, which are then 
#   traversed to find .h and .cpp files to add to the target. 
#   This allows for more flexible organization of source files without having to update 
#   CMakeLists.txt every time a new file is added.
#   List each source directory explicitly. Use './' to add sources from current cmake directory
#   and list any additional subdirs relative to the current cmake directory (e.g. SOURCES ./ utils)
#
# Syntax:
#	add_target(<target-name> LIBRARY 
#		[TYPE STATIC|SHARED|MODULE|OBJECT|INTERFACE] 
#		[EXCLUDE_FROM_ALL]
#		[WITH_INSTALL]
#		[INCLUDE_DIRS dir1 [dir2 ...]]
#		[LINK_LIBS lib1 [lib2 ...]] 
#		[DEPENDENCIES target1 [target2 ...]]
#		[DEPENDENTS target1 [target2 ...]]
#		[SOURCES <sources>...])
#
#   for this project we create tests along with executables and eventually add_test for them 
#   assuming COMMAND is the executable itself.
#	add_target(<target-name> EXECUTABLE
#		[WIN32] [MACOSX_BUNDLE] 
#		[EXCLUDE_FROM_ALL]
#		[WITH_INSTALL]
#		[INCLUDE_DIRS dir1 [dir2 ...]]
#		[LINK_LIBS [lib1 lib2 ...]] 
#		[DEPENDENCIES target1 [target2 ...]]
#		[DEPENDENTS target1 [target2 ...]]
#		[SOURCES <sources>...])
#	add_target(<target-name> TEST|TOOL
#		[INCLUDE_DIRS dir1 [dir2 ...]]
#		[LINK_LIBS lib1 [lib2 ...]] 
#		[DEPENDENCIES target1 [target2 ...]]
#		[DEPENDENTS target1 [target2 ...]]
#		[SOURCES <sources>...])
#		[LABELS label1 [label2 ...]])
#	add_target(<target-name> CUSTOM
#		[SOURCES <sources>...])
#
# Note:
#	the target must not already exist when calling this function, otherwise an error is raised.
function(add_target TNAME TTYPE)

	set(_supported_target_types "LIBRARY;EXECUTABLE;TEST;TOOL;CUSTOM")

	if(${TNAME} STREQUAL "")
		message(FATAL_ERROR "add_target: target name cannot be empty.")
	elseif(TARGET ${TNAME})
		message(FATAL_ERROR "add_target: target '${TNAME}' already exists.")
	elseif(NOT "${TTYPE}" IN_LIST _supported_target_types)
		message(FATAL_ERROR "add_target: unsupported target type '${TTYPE}'. Supported types are: ${_supported_target_types}.")
	endif()
	
	# Prepare for argument parsing
	set(_fn_options)
	set(_fn_single)
	set(_fn_multi INCLUDE_DIRS LINK_LIBS DEPENDENCIES DEPENDENTS SOURCES)

	# Call appropriate add_* function based on the target type
	if(TTYPE STREQUAL "LIBRARY")
		
		list(APPEND _fn_options EXCLUDE_FROM_ALL WITH_INSTALL)
		list(APPEND _fn_single TYPE)
		_parse_arguments()

		if(NOT DEFINED _arg_TYPE)
			if(BUILD_SHARED_LIBS)
				set(_arg_TYPE "SHARED")
			else()
				set(_arg_TYPE "STATIC")
			endif()
		endif()
		set(_supported_types "STATIC;SHARED;MODULE;OBJECT;INTERFACE")		
		if(NOT _arg_TYPE IN_LIST _supported_types)
			message(FATAL_ERROR "add_target LIBRARY: missing or unsupported TYPE ${_arg_TYPE} for '${TNAME}'. Supported types are: ${_supported_types}.")
		endif()

		message(TRACE "[+] adding ${_arg_TYPE} LIBRARY '${TNAME}'")
		add_library(${TNAME} ${_arg_TYPE})

		_handle_link_libs()
		_handle_includes()
		_handle_sources()
		_handle_dependencies_dependents()

		if(_arg_EXCLUDE_FROM_ALL)
			set_target_properties(${TNAME} PROPERTIES EXCLUDE_FROM_ALL TRUE)
		endif()

		if(_arg_TYPE STREQUAL "SHARED")
			set(_arg_WITH_INSTALL ON)
			target_sources(${TNAME} PRIVATE ${VERSION_RESOURCES})
			if(MSVC)
				# Old set_win_version_definitions logic
				target_compile_definitions(${TNAME} PUBLIC
					CATAPULT_VERSION_DESCRIPTION="${CATAPULT_VERSION_DESCRIPTION}"
					WIN_FILETYPE=VFT_DLL
					$<$<BOOL:${CATAPULT_BUILD_RELEASE}>:CATAPULT_BUILD_RELEASE=1>
				)
			endif()
		endif()

		if(_arg_WITH_INSTALL)
			install(TARGETS ${TNAME})
		endif()

	elseif(TTYPE STREQUAL "EXECUTABLE")

		list(APPEND _fn_options WIN32 MACOSX_BUNDLE EXCLUDE_FROM_ALL WITH_INSTALL)
		_parse_arguments()
		
		message(TRACE "[+] adding EXECUTABLE '${TNAME}'")
		add_executable(${TNAME} ${VERSION_RESOURCES})

		set(_link_libs build.defaults ${Boost_LIBRARIES} ${_arg_LINK_LIBS})
		target_link_libraries(${TNAME} ${_link_libs})

		if(_arg_EXCLUDE_FROM_ALL)
			set_target_properties(${TNAME} PROPERTIES EXCLUDE_FROM_ALL TRUE)
		endif()
		if(_arg_WIN32)
			set_target_properties(${TNAME} PROPERTIES WIN32_EXECUTABLE TRUE)
		endif()
		if(_arg_MACOSX_BUNDLE)
			set_target_properties(${TNAME} PROPERTIES MACOSX_BUNDLE TRUE)
		endif()

		_handle_includes()
		_handle_sources()
		_handle_dependencies_dependents()

		if(MSVC)
			target_compile_definitions(${TNAME} PUBLIC
				CATAPULT_VERSION_DESCRIPTION="${CATAPULT_VERSION_DESCRIPTION}"
				WIN_FILETYPE=VFT_APP
				$<$<BOOL:${CATAPULT_BUILD_RELEASE}>:CATAPULT_BUILD_RELEASE=1>
			)
		endif()
		if(WIN32 AND MINGW)
			target_link_libraries(${TNAME} wsock32 ws2_32)
		endif()

		if(_arg_WITH_INSTALL)
			install(TARGETS ${TNAME})
		endif()

	elseif(TTYPE STREQUAL "TEST")

		
		list(APPEND _fn_multi LABELS)

		# Test targets should be in the form test.xyz so it's possible to derive xyz as the library under test
		# and automatically link it to the test executable. 
		# There are cases though where this is not possible or desireable
		# To allow to skip this automatic linking, pass NO_DERIVE_LIB as an argument.
		list(APPEND _fn_options NO_DERIVED_LIB)

		_parse_arguments()

		set(_derived_lib)
		if(NOT _arg_NO_DERIVED_LIB)
			string(REPLACE "." ";" _parts "${TNAME}")
			list(LENGTH _parts _num_parts)
			if(_num_parts LESS 2)
				message(FATAL_ERROR "add_target TEST: unexpected test target name '${TNAME}'. Expected format is 'test.xyz' where xyz is the library under test.")
			endif()
			list(REMOVE_AT _parts 0)
			string(JOIN "." _derived_lib ${_parts})
		endif()
		
		list(PREPEND _arg_LINK_LIBS build.tests ${_derived_lib})

		set(_call_args EXECUTABLE LINK_LIBS ${_arg_LINK_LIBS})

		if(DEFINED _arg_DEPENDENCIES)
			list(APPEND _call_args DEPENDENCIES ${_arg_DEPENDENCIES})
		endif()
		if(DEFINED _arg_DEPENDENTS)
			list(APPEND _call_args DEPENDENTS ${_arg_DEPENDENTS})
		endif()
		if(DEFINED _arg_INCLUDE_DIRS)
			list(APPEND _call_args INCLUDE_DIRS ${_arg_INCLUDE_DIRS})
		endif()
		if(DEFINED _arg_SOURCES)
			list(APPEND _call_args SOURCES ${_arg_SOURCES})
		endif()
		add_target(${TNAME} ${_call_args})
		add_test(NAME ${TNAME} WORKING_DIRECTORY ${CMAKE_BINARY_DIR} COMMAND ${TNAME})

		if(_arg_LABELS)
			set_property(TEST ${TNAME} PROPERTY LABELS ${_arg_LABELS})
		endif()
		
	elseif(TTYPE STREQUAL "TOOL")
		
		_parse_arguments()

		set(TNAME catapult.tools.${TNAME})
		list(PREPEND _arg_LINK_LIBS catapult.tools)
		list(PREPEND _arg_DEPENDENCIES catapult_sdk_publish)
		list(PREPEND _arg_DEPENDENTS tools)
		set(_call_args EXECUTABLE LINK_LIBS ${_arg_LINK_LIBS} DEPENDENCIES ${_arg_DEPENDENCIES} DEPENDENTS ${_arg_DEPENDENTS})

		if(DEFINED _arg_INCLUDE_DIRS)
			list(APPEND _call_args INCLUDE_DIRS ${_arg_INCLUDE_DIRS})
		endif()
		if(DEFINED _arg_SOURCES)
			list(APPEND _call_args SOURCES ${_arg_SOURCES})
		endif()

		add_target(${TNAME} ${_call_args})

	elseif(TTYPE STREQUAL "CUSTOM")
		
		_parse_arguments()

		add_custom_target(${TNAME})

		_handle_link_libs()
		_handle_includes()
		_handle_sources()
		_handle_dependencies_dependents()

	endif()

endfunction()
