cmake_minimum_required(VERSION 3.23)

set(PLUGIN_CATAPULT_LIBS catapult.mongo)

# a plugin needs a deps lib when it has a cache storage (living under a "storages" subdir);
# the cache storage needs access to the plugin's core cache, hence the extra .deps link target
function(catapult_mongo_plugin_src PLUGIN_BASE_NAME)
	if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/storages")
		# create a deps lib
		catapult_library_target(${PLUGIN_BASE_NAME}.deps "${ARGN}")
		catapult_add_mongo_dependencies(${PLUGIN_BASE_NAME}.deps)
		target_link_libraries(${PLUGIN_BASE_NAME}.deps ${PLUGIN_CATAPULT_LIBS})

		# create a plugin dll
		catapult_shared_library_target(${PLUGIN_BASE_NAME})
		target_link_libraries(${PLUGIN_BASE_NAME} ${PLUGIN_BASE_NAME}.deps)
	else()
		catapult_shared_library_target(${PLUGIN_BASE_NAME} "${ARGN}")
		catapult_add_mongo_dependencies(${PLUGIN_BASE_NAME})
		target_link_libraries(${PLUGIN_BASE_NAME} ${PLUGIN_CATAPULT_LIBS})
	endif()

	add_dependencies(mongo ${PLUGIN_BASE_NAME})
endfunction()

function(catapult_mongo_plugin_tests TARGET_NAME)
	string(REGEX MATCH "\.[a-z_]+$" PLUGIN_NAME ${TARGET_NAME})
	string(SUBSTRING ${PLUGIN_NAME} 1 -1 PLUGIN_NAME)

	catapult_test_executable_target(${TARGET_NAME} mongo "${ARGN}")
	catapult_add_mongo_dependencies(${TARGET_NAME})

	if(TARGET catapult.mongo.plugins.${PLUGIN_NAME}.deps)
		target_link_libraries(${TARGET_NAME} catapult.mongo.plugins.${PLUGIN_NAME}.deps)
		target_link_libraries(${TARGET_NAME} tests.catapult.test.mongo.plugins.${PLUGIN_NAME})

		if(TARGET "tests.catapult.test.plugins.${PLUGIN_NAME}")
			target_link_libraries(${TARGET_NAME} tests.catapult.test.plugins.${PLUGIN_NAME})
		endif()
	endif()

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" PARENT_SCOPE)
endfunction()

function(catapult_mongo_plugin_test_library TARGET_NAME)
	catapult_library_target(${TARGET_NAME})
	catapult_add_mongo_dependencies(${TARGET_NAME})
	target_link_libraries(${TARGET_NAME} ${GTest_LIBRARIES})
endfunction()

add_subdirectory(account_link)
add_subdirectory(aggregate)
add_subdirectory(lock_hash)
add_subdirectory(lock_secret)
add_subdirectory(lock_shared)
add_subdirectory(metadata)
add_subdirectory(mosaic)
add_subdirectory(multisig)
add_subdirectory(namespace)
add_subdirectory(restriction_account)
add_subdirectory(restriction_mosaic)
add_subdirectory(transfer)
