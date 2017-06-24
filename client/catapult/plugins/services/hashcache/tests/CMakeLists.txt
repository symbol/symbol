cmake_minimum_required(VERSION 3.2)

find_package(GTest REQUIRED)
include_directories(SYSTEM ${GTEST_INCLUDE_DIR})

catapult_test_executable_target(tests.catapult.plugins.hashcache cache cache handlers observers plugins state validators test)
