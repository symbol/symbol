cmake_minimum_required(VERSION 3.23)

catapult_library_target(catapult.plugins)
target_link_libraries(catapult.plugins catapult.validators ${CMAKE_DL_LIBS})
