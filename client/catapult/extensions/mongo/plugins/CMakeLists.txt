set(PLUGIN_CATAPULT_LIBS catapult.mongo)

function(catapult_mongo_plugin_src_no_deps PLUGIN_BASE_NAME)
	catapult_shared_library_target(${PLUGIN_BASE_NAME} "${ARGN}")
	target_link_libraries(${PLUGIN_BASE_NAME} ${MongoDB_IMPORTED_TARGETS} ${PLUGIN_CATAPULT_LIBS})
	add_dependencies(mongo ${PLUGIN_BASE_NAME})
endfunction()

function(catapult_mongo_plugin_src_with_deps PLUGIN_BASE_NAME)
	# create a deps lib
	catapult_library_target(${PLUGIN_BASE_NAME}.deps "${ARGN}")
	target_link_libraries(${PLUGIN_BASE_NAME}.deps ${MongoDB_IMPORTED_TARGETS} ${PLUGIN_CATAPULT_LIBS})

	# create a plugin dll
	catapult_shared_library_target(${PLUGIN_BASE_NAME})
	target_link_libraries(${PLUGIN_BASE_NAME} ${PLUGIN_BASE_NAME}.deps)

	add_dependencies(mongo ${PLUGIN_BASE_NAME})
endfunction()

function(catapult_mongo_plugin_tests_no_deps TARGET_NAME)
	catapult_test_executable_target(${TARGET_NAME} mongo)
	target_link_libraries(${TARGET_NAME} ${MongoDB_IMPORTED_TARGETS})
endfunction()

function(catapult_mongo_plugin_tests_with_header_only_deps TARGET_NAME)
	string(REGEX MATCH "\.[a-z_]+$" PLUGIN_NAME ${TARGET_NAME})
	string(SUBSTRING ${PLUGIN_NAME} 1 -1 PLUGIN_NAME)

	catapult_test_executable_target(${TARGET_NAME} mongo "${ARGN}")
	target_link_libraries(${TARGET_NAME} ${MongoDB_IMPORTED_TARGETS} catapult.mongo.plugins.${PLUGIN_NAME}.deps)

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
	catapult_library_target(${TARGET_NAME})
	target_link_libraries(${TARGET_NAME} ${MongoDB_IMPORTED_TARGETS} ${GTest_IMPORTED_TARGETS})
endfunction()

subdirectories(ADD)
