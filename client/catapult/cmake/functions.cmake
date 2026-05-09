# This file contains custom CMake functions that are used across the project.

# Validates and resolves the base directory for subdirectory discovery
macro(_subdirectories_resolve_base _output_variable _provided_base)
	if(_provided_base)
		set(${_output_variable} "${_provided_base}")
	else()
		set(${_output_variable} "${CMAKE_CURRENT_SOURCE_DIR}")
	endif()

	if(NOT IS_ABSOLUTE "${${_output_variable}}")
		cmake_path(
			ABSOLUTE_PATH "${${_output_variable}}"
			BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
			OUTPUT_VARIABLE ${_output_variable}
		)
	endif()

	if(NOT IS_DIRECTORY "${${_output_variable}}")
		message(FATAL_ERROR "subdirectories: base directory '${${_output_variable}}' is not a valid directory")
	endif()
endmacro()

# Description:
#	Retrieves the list of subdirectories from a provided base path
	# Optionally filters to only include subdirectories that contain a CMakeLists.txt file
	# so the result(s) can be directly used for add_subdirectory calls
# Syntax: 
#	_subdirectories_collect(<out-variable> <base-dir> [FOLLOW_SYMLINKS] [WITH_CMAKELISTS])
function(_subdirectories_collect OUT_VAR BASE_DIR)
	if(ARGC LESS 2)
		message(FATAL_ERROR "[!] _subdirectories_collect: missing required arguments. Expected OUT_VAR and BASE_DIR.")
	endif()

	set(_fn_options FOLLOW_SYMLINKS WITH_CMAKELISTS)
	cmake_parse_arguments(
		PARSE_ARGV 2
		_arg
		"${_fn_options}"
		""
		""
	)

	if(_arg_UNPARSED_ARGUMENTS)
		message(FATAL_ERROR "[!] _subdirectories_collect: unrecognized arguments: ${_arg_UNPARSED_ARGUMENTS}.")
	endif()

	set(_glob_args LIST_DIRECTORIES true CONFIGURE_DEPENDS)
	if(_arg_FOLLOW_SYMLINKS)
		list(APPEND _glob_args FOLLOW_SYMLINKS)
	endif()
	
	file(GLOB _items ${_glob_args} "${BASE_DIR}/*")

	set(_dirs)
	foreach(_item IN LISTS _items)
		if(IS_DIRECTORY "${_item}")
			if(_arg_WITH_CMAKELISTS AND NOT EXISTS "${_item}/CMakeLists.txt")
				continue()
			endif()
			list(APPEND _dirs "${_item}")
		endif()
	endforeach()

	set(${OUT_VAR} ${_dirs} PARENT_SCOPE)
endfunction()

# Description:
#	Main function to either get a list of subdirectories or add subdirectories based 
#   on a provided base path.
# Syntax:
#   subdirectories(LIST <out-var> BASE <path> [RELATIVE] [WITH_CMAKELISTS] [FOLLOW_SYMLINKS])
#   subdirectories(ADD  BASE <path> [WITH_TESTS true | false] [FOLLOW_SYMLINKS])
#   subdirectories(ADD  BASE <path> [DIRS path1 path2 pathN] [WITH_TESTS true | false] [FOLLOW_SYMLINKS])
function(subdirectories)
	if(ARGC EQUAL 0)
		message(FATAL_ERROR "[!] subdirectories: missing subcommand. Expected LIST or ADD.")
	endif()

	set(_mode "${ARGV0}")

	if(_mode STREQUAL "LIST")
		
		if(ARGC LESS 3)
			message(FATAL_ERROR "[!] subdirectories LIST: missing required arguments. Expected <out-var> and <base-dir>")
		endif()

		set(_out_var "${ARGV1}")

		set(_fn_options RELATIVE WITH_CMAKELISTS FOLLOW_SYMLINKS)
		set(_fn_single BASE)
		cmake_parse_arguments(
			PARSE_ARGV 2
			_arg
			"${_fn_options}"
			"${_fn_single}"
			""
		)
		if(_arg_UNPARSED_ARGUMENTS)
			message(FATAL_ERROR "[!] subdirectories LIST: unrecognized arguments: ${_arg_UNPARSED_ARGUMENTS}.")
		endif()
		if(NOT _arg_BASE)
			set(_arg_BASE "${CMAKE_CURRENT_SOURCE_DIR}")
		endif()

		set(_call_args)
		if(_arg_WITH_CMAKELISTS)
			list(APPEND _call_args WITH_CMAKELISTS)
		endif()
		if(_arg_FOLLOW_SYMLINKS)
			list(APPEND _call_args FOLLOW_SYMLINKS)
		endif()

		_subdirectories_resolve_base(_base "${_arg_BASE}")
		_subdirectories_collect(_collected "${_base}" ${_call_args})

		set(_result)
		foreach(_dir IN LISTS _collected)
			if(_arg_RELATIVE)
				file(RELATIVE_PATH _dir "${_base}" "${_dir}")
			endif()
			list(APPEND _result "${_dir}")
		endforeach()

		set(${_out_var} ${_result} PARENT_SCOPE)

	elseif(_mode STREQUAL "ADD")

		set(_fn_options FOLLOW_SYMLINKS)
		set(_fn_single BASE WITH_TESTS)
		set(_fn_multi DIRS)

		cmake_parse_arguments(
			PARSE_ARGV 1
			_arg
			"${_fn_options}"
			"${_fn_single}"
			"${_fn_multi}"
		)

		if(_arg_UNPARSED_ARGUMENTS)
			message(FATAL_ERROR "[!] subdirectories ADD: unrecognized arguments: ${_arg_UNPARSED_ARGUMENTS}.")
		endif()

		_subdirectories_resolve_base(_arg_BASE "${_arg_BASE}")

		if(NOT DEFINED _arg_WITH_TESTS)
			set(_arg_WITH_TESTS ${ENABLE_TESTS})
		endif()

		if(NOT _arg_DIRS)

			set(_call_args WITH_CMAKELISTS)
			if(_arg_FOLLOW_SYMLINKS)
				list(APPEND _call_args FOLLOW_SYMLINKS)
			endif()

			# Call self with LIST mode to discover all
			subdirectories(LIST _collected BASE "${_arg_BASE}" ${_call_args})
		else()
			# Use provided list of directories, 
			# but validate them and filter to only those that contain CMakeLists.txt 
			# (if any don't, print a warning and skip them)
			set(_collected)
			foreach(_dir IN LISTS _arg_DIRS)
				# resolve the directory path relative to the base
				if(IS_ABSOLUTE "${_dir}")
					set(_resolved_dir "${_dir}")
				else()
					set(_resolved_dir "${_arg_BASE}/${_dir}")
				endif()
				if(NOT IS_DIRECTORY "${_resolved_dir}")
					message(TRACE "[i] subdirectories ADD: skipping '${_dir}' since it is not a valid directory (resolved path: '${_resolved_dir}')")
					continue()
				endif()
				list(APPEND _collected "${_resolved_dir}")
			endforeach()
		endif()

		foreach(_dir IN LISTS _collected)
			if(NOT EXISTS "${_dir}/CMakeLists.txt")
				message(TRACE "[i] subdirectories ADD: directory '${_dir}' no longer contains a CMakeLists.txt file. Skipped")
				continue()
			endif()
			if(NOT _arg_WITH_TESTS AND "${_dir}" MATCHES "tests$")
				message(TRACE "[i] subdirectories ADD: skipping '${_dir}' subdirectory since WITH_TESTS is false")
				continue()
			endif()
			message(TRACE "[+] Adding subdirectory '${_dir}'")
			add_subdirectory("${_dir}")
		endforeach()

	else()
		message(FATAL_ERROR "subdirectories: unsupported subcommand '${_mode}'. Expected LIST or ADD.")
	endif()
endfunction()

# Description:
#	Main function to add source files to a target based on a provided base path and optional list
#	of subdirectories. Only .h and .cpp files are added. 
#	If no subdirectories are provided, all .h and .cpp files in the base directory are added.
# Syntax:
#    add_target_sources(<target-name> BASE <path> [DIRS path1 path2 pathN])
# Note: 
#	the target must already exist when calling this function, otherwise an error is raised.
function(add_target_sources TARGET_NAME)

	if(NOT TARGET ${TARGET_NAME})
		message(FATAL_ERROR "add_target_sources: target '${TARGET_NAME}' does not exist yet.")
	endif()
	
	get_target_property(TARGET_TYPE ${TARGET_NAME} TYPE)
	message(TRACE "[i]\tadding sources to '${TARGET_NAME}' (${TARGET_TYPE}) from .")

	# find all source files in the BASE directory if provided, otherwise in the current source directory
	set(_fn_single BASE)
	set(_fn_multi DIRS)

	cmake_parse_arguments(
		PARSE_ARGV 1
		_arg
		""
		"${_fn_single}"
		"${_fn_multi}"
	)
	_subdirectories_resolve_base(_arg_BASE "${_arg_BASE}")

	file(GLOB _files LIST_DIRECTORIES false CONFIGURE_DEPENDS "${_arg_BASE}/*.h" "${_arg_BASE}/*.cpp")
	target_sources(${TARGET_NAME} PRIVATE ${_files})

	# traverse all subdirs if provided still only looking for .h and .cpp files, and add them to the target sources as well
	foreach(_item IN LISTS _arg_DIRS)
		# resolve the directory path relative to the base
		if(IS_ABSOLUTE "${_item}")
			set(_resolved_dir "${_item}")
		else()
			set(_resolved_dir "${_arg_BASE}/${_item}")
		endif()
		if(NOT IS_DIRECTORY "${_resolved_dir}")
			message(TRACE "[i]\tskipping '${_resolved_dir}' for target '${TARGET_NAME}' since it is not a valid directory")
			continue()
		endif()
		message(TRACE "[i]\tadding sources to '${TARGET_NAME}' (${TARGET_TYPE}) from ./${_item}")
		file(GLOB _subdir_files LIST_DIRECTORIES false CONFIGURE_DEPENDS "${_resolved_dir}/*.h" "${_resolved_dir}/*.cpp")
		target_sources(${TARGET_NAME} PRIVATE ${_subdir_files})
	endforeach()
endfunction()

# Description:
#	Wraps the add_library or add_executable call for a target, allowing to define 
#	custom target types (e.g. header-only) without having to duplicate the logic for 
#	adding sources, setting properties, etc.
# Note:
#	the target must not already exist when calling this function, otherwise an error is raised.
function(add_target TARGET_NAME)

	if(TARGET ${TARGET_NAME})
		message(FATAL_ERROR "add_target: target '${TARGET_NAME}' already exists.")
	endif()

	set(_supported_target_types "LIBRARY;EXECUTABLE")

	# Parse arguments
	set(_fn_options)
	set(_fn_single TYPE LINKAGE)
	set(_fn_multi SOURCE_DIRS)

	cmake_parse_arguments(
		PARSE_ARGV 1
		_arg
		"${_fn_options}"
		"${_fn_single}"
		"${_fn_multi}"
	)

	# Validate arguments
	if(NOT DEFINED _arg_TYPE OR NOT ${_arg_TYPE} IN_LIST _supported_target_types)
		message(FATAL_ERROR "add_target: unsupported target type '${_arg_TYPE}'. Supported types are: ${_supported_target_types}.")
	endif()

	# Call appropriate add_* function based on the target type
	if(_arg_TYPE STREQUAL "LIBRARY")
		if(NOT DEFINED _arg_LINKAGE)
			if(BUILD_SHARED_LIBS)
				set(_arg_LINKAGE "SHARED")
			else()
				set(_arg_LINKAGE "STATIC")
			endif()
		endif()

		set(_supported_linkages "STATIC;SHARED;MODULE")
		if(NOT _arg_LINKAGE IN_LIST _supported_linkages)
			message(FATAL_ERROR "add_target: missing or unsupported LINKAGE ${_arg_LINKAGE} for library '${TARGET_NAME}'. Supported linkages are: ${_supported_linkages}.")
		endif()

		add_library(${TARGET_NAME} ${_arg_LINKAGE})

		if(DEFINED _arg_SOURCE_DIRS OR "SOURCE_DIRS" IN_LIST _arg_KEYWORDS_MISSING_VALUES)
			add_target_sources(${TARGET_NAME} BASE . DIRS ${_arg_SOURCE_DIRS})
		endif()

		if(_arg_LINKAGE STREQUAL "SHARED")
			target_sources(${TARGET_NAME} PRIVATE ${VERSION_RESOURCES})
			if(MSVC)
				# Old set_win_version_definitions logic
				target_compile_definitions(${TARGET_NAME} PUBLIC
					CATAPULT_VERSION_DESCRIPTION="${CATAPULT_VERSION_DESCRIPTION}"
					WIN_FILETYPE=VFT_DLL
					$<$<BOOL:${CATAPULT_BUILD_RELEASE}>:CATAPULT_BUILD_RELEASE=1>
				)
			endif()
		endif()

		# Old catapult_target logic
		target_link_libraries(${TARGET_NAME} build.defaults ${Boost_LIBRARIES})
		string(REGEX MATCH "\.(plugins|tools)" _folder "${TARGET_NAME}")
		if(_folder)
			set_property(TARGET ${TARGET_NAME} PROPERTY FOLDER "${_folder}") 
		endif()

		if(_arg_LINKAGE STREQUAL "SHARED")
			install(TARGETS ${TARGET_NAME})
		endif()

	elseif(_arg_TYPE STREQUAL "EXECUTABLE")
		
		add_executable(${TARGET_NAME} ${VERSION_RESOURCES})
		if(DEFINED _arg_SOURCE_DIRS OR "SOURCE_DIRS" IN_LIST _arg_KEYWORDS_MISSING_VALUES)
			add_target_sources(${TARGET_NAME} BASE . DIRS ${_arg_SOURCE_DIRS})
		endif()
		if(MSVC)
			target_compile_definitions(${TARGET_NAME} PUBLIC
				CATAPULT_VERSION_DESCRIPTION="${CATAPULT_VERSION_DESCRIPTION}"
				WIN_FILETYPE=VFT_APP
				$<$<BOOL:${CATAPULT_BUILD_RELEASE}>:CATAPULT_BUILD_RELEASE=1>
			)
		endif()
		if(WIN32 AND MINGW)
			target_link_libraries(${TARGET_NAME} wsock32 ws2_32)
		endif()

	endif()

endfunction()
