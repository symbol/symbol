cmake_minimum_required(VERSION 3.2)

add_subdirectory(statehash)

set(TARGET_NAME catapult.tools.nemgen)

catapult_executable(${TARGET_NAME})
target_link_libraries(${TARGET_NAME}
	${TARGET_NAME}.statehash
	catapult.tools
	catapult.plugins.namespace
	catapult.plugins.transfer)
catapult_target(${TARGET_NAME})
