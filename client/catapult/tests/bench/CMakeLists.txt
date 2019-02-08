cmake_minimum_required(VERSION 3.2)

# used to define a catapult bench executable
function(catapult_bench_executable_target TARGET_NAME)
	find_package(benchmark REQUIRED)

	catapult_executable(${TARGET_NAME} ${ARGN})

	catapult_set_test_compiler_options(${TARGET_NAME})

	# this should both link and add proper include path
	target_link_libraries(${TARGET_NAME} benchmark::benchmark)
	catapult_target(${TARGET_NAME})
endfunction()

add_subdirectory(crypto)
