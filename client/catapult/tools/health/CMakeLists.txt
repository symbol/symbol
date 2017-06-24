cmake_minimum_required(VERSION 3.2)

catapult_executable(catapult.tools.health)
target_link_libraries(catapult.tools.health catapult.tools catapult.local catapult.sdk)
catapult_target(catapult.tools.health)
