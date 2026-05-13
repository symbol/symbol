set(TARGET_NAME tests.catapult.mongo.plugins.lockhash)

include_directories(${PROJECT_SOURCE_DIR}/plugins/txes/lock_hash)
catapult_mongo_plugin_tests_no_deps(${TARGET_NAME} mappers)

add_subdirs()
