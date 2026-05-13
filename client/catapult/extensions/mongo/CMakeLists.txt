# used to define a catapult mongo int test executable by combining catapult_test_executable and catapult_target
function(catapult_int_test_executable_target TARGET_NAME)
	
	add_target(${TARGET_NAME} TEST LINK SOURCES ${ARGN})
	set_property(TEST ${TARGET_NAME} PROPERTY LABELS Database)

endfunction()

add_subdirs(plugins)

catapult_define_extension_no_group(mongo)
target_link_libraries(extension.mongo ${MongoDB_IMPORTED_TARGETS})

# add extension to mongo group
add_dependencies(mongo extension.mongo)
