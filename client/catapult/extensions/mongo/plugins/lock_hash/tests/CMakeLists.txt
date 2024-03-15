cmake_minimum_required(VERSION 3.23)

set(TARGET_NAME tests.catapult.mongo.plugins.lockhash)

include_directories(${PROJECT_SOURCE_DIR}/plugins/txes/lock_hash)
catapult_mongo_plugin_tests_no_deps(${TARGET_NAME} mappers)

add_subdirectory(int)
add_subdirectory(test)
