cmake_minimum_required(VERSION 3.2)

set(TARGET_NAME tests.catapult.int.plugins.mongo.coremongo)

catapult_int_test_executable_target(${TARGET_NAME})
catapult_add_mongo_dependencies(${TARGET_NAME})
target_link_libraries(${TARGET_NAME} tests.catapult.test.cache tests.catapult.test.mongo catapult.plugins.mongo.coremongo)
