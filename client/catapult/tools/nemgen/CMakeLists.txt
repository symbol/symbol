cmake_minimum_required(VERSION 3.2)

add_subdirectory(blockhashes)

set(TARGET_NAME catapult.tools.nemgen)

catapult_executable(${TARGET_NAME})
target_link_libraries(${TARGET_NAME}
	${TARGET_NAME}.blockhashes
	catapult.tools
	catapult.plugins.mosaic
	catapult.plugins.namespace
	catapult.plugins.transfer)
catapult_target(${TARGET_NAME})
