set(TARGET_NAME tests.catapult.mongo.plugins.namespace)

catapult_mongo_plugin_tests_with_deps(${TARGET_NAME} mappers storages)

add_subdirs()
