cmake_minimum_required(VERSION 3.23)

set(TARGET_NAME tests.catapult.mongo.plugins.locksecret)

include_directories(${PROJECT_SOURCE_DIR}/plugins/txes/lock_secret)
catapult_mongo_plugin_tests_no_deps(${TARGET_NAME} mappers)

add_subdirectory(int)
add_subdirectory(test)
