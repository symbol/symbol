cmake_minimum_required(VERSION 3.2)

catapult_library_target(catapult.local.api)
target_link_libraries(catapult.local.api catapult.local)
