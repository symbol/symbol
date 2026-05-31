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
	set(_link_libs ${MongoDB_IMPORTED_TARGETS} catapult.mongo)
	# Extra link lib from catapult
	if(TARGET "catapult.plugins.${_name}")
		list(APPEND _link_libs catapult.plugins.${_name})
	endif()
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

	# Lookup for plugin with deps
	list(FIND _mongo_plugins_with_deps ${_name} _index)
	if(_index EQUAL -1)
		# No deps here
		add_target(
			${_base_name} 
			LIBRARY TYPE SHARED
			LINK_LIBS ${_link_libs}
			DEPENDENTS mongo
			SOURCES ${_all_source_files}
		)
		return()
	endif()

	# Create deps lib
	add_target(
		${_base_name}.deps LIBRARY
		LINK_LIBS ${_link_libs}
		SOURCES ${_all_source_files}
	)

	# Create main plugin library with only direct sources
	# i.e. files in src/ but not in subdirs
	glob_sources(_all_source_files ${_arg_BASE_PATH})
	add_target(
		${_base_name} LIBRARY TYPE SHARED
		LINK_LIBS ${_base_name}.deps
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

	# Tests might have /int/ and /test/ subdirs which need to be processed
	# separately. As a result we need to keep in _all_source_files only
	# the files NOT matching those subdirs.
	list(FILTER _all_source_files EXCLUDE REGEX "[/\\](int|test)[/\\]")

	# Lookup for plugin with deps
	list(FIND _mongo_plugins_tests_with_deps ${_name} _index)
	if(_index EQUAL -1)
		# No deps here
		# TODO this causes 'mappers' and eventually 'storages' to be excluded 
		# entirely from compilation which is not wanted.
		# See issue #1992 for details
		list(FILTER _all_source_files EXCLUDE REGEX "[/\\](mappers)[/\\]")
		add_target(
			tests.catapult.mongo.plugins.${_name} TEST
			LINK_LIBS ${_link_libs}
			SOURCES ${_all_source_files}
		)
	else()
		# With deps
		list(APPEND _link_libs 
			catapult.mongo.plugins.${_name}.deps
			tests.catapult.test.mongo.plugins.${_name}
		)
		if(TARGET "tests.catapult.test.plugins.${_name}")
			list(APPEND _link_libs tests.catapult.test.plugins.${_name})
			message(FATA_ERROR "Stop Debug")
		endif()
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

# The logic driving the 'with_deps' thingy is quite obscure and I couldn't find a better way
# than to just have two separate lists.
set(_mongo_plugins_with_deps
	lockhash
	locksecret
	metadata
	mosaic
	multisig
	namespace
	restrictionaccount
	restrictionmosaic
)

set(_mongo_plugins_tests_with_deps
	metadata
	mosaic
	multisig
	namespace
	restrictionaccount
	restrictionmosaic
)

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
