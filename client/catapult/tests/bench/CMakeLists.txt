# used to define a catapult bench executable
function(catapult_bench_executable_target TARGET_NAME)
	add_target(${TARGET_NAME} EXECUTABLE
		WITH_INSTALL
		LINK_LIBS build.tests
		DEPENDENTS tools
		SOURCES ./
	)
endfunction()

add_subdirectory(crypto)

add_subdirectory(nodeps)
