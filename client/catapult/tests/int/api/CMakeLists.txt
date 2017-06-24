cmake_minimum_required(VERSION 3.2)

set(TARGET_NAME tests.catapult.int.api)

catapult_int_test_executable_target(${TARGET_NAME})
target_link_libraries(${TARGET_NAME} tests.catapult.test.int catapult.local.api)
