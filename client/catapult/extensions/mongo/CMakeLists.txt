# used to define a catapult mongo int test executable by combining catapult_test_executable and catapult_target
function(catapult_int_test_executable_target TARGET_NAME)
	catapult_test_executable(${TARGET_NAME} ${ARGN})
	catapult_target(${TARGET_NAME})

	set_property(TEST ${TARGET_NAME} PROPERTY LABELS Database)
	target_link_libraries(${TARGET_NAME} build.tests)

endfunction()

subdirectories(ADD BASE . DIRS plugins)

catapult_define_extension_no_group(mongo)
target_link_libraries(${TARGET_NAME} ${MongoDB_IMPORTED_TARGETS})

# add extension to mongo group
add_dependencies(mongo extension.mongo)
