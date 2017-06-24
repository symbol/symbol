cmake_minimum_required(VERSION 3.2)

catapult_executable(catapult.server)
target_link_libraries(catapult.server catapult.servermain catapult.local.p2p)
catapult_target(catapult.server)
