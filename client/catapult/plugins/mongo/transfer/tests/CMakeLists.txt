cmake_minimum_required(VERSION 3.2)

find_package(GTest REQUIRED)
include_directories(SYSTEM ${GTEST_INCLUDE_DIR})

set(TARGET_NAME tests.catapult.plugins.mongo.transfer)

catapult_test_executable_target(${TARGET_NAME} mongo)
catapult_add_mongo_dependencies(${TARGET_NAME})
