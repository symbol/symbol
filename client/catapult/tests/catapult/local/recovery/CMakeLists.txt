cmake_minimum_required(VERSION 3.14)

catapult_test_executable_target(tests.catapult.local.recovery local)
target_link_libraries(tests.catapult.local.recovery tests.catapult.test.nemesis)
