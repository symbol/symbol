cmake_minimum_required(VERSION 3.2)

catapult_library_target(catapult.thread)
target_link_libraries(catapult.thread catapult.ionet catapult.model catapult.utils)
