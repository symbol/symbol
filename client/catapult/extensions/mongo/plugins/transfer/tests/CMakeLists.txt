cmake_minimum_required(VERSION 3.2)

set(TARGET_NAME tests.catapult.mongo.plugins.transfer)

catapult_test_executable_target(${TARGET_NAME} mongo)
catapult_add_mongo_dependencies(${TARGET_NAME})
