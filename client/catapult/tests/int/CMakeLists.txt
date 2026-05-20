
# used to define a catapult int test executable by combining catapult_test_executable and catapult_target
function(catapult_int_test_executable_target TARGET_NAME)
	catapult_test_executable(${TARGET_NAME} ${ARGN})
	catapult_target(${TARGET_NAME})

	set_property(TEST ${TARGET_NAME} PROPERTY LABELS Integration)

	target_link_libraries(${TARGET_NAME} build.tests)
endfunction()

add_subdirectory(node)
add_subdirectory(stress)
