cmake_minimum_required(VERSION 3.23)

# setup benchmark
message("--- locating bench dependencies ---")
find_package(benchmark 1.8.3 EXACT REQUIRED)
message("bench     ver: ${benchmark_VERSION}")

# used to define a catapult bench executable
function(catapult_bench_executable_target TARGET_NAME)
	catapult_executable(${TARGET_NAME} ${ARGN})

	catapult_set_test_compiler_options()

	catapult_target(${TARGET_NAME})

	add_dependencies(tools ${TARGET_NAME})
	install(TARGETS ${TARGET_NAME})
endfunction()

add_subdirectory(crypto)

add_subdirectory(nodeps)
