function(add_mongo_plugin TNAME)

	set(_fn_options WITH_DEPS WITH_HEADER_ONLY)
	set(_fn_single)
	set(_fn_multi LINK_LIBS SOURCES)

	cmake_parse_arguments(
		PARSE_ARGV 1
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

	# Validate plugin name
	if(NOT TNAME MATCHES "^catapult\\.mongo\\.plugins\\.[a-zA-Z0-9_]+$")
		if(TNAME MATCHES "^[a-zA-Z0-9_]+$")
			set(_plugin_name ${TNAME})
			set(_plugin_full_name "catapult.mongo.plugins.${_plugin_name}")
		else()
			message(FATAL_ERROR "add_mongo_plugin ${TNAME}: invalid plugin name.")
		endif()
	else()
		string(REGEX MATCH "[^.]+$" _plugin_name ${TNAME})
		set(_plugin_full_name ${TNAME})
	endif()

	# Ensure all the items in the SOURCES list are DIRECTORY items.
	foreach(_src IN LISTS _arg_SOURCES)
		cmake_path(IS_RELATIVE _src _is_relative)
		if(_is_relative)
			cmake_path(
				ABSOLUTE_PATH _src
				BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
				NORMALIZE
				OUTPUT_VARIABLE _src
			)
		endif()
		if(NOT IS_DIRECTORY "${_src}")
			message(FATAL_ERROR "add_target ${TNAME}: all items in SOURCES must be directories. '${_src}' is not a directory.")
		endif()
	endforeach()

	# Default libraries to link with
	set(_link_libs ${MongoDB_IMPORTED_TARGETS} catapult.mongo)
	
	# If the plugin has a corresponding plugin library, link with it.
	if(TARGET catapult.plugins.${_plugin_name})
		list(APPEND _link_libs catapult.plugins.${_plugin_name})
	endif()
	
	# Extra user-defined libraries to link with
	if(_arg_LINK_LIBS)
		list(APPEND _link_libs ${_arg_LINK_LIBS})
	endif()	

	list(REMOVE_DUPLICATES _link_libs)

	if(_arg_WITH_HEADER_ONLY)
		add_target(
			${_plugin_full_name} LIBRARY TYPE INTERFACE
			SOURCES ${_arg_SOURCES}
		)
		return()
	endif()

	if(_arg_WITH_DEPS)
		if(TARGET catapult.plugins.${_plugin_name}.deps)
			list(APPEND _link_libs catapult.plugins.${_plugin_name}.deps)
		endif()
		add_target(
			${_plugin_full_name}.deps LIBRARY
			LINK_LIBS ${_link_libs}
			DEPENDENTS mongo
			SOURCES ${_arg_SOURCES}
		)
		list(APPEND _link_libs ${_plugin_full_name}.deps)
		list(SUBLIST _arg_SOURCES 0 1 _arg_sources)
	endif()

	add_target(
		${_plugin_full_name} LIBRARY TYPE SHARED
		LINK_LIBS ${_link_libs}
		DEPENDENTS mongo
		SOURCES ${_arg_SOURCES}
	)

endfunction()

function(add_mongo_plugin_tests TNAME)

	# Validate plugin name
	if(NOT TNAME MATCHES "^tests\\.catapult\\.mongo\\.plugins\\.[a-zA-Z0-9_]+$")
		if(TNAME MATCHES "^[a-zA-Z0-9_]+$")
			set(_plugin_name ${TNAME})
			set(_plugin_full_name "tests.catapult.mongo.plugins.${_plugin_name}")
		else()
			message(FATAL_ERROR "add_mongo_plugin_tests ${TNAME}: invalid plugin name.")
		endif()
	else()
		string(REGEX MATCH "[^.]+$" _plugin_name ${TNAME})
		set(_plugin_full_name ${TNAME})
	endif()


endfunction()

function(catapult_mongo_plugin_tests_no_deps TARGET_NAME)

	add_target(${TARGET_NAME} TEST
		LINK_LIBS 
			${MongoDB_IMPORTED_TARGETS}
			tests.catapult.test.mongo
		SOURCES ./
	)

endfunction()

function(catapult_mongo_plugin_tests_with_header_only_deps TARGET_NAME)
	string(REGEX MATCH "\.[a-z_]+$" PLUGIN_NAME ${TARGET_NAME})
	string(SUBSTRING ${PLUGIN_NAME} 1 -1 PLUGIN_NAME)

	add_target(${TARGET_NAME} TEST
		LINK_LIBS 
			${MongoDB_IMPORTED_TARGETS}
			tests.catapult.test.mongo
			catapult.plugins.${PLUGIN_NAME}.deps
		SOURCES 
			./ 
			${ARGN}
	)
	if(TARGET "tests.catapult.test.plugins.${PLUGIN_NAME}")
		target_link_libraries(${TARGET_NAME} tests.catapult.test.plugins.${PLUGIN_NAME})
	endif()

endfunction()

function(catapult_mongo_plugin_tests_with_deps TARGET_NAME)
	string(REGEX MATCH "\.[a-z]+$" PLUGIN_NAME ${TARGET_NAME})
	string(SUBSTRING ${PLUGIN_NAME} 1 -1 PLUGIN_NAME)

	catapult_mongo_plugin_tests_with_header_only_deps(${TARGET_NAME} "${ARGN}")
	target_link_libraries(${TARGET_NAME} tests.catapult.test.mongo.plugins.${PLUGIN_NAME})
endfunction()

function(catapult_mongo_plugin_test_library TARGET_NAME)
	add_target(
		${TARGET_NAME} LIBRARY
		LINK_LIBS 
			${MongoDB_IMPORTED_TARGETS} 
			${GTest_IMPORTED_TARGETS}
		SOURCES ./
	)
endfunction()

#add_subdirs()

glob_subdirs(_all_plugins RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
foreach(_item IN LISTS _all_plugins)
	
	cmake_path(SET _plugin_src_dir NORMALIZE "${CMAKE_CURRENT_SOURCE_DIR}/${_item}/src")
	cmake_path(SET _plugin_tests_dir NORMALIZE "${CMAKE_CURRENT_SOURCE_DIR}/${_item}/tests")
	string(REPLACE "_" "" _plugin_name ${_item})
	if(NOT EXISTS "${_plugin_src_dir}")
		message(TRACE "[!] Skipping mongo plugin ${_plugin_name}: source directory not found")
		continue()
	else()
		message(TRACE "[i] Processing mongo plugin: ${_plugin_name}")
	endif()

	set(_call_args ${_plugin_name})

	# Get subdirs of source directory. If there are then build plugin with dependencies
	glob_subdirs(_plugin_src_subdirs ${_plugin_src_dir})
	if(_plugin_src_subdirs)
		message(TRACE "_plugin_src_subdirs: ${_plugin_src_subdirs}")
		list(APPEND _call_args WITH_DEPS)
	endif()

	if(${_plugin_name} STREQUAL "lockshared")
		set(_with_header_only WITH_HEADER_ONLY)
	else()
		set(_with_header_only)
	endif()

	add_mongo_plugin(${_call_args} ${_with_header_only} SOURCES ${_plugin_src_dir} ${_plugin_src_subdirs})

	if(ENABLE_TESTS AND EXISTS "${_plugin_tests_dir}")
		#TODO
	endif()

endforeach()

#message(FATAL_ERROR "Stop")
