cmake_minimum_required(VERSION 3.2)

catapult_library_target(catapult.sdk builders extensions)
target_link_libraries(catapult.sdk catapult.model)
target_link_libraries(catapult.sdk catapult.plugins.namespace.sdk) # for IdGenerator

find_program(PYTHON_EXECUTABLE python3 python)

add_custom_command(
	OUTPUT ${CMAKE_BINARY_DIR}/catapult/catapult.h
	DEPENDS ${PROJECT_SOURCE_DIR}/scripts/sdk/publishSdk.py
	COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/scripts/sdk/publishSdk.py ARGS --r ${PROJECT_SOURCE_DIR} --p ${CMAKE_BINARY_DIR}/inc)

add_custom_target(
	publish
	DEPENDS ${CMAKE_BINARY_DIR}/catapult/catapult.h)
