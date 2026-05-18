set(TARGET_NAME tests.catapult.mongo.plugins.multisig)

catapult_mongo_plugin_tests_with_deps(${TARGET_NAME} mappers)

add_subdirs()
