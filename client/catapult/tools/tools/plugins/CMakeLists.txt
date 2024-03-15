cmake_minimum_required(VERSION 3.23)

set(TARGET_NAME catapult.tools.plugins)

catapult_library_target(${TARGET_NAME})
target_link_libraries(${TARGET_NAME} catapult.plugins)

# tool has plugins dependency so it must be able to access src
include_directories(${PROJECT_SOURCE_DIR}/src)
