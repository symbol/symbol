cmake_minimum_required(VERSION 3.23)

catapult_library_target(tests.catapult.test.nodeps)
target_link_libraries(tests.catapult.test.nodeps catapult.crypto catapult.utils catapult.version ${GTest_LIBRARIES} ${CMAKE_DL_LIBS})
