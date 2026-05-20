
function(catapult_tx_plugin_src PLUGIN_BASE_NAME)
	# create an sdk lib
	if(DEFINED PLUGIN_SDK_FOLDERS)
		catapult_library_target(${PLUGIN_BASE_NAME}.sdk ${PLUGIN_SDK_FOLDERS})
		target_link_libraries(${PLUGIN_BASE_NAME}.sdk catapult.model)
	endif()

	# create a deps lib
	if(DEFINED PLUGIN_DEPS_FOLDERS)
		catapult_library_target(${PLUGIN_BASE_NAME}.deps ${PLUGIN_DEPS_FOLDERS})
		target_link_libraries(${PLUGIN_BASE_NAME}.deps ${PLUGIN_CATAPULT_LIBS})
	endif()

	if(DEFINED PLUGIN_SDK_FOLDERS)
		target_link_libraries(${PLUGIN_BASE_NAME}.deps ${PLUGIN_BASE_NAME}.sdk)
	endif()

	# create a plugin dll
	catapult_shared_library_target(${PLUGIN_BASE_NAME} plugins)

	if(DEFINED PLUGIN_DEPS_FOLDERS)
		target_link_libraries(${PLUGIN_BASE_NAME} ${PLUGIN_BASE_NAME}.deps)
	endif()

	add_dependencies(plugins ${PLUGIN_BASE_NAME})
endfunction()

function(catapult_tx_plugin_tests TARGET_NAME)
	string(REGEX MATCH "\.[a-z_]+$" PLUGIN_NAME ${TARGET_NAME})
	string(SUBSTRING ${PLUGIN_NAME} 1 -1 PLUGIN_NAME)

	if(NOT TARGET "tests.catapult.test.plugins.${PLUGIN_NAME}")
		set(PLUGIN_TEST_FOLDERS test ${PLUGIN_TEST_FOLDERS})
	endif()

	catapult_test_executable_target(${TARGET_NAME} cache ${PLUGIN_SDK_FOLDERS} ${PLUGIN_DEPS_FOLDERS} ${PLUGIN_TEST_FOLDERS} plugins)

	if(TARGET "tests.catapult.test.plugins.${PLUGIN_NAME}")
		target_link_libraries(${TARGET_NAME} tests.catapult.test.plugins.${PLUGIN_NAME})
	endif()

endfunction()

function(catapult_plugin_test_library TARGET_NAME)
	catapult_library_target(${TARGET_NAME})
	target_link_libraries(${TARGET_NAME} ${GTest_IMPORTED_TARGETS})
endfunction()

add_subdirs()
