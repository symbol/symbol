function(add_mongo_plugin TNAME)
	
	set(_fn_argc ${ARGC})
	set(_fn_options)
	set(_fn_single BASE_PATH)
	set(_fn_multi LINK_LIBS)

	_parse_arguments(1)

	if(NOT DEFINED _arg_BASE_PATH)
		message(FATAL_ERROR "${CMAKE_CURRENT_FUNCTION} '${TNAME}': missing required argument: BASE_PATH.")
	endif()

	# Validate plugin name
	if(NOT TNAME MATCHES "^[a-zA-Z0-9]+$")
		message(FATAL_ERROR "${CMAKE_CURRENT_FUNCTION} '${TNAME}': invalid plugin name.")
	endif()
	set(_name ${TNAME})
	set(_base_name "catapult.mongo.plugins.${_name}")

	# Default libraries to link with
	set(_link_libs 
		${MongoDB_IMPORTED_TARGETS} 
		catapult.mongo
		$<TARGET_NAME_IF_EXISTS:catapult.plugins.${_name}>
	)
	# Extra user-defined libraries to link with
	if(_arg_LINK_LIBS)
		list(APPEND _link_libs ${_arg_LINK_LIBS})
	endif()	

	# Grab all sources and check whether is header-only
	glob_sources(_all_source_files RECURSE ${_arg_BASE_PATH})
	set(_source_non_header_files ${_all_source_files})
	list(FILTER _source_non_header_files EXCLUDE REGEX ".*\\.(h|hpp)$")
	list(LENGTH _source_non_header_files _num_non_header_files)
	if(_num_non_header_files EQUAL 0)
		add_target(
			${_base_name} 
			LIBRARY TYPE INTERFACE
			SOURCES ${_all_source_files}
		)
		return()
	endif()

	# A plugin whose src is partitioned into 'mappers' and 'storages' subdirs is a 'deps' plugin:
	# the subdir sources go into a separate static '.deps' lib while the plugin dll is built from
	# the direct src/ files only. The two subdirs always co-occur (a plugin has both or neither).
	cmake_path(SET _mappers_dir NORMALIZE "${_arg_BASE_PATH}/mappers")
	cmake_path(SET _storages_dir NORMALIZE "${_arg_BASE_PATH}/storages")
	if(IS_DIRECTORY "${_mappers_dir}" AND IS_DIRECTORY "${_storages_dir}")
		
		# Create deps lib from all (recursive) sources
		add_target(
			${_base_name}.deps LIBRARY
			LINK_LIBS ${_link_libs}
			SOURCES ${_all_source_files}
		)

		# the plugin dll links the deps lib and is built from only direct sources,
		# i.e. files in src/ but not in subdirs
		set(_link_libs ${_base_name}.deps)
		glob_sources(_all_source_files ${_arg_BASE_PATH})

	endif()

	# Create main plugin library
	add_target(
		${_base_name} LIBRARY TYPE SHARED
		LINK_LIBS ${_link_libs}
		DEPENDENTS mongo
		SOURCES ${_all_source_files}
	)

endfunction()

function(add_mongo_plugin_tests TNAME)

	set(_fn_argc ${ARGC})
	set(_fn_options)
	set(_fn_single BASE_PATH)
	set(_fn_multi LINK_LIBS)

	_parse_arguments(1)

	if(NOT DEFINED _arg_BASE_PATH)
		message(FATAL_ERROR "${CMAKE_CURRENT_FUNCTION} '${TNAME}': missing required argument: BASE_PATH.")
	endif()

	# Validate test plugin name
	if(NOT TNAME MATCHES "^[a-zA-Z0-9]+$")
		message(FATAL_ERROR "${CMAKE_CURRENT_FUNCTION} '${TNAME}': invalid plugin name.")
	endif()
	set(_name ${TNAME})
	set(_base_name "tests.catapult.mongo.plugins.${_name}")

	# Grab all sources and check whether is header-only
	glob_sources(_all_source_files RECURSE ${_arg_BASE_PATH})
	set(_source_non_header_files ${_all_source_files})
	list(FILTER _source_non_header_files EXCLUDE REGEX ".*\\.(h|hpp)$")
	list(LENGTH _source_non_header_files _num_non_header_files)
	if(_num_non_header_files EQUAL 0)
		add_target(
			${_base_name} 
			LIBRARY TYPE INTERFACE
			SOURCES ${_all_source_files}
		)
		return()
	endif()

	# Default libraries to link with
	set(_link_libs ${MongoDB_IMPORTED_TARGETS} tests.catapult.test.mongo)

	# Extra user-defined libraries to link with
	if(_arg_LINK_LIBS)
		list(APPEND _link_libs ${_arg_LINK_LIBS})
	endif()	

	# Tests might have /int/ and /test/ subdirs which need to be processed
	# separately. As a result we need to keep in _all_source_files only
	# the files NOT matching those subdirs.
	list(FILTER _all_source_files EXCLUDE REGEX "[/\\](int|test)[/\\]")

	# A 'deps' plugin (one with a '.deps' lib, see add_mongo_plugin) requires the extra plugin
	# and test-support libraries to be linked; a plain plugin only needs the mongo test lib.
	if(NOT TARGET catapult.mongo.plugins.${_name}.deps)
		add_target(
			tests.catapult.mongo.plugins.${_name} TEST
			LINK_LIBS ${_link_libs}
			SOURCES ${_all_source_files}
		)
	else()
		list(APPEND _link_libs 
			catapult.mongo.plugins.${_name}.deps
			tests.catapult.test.mongo.plugins.${_name}
			$<TARGET_NAME_IF_EXISTS:tests.catapult.test.plugins.${_name}>
		)
		add_target(
			tests.catapult.mongo.plugins.${_name} TEST
			LINK_LIBS ${_link_libs}
			SOURCES ${_all_source_files}
		)
	endif()

	# Process /int/ subdir if exists
	cmake_path(SET _int_dir NORMALIZE "${_arg_BASE_PATH}/int")
	if(EXISTS "${_int_dir}" AND IS_DIRECTORY "${_int_dir}")
		glob_sources(_int_source_files RECURSE ${_int_dir})
		set(_int_link_libs
				tests.catapult.test.mongo.plugins.${_name}
				tests.catapult.test.cache
				tests.catapult.test.mongo
				catapult.mongo.plugins.${_name}
		)

		if(NOT _name STREQUAL "multisig")
			list(APPEND _int_link_libs tests.catapult.test.plugins.${_name})
		elseif(TARGET catapult.plugins.${_name}.deps)
			list(APPEND _int_link_libs catapult.plugins.${_name}.deps)
		endif()
			
		add_target(
			tests.catapult.int.mongo.plugins.${_name} TEST
			WNO_DERIVED_LIB
			LINK_LIBS ${_int_link_libs}
			LABELS Database
			SOURCES ${_int_source_files}
		)
	endif()

	# Process /test/ subdir if exists
	cmake_path(SET _test_dir NORMALIZE "${_arg_BASE_PATH}/test")
	if(EXISTS "${_test_dir}" AND IS_DIRECTORY "${_test_dir}")
		glob_sources(_test_source_files RECURSE ${_test_dir})
		add_target(
			tests.catapult.test.mongo.plugins.${_name} LIBRARY
			LINK_LIBS 
				${MongoDB_IMPORTED_TARGETS}
				${GTest_IMPORTED_TARGETS}
			SOURCES ${_test_source_files}
		)
	endif()

endfunction()

glob_subdirs(_all_plugins RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
foreach(_item IN LISTS _all_plugins)

	string(REPLACE "_" "" _name ${_item})
	cmake_path(SET _src_dir NORMALIZE "${CMAKE_CURRENT_SOURCE_DIR}/${_item}/src")

	if(NOT EXISTS "${_src_dir}")
		message(FATAL_ERROR "Source directory not found for mongo plugin ${_name}: expected at ${_src_dir}")
	endif()

	add_mongo_plugin(${_name} BASE_PATH "${_src_dir}")

	cmake_path(SET _plugin_tests_dir NORMALIZE "${CMAKE_CURRENT_SOURCE_DIR}/${_item}/tests")
	if(ENABLE_TESTS AND EXISTS "${_plugin_tests_dir}")
		add_mongo_plugin_tests(${_name} BASE_PATH ${_plugin_tests_dir})
	endif()

endforeach()
