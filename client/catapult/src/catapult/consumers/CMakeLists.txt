cmake_minimum_required(VERSION 3.14)

catapult_library_target(catapult.consumers)
target_link_libraries(catapult.consumers catapult.chain)
