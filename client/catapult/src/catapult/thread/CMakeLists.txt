cmake_minimum_required(VERSION 3.14)

catapult_library_target(catapult.thread)
target_link_libraries(catapult.thread catapult.model catapult.utils)
