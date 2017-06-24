cmake_minimum_required(VERSION 3.2)

find_package(GTest REQUIRED)
include_directories(SYSTEM ${GTEST_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/plugins/txes/multisig)

set(TARGET_NAME tests.catapult.plugins.mongo.multisig)

add_subdirectory(int)
add_subdirectory(test)

catapult_test_executable_target(${TARGET_NAME} mongo mappers storages)
catapult_add_mongo_dependencies(${TARGET_NAME})
target_link_libraries(${TARGET_NAME}
	catapult.plugins.mongo.multisig.deps
	tests.catapult.plugins.test.mongo.multisig)
