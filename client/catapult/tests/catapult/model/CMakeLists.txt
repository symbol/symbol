cmake_minimum_required(VERSION 3.14)

catapult_test_executable_target(tests.catapult.model core)
target_link_libraries(tests.catapult.model external)
