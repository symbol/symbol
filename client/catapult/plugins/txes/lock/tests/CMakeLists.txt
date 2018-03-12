cmake_minimum_required(VERSION 3.2)

set(TARGET_NAME tests.catapult.plugins.lock)

add_subdirectory(test)

catapult_test_executable_target(${TARGET_NAME} cache cache config model observers plugins utils validators)
target_link_libraries(${TARGET_NAME} tests.catapult.test.plugins.lock)
