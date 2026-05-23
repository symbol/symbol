function(catapult_mongo_plugin_src_no_deps PLUGIN_BASE_NAME)
	add_target(
		${PLUGIN_BASE_NAME} LIBRARY TYPE SHARED
		LINK_LIBS 
			${MongoDB_IMPORTED_TARGETS} 
			catapult.mongo
		DEPENDENTS mongo
		SOURCES 
			./
			${ARGN}
	)
endfunction()

function(catapult_mongo_plugin_src_with_deps PLUGIN_BASE_NAME)
	# create a deps lib
	add_target(
		${PLUGIN_BASE_NAME}.deps LIBRARY
		LINK_LIBS 
			${MongoDB_IMPORTED_TARGETS} 
			catapult.mongo
		SOURCES 
			./
			${ARGN}
	)

	# create a plugin dll
	add_target(
		${PLUGIN_BASE_NAME} LIBRARY TYPE SHARED
		DEPENDENTS mongo
		LINK_LIBS 
			${MongoDB_IMPORTED_TARGETS} 
			catapult.mongo
			${PLUGIN_BASE_NAME}.deps
		SOURCES ./
	)

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
	target_link_libraries(${TARGET_NAME} ${MongoDB_IMPORTED_TARGETS} ${GTest_IMPORTED_TARGETS})
endfunction()

add_subdirs()
