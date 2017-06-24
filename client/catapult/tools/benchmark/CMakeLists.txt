cmake_minimum_required(VERSION 3.2)

catapult_executable(catapult.tools.benchmark)
target_link_libraries(catapult.tools.benchmark catapult.tools)
catapult_target(catapult.tools.benchmark)
