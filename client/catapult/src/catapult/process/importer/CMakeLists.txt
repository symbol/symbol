cmake_minimum_required(VERSION 3.23)

set(TARGET_NAME catapult.importer)

catapult_executable(${TARGET_NAME})
target_link_libraries(${TARGET_NAME} catapult.local.importer catapult.process)
catapult_target(${TARGET_NAME})

add_dependencies(server ${TARGET_NAME})
