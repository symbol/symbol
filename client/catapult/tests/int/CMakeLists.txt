cmake_minimum_required(VERSION 3.2)

# used to define a catapult int test executable by combining catapult_test_executable and catapult_target
function(catapult_int_test_executable_target TARGET_NAME)
	catapult_test_executable(${TARGET_NAME} ${ARGN})
	catapult_target(${TARGET_NAME})

	set_property(TEST ${TARGET_NAME} PROPERTY LABELS Integration)
endfunction()

add_subdirectory(api)
add_subdirectory(p2p)
add_subdirectory(stress)
