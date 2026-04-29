if(MSVC)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj")
endif()

catapult_test_executable_target(tests.catapult.deltaset nodeps test NOLIB)
