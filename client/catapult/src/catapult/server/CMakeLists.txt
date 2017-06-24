cmake_minimum_required(VERSION 3.2)

catapult_library_target(catapult.servermain)
target_link_libraries(catapult.servermain catapult.local)

add_subdirectory(api)
add_subdirectory(p2p)
