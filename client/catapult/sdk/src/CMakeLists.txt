cmake_minimum_required(VERSION 3.23)

set(TARGET_NAME catapult.sdk)

catapult_library_target(${TARGET_NAME} builders extensions parsers)
target_link_libraries(${TARGET_NAME}
	catapult.model
	catapult.state
	catapult.plugins.aggregate.sdk
	catapult.plugins.metadata.sdk
	catapult.plugins.mosaic.sdk
	catapult.plugins.namespace.sdk
	catapult.plugins.restrictionaccount.sdk)

find_program(PYTHON_EXECUTABLE python3 python REQUIRED)

add_custom_target(
	publish ALL
	COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/scripts/sdk/publishSdk.py -r ${PROJECT_SOURCE_DIR} -p ${CMAKE_BINARY_DIR}/inc
	BYPRODUCTS ${CMAKE_BINARY_DIR}/inc/catapult/catapult.h
)
