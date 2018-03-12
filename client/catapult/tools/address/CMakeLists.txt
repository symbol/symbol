cmake_minimum_required(VERSION 3.2)

set(TARGET_NAME catapult.tools.address)

catapult_executable(${TARGET_NAME})
target_link_libraries(${TARGET_NAME} catapult.tools catapult.sdk)
catapult_target(${TARGET_NAME})
