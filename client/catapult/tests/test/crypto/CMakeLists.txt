cmake_minimum_required(VERSION 3.2)

include_directories(../../../external)

catapult_library_target(tests.catapult.test.crypto)
target_link_libraries(tests.catapult.test.crypto tests.catapult.test.nodeps)
