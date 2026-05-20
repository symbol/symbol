set(TARGET_NAME tests.catapult.mongo.plugins.restrictionmosaic)

catapult_mongo_plugin_tests_with_deps(${TARGET_NAME} mappers)

add_subdirectory(int)
add_subdirectory(test)
