
# All extensions follow roughly the same build pattern, so we can just loop through all subdirs and 
# define them in a consistent way and in a single place. Little differences like extra link libs
# can be handled inline with generator expressions.
# When differences are more significant (e.g. the mongo extension) we can just skip the loop 
# and add the subdir directly.

glob_subdirs(_all_extensions RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
foreach(_extension_name IN LISTS _all_extensions)

	message(TRACE "Defining extension ${_extension_name}")	
	cmake_path(APPEND _extension_path "${CMAKE_CURRENT_SOURCE_DIR}" "${_extension_name}")

	if(_extension_name STREQUAL "mongo")
		add_subdirs("${_extension_path}")
		continue()
	endif()

	# Step 1: define the core library target for the extension (src dir)
	cmake_path(APPEND _sources_path "${_extension_path}" "src")
	glob_sources(_sources RECURSE "${_sources_path}")
	add_target(
		catapult.${_extension_name} LIBRARY
		LINK_LIBS
			catapult.extensions
			catapult.handlers
			catapult.net
 			
			$<$<STREQUAL:${_extension_name},finalization>:catapult.consumers catapult.crypto_voting>
			$<$<STREQUAL:${_extension_name},hashcache>:catapult.plugins.hashcache.cache>
			$<$<STREQUAL:${_extension_name},partialtransaction>:catapult.consumers catapult.plugins.aggregate.sdk>
			$<$<STREQUAL:${_extension_name},sync>:catapult.consumers>
			$<$<STREQUAL:${_extension_name},zeromq>:catapult.model catapult.thread cppzmq>

		FOLDER "extensions"
		SOURCES ${_sources}
	)

	# Step 2: build the extension's library tests if applicable (tests dir)
	cmake_path(APPEND _tests_path "${_extension_path}" "tests")
	if(ENABLE_TESTS AND EXISTS "${_tests_path}" AND IS_DIRECTORY "${_tests_path}")
		glob_sources(_sources RECURSE "${_tests_path}")
		add_target(
			tests.catapult.${_extension_name} TEST
			LINK_LIBS 
				tests.catapult.test.local
				$<$<STREQUAL:${_extension_name},zeromq>:cppzmq>
			FOLDER "extensions"
			SOURCES ${_sources}
		)
	endif()

	# Step 3: define the pluggable extension target itself
	add_target(
		extension.${_extension_name} EXTENSION SHARED
		LINK_LIBS 
			catapult.${_extension_name}
			$<$<STREQUAL:${_extension_name},zeromq>:cppzmq catapult.extensions>
		DEPENDENTS extensions
		FOLDER "extensions"
		SOURCES "${_extension_path}"
	)

endforeach()
