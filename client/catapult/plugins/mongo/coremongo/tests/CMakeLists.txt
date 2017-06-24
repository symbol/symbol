cmake_minimum_required(VERSION 3.2)

find_package(GTest REQUIRED)
include_directories(SYSTEM ${GTEST_INCLUDE_DIR})

set(TARGET_NAME tests.catapult.plugins.mongo.coremongo)

add_subdirectory(int)

catapult_test_executable_target(${TARGET_NAME} mongo mappers)
catapult_add_mongo_dependencies(${TARGET_NAME})
target_link_libraries(${TARGET_NAME} tests.catapult.test.cache)
target_link_libraries(${TARGET_NAME} catapult.plugins.mongo.transfer) # allow transfer to be loaded implicitly
