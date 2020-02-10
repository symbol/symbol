cmake_minimum_required(VERSION 3.14)

set(TARGET_NAME tests.catapult.sdk)

catapult_test_executable_target(${TARGET_NAME} core builders extensions parsers)
