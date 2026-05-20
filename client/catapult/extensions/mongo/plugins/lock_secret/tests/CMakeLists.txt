set(TARGET_NAME tests.catapult.mongo.plugins.locksecret)

catapult_mongo_plugin_tests_no_deps(${TARGET_NAME} mappers)

add_subdirectory(int)
add_subdirectory(test)
