cmake_minimum_required(VERSION 3.2)

catapult_test_executable_target(tests.catapult.local local)

add_subdirectory(api)
add_subdirectory(p2p)
