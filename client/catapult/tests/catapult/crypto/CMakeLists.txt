cmake_minimum_required(VERSION 3.2)

include_directories(../../../external)

catapult_test_executable_target(tests.catapult.crypto crypto)
target_link_libraries(tests.catapult.crypto external)
