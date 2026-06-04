function(catapult_tx_plugin_src PLUGIN_BASE_NAME)
	# create an sdk lib
	if(DEFINED PLUGIN_SDK_FOLDERS)
		add_target(${PLUGIN_BASE_NAME}.sdk LIBRARY
			LINK_LIBS catapult.model
			SOURCES ./ ${PLUGIN_SDK_FOLDERS}
		)
	endif()

	# create a deps lib
	if(DEFINED PLUGIN_DEPS_FOLDERS)
		set(_deps_link_libs ${PLUGIN_CATAPULT_LIBS})
		if(DEFINED PLUGIN_SDK_FOLDERS)
			list(APPEND _deps_link_libs ${PLUGIN_BASE_NAME}.sdk)
		endif()
		add_target(${PLUGIN_BASE_NAME}.deps LIBRARY
			LINK_LIBS ${_deps_link_libs}
			SOURCES ./ ${PLUGIN_DEPS_FOLDERS}
		)
	endif()

	# create a plugin dll
	set(_plugin_link_libs)
	if(DEFINED PLUGIN_DEPS_FOLDERS)
		list(APPEND _plugin_link_libs ${PLUGIN_BASE_NAME}.deps)
	endif()
	add_target(${PLUGIN_BASE_NAME} LIBRARY TYPE SHARED
		LINK_LIBS ${_plugin_link_libs}
		DEPENDENTS plugins
		SOURCES ./ plugins
	)
endfunction()

function(catapult_tx_plugin_tests TARGET_NAME)
	string(REGEX MATCH "\.[a-z_]+$" PLUGIN_NAME ${TARGET_NAME})
	string(SUBSTRING ${PLUGIN_NAME} 1 -1 PLUGIN_NAME)

	if(NOT TARGET "tests.catapult.test.plugins.${PLUGIN_NAME}")
		set(PLUGIN_TEST_FOLDERS test ${PLUGIN_TEST_FOLDERS})
	endif()

	set(_source_dirs ./)
	if(DEFINED PLUGIN_SDK_FOLDERS)
		foreach(_relative_source IN LISTS PLUGIN_SDK_FOLDERS)
			list(APPEND _source_dirs "./${_relative_source}")
		endforeach()
	endif()
	if(DEFINED PLUGIN_DEPS_FOLDERS)
		foreach(_relative_source IN LISTS PLUGIN_DEPS_FOLDERS)
			list(APPEND _source_dirs "./${_relative_source}")
		endforeach()
	endif()
	if(DEFINED PLUGIN_TEST_FOLDERS)
		foreach(_relative_source IN LISTS PLUGIN_TEST_FOLDERS)
			list(APPEND _source_dirs "./${_relative_source}")
		endforeach()
	endif()
	list(APPEND _source_dirs ./plugins)

	add_target(${TARGET_NAME} TEST
		LINK_LIBS tests.catapult.test.cache
		SOURCES ${_source_dirs}
	)

	if(TARGET "tests.catapult.test.plugins.${PLUGIN_NAME}")
		target_link_libraries(${TARGET_NAME} tests.catapult.test.plugins.${PLUGIN_NAME})
	endif()

endfunction()

function(catapult_plugin_test_library TARGET_NAME)
	add_target(${TARGET_NAME} LIBRARY
		LINK_LIBS ${GTest_IMPORTED_TARGETS}
		SOURCES ./
	)
endfunction()

add_subdirs()
