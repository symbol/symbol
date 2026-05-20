set(TARGET_NAME tests.catapult.mongo.plugins.mosaic)

catapult_mongo_plugin_tests_with_deps(${TARGET_NAME} mappers storages)

add_subdirectory(int)
add_subdirectory(test)
