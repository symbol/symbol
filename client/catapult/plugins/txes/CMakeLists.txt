function(catapult_tx_plugin_src TNAME)
	# create an sdk lib
	if(DEFINED PLUGIN_SDK_FOLDERS)
		add_target(${TNAME}.sdk LIBRARY
			LINK_LIBS catapult.model
			SOURCES ./ ${PLUGIN_SDK_FOLDERS}
		)
	endif()

	# create a deps lib
	if(DEFINED PLUGIN_DEPS_FOLDERS)
		add_target(${TNAME}.deps LIBRARY
			LINK_LIBS 
				${PLUGIN_CATAPULT_LIBS}
				$<TARGET_NAME_IF_EXISTS:${TNAME}.sdk>
			SOURCES ./ ${PLUGIN_DEPS_FOLDERS}
		)
	endif()

	# create a plugin dll
	add_target(${TNAME} LIBRARY TYPE SHARED
		LINK_LIBS ${TNAME}.deps
		DEPENDENTS plugins
		SOURCES ./ plugins
	)
endfunction()

function(catapult_tx_plugin_tests TNAME)
	string(REGEX MATCH "\.[a-z_]+$" _name ${TNAME})
	string(SUBSTRING ${_name} 1 -1 _name)

	if(NOT TARGET "tests.catapult.test.plugins.${_name}")
		set(PLUGIN_TEST_FOLDERS test ${PLUGIN_TEST_FOLDERS})
	endif()

	add_target(${TNAME} TEST
		LINK_LIBS 
			tests.catapult.test.cache
			$<TARGET_NAME_IF_EXISTS:tests.catapult.test.plugins.${_name}>
		SOURCES 
			./
			${PLUGIN_SDK_FOLDERS} 
			${PLUGIN_DEPS_FOLDERS} 
			${PLUGIN_TEST_FOLDERS}
			./plugins
	)

endfunction()

function(catapult_plugin_test_library TNAME)
	add_target(${TNAME} LIBRARY
		LINK_LIBS ${GTest_IMPORTED_TARGETS}
		SOURCES ./
	)
endfunction()

derive_plugin_full_name(_plugin_full_name)
add_subdirs()
