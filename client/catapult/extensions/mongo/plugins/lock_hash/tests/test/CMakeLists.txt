cmake_minimum_required(VERSION 3.14)

set(TARGET_NAME tests.catapult.test.mongo.plugins.lockhash)

catapult_add_gtest_dependencies()
catapult_library_target(${TARGET_NAME})
target_link_libraries(${TARGET_NAME}
	catapult.plugins.lockhash.deps
	catapult.mongo.plugins.lockhash.deps
	tests.catapult.test.plugins.lockhash)
catapult_add_mongo_dependencies(${TARGET_NAME})
