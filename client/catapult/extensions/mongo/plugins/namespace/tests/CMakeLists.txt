cmake_minimum_required(VERSION 3.2)

include_directories(${PROJECT_SOURCE_DIR}/plugins/txes/namespace)

set(TARGET_NAME tests.catapult.mongo.plugins.namespace)

add_subdirectory(int)
add_subdirectory(test)

catapult_test_executable_target(${TARGET_NAME} mongo mappers storages)
catapult_add_mongo_dependencies(${TARGET_NAME})
target_link_libraries(${TARGET_NAME}
	catapult.mongo.plugins.namespace.deps
	tests.catapult.test.mongo.plugins.namespace
	tests.catapult.test.plugins.namespace)
