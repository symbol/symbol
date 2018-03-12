cmake_minimum_required(VERSION 3.2)

set(TARGET_NAME catapult.server)

catapult_executable(${TARGET_NAME})
target_link_libraries(${TARGET_NAME} catapult.local)
catapult_target(${TARGET_NAME})
