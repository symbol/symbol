cmake_minimum_required(VERSION 3.2)

set(TARGET_NAME tests.catapult.test.mongo.plugins.locksecret)

catapult_add_gtest_dependencies()
catapult_library_target(${TARGET_NAME} tests.catapult.test.plugins.locksecret)
catapult_add_mongo_dependencies(${TARGET_NAME})
