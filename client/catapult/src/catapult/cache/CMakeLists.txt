cmake_minimum_required(VERSION 3.2)

catapult_library_target(catapult.cache)
target_link_libraries(catapult.cache catapult.model catapult.io)
