cmake_minimum_required(VERSION 3.14)

catapult_library_target(catapult.plugins)
target_link_libraries(catapult.plugins catapult.validators ${CMAKE_DL_LIBS})
