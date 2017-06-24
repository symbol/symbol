cmake_minimum_required(VERSION 3.2)

catapult_executable(catapult.tools.nemgen)
target_link_libraries(catapult.tools.nemgen catapult.tools catapult.plugins.namespace catapult.sdk)
catapult_target(catapult.tools.nemgen)
