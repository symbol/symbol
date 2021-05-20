cmake_minimum_required(VERSION 3.14)

# setup benchmark
message("--- locating bench dependencies ---")
find_package(benchmark 1.5.3 EXACT REQUIRED)
message("bench     ver: ${benchmark_VERSION}")

# add benchmark dependencies
function(catapult_add_benchmark_dependencies TARGET_NAME)
	# this should both link and add proper include path
	target_link_libraries(${TARGET_NAME} benchmark::benchmark)
endfunction()

# used to define a catapult bench executable
function(catapult_bench_executable_target TARGET_NAME)
	catapult_executable(${TARGET_NAME} ${ARGN})

	catapult_add_benchmark_dependencies(${TARGET_NAME})
	catapult_set_test_compiler_options()

	catapult_target(${TARGET_NAME})

	add_dependencies(tools ${TARGET_NAME})
	install(TARGETS ${TARGET_NAME})
endfunction()

add_subdirectory(crypto)

add_subdirectory(nodeps)
