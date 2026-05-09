# used to define a catapult bench executable
function(catapult_bench_executable_target TARGET_NAME)
	catapult_executable(${TARGET_NAME} ${ARGN})
	target_link_libraries(${TARGET_NAME} build.tests)
	catapult_target(${TARGET_NAME})

	add_dependencies(tools ${TARGET_NAME})
	install(TARGETS ${TARGET_NAME})
endfunction()

subdirectories(ADD)
