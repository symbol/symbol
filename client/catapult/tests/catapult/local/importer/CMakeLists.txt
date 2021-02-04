cmake_minimum_required(VERSION 3.14)

catapult_test_executable_target(tests.catapult.local.importer local)
target_link_libraries(tests.catapult.local.importer tests.catapult.test.nemesis)
