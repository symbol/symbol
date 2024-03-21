cmake_minimum_required(VERSION 3.23)

if(MSVC)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj")
endif()

catapult_test_executable_target_no_lib(tests.catapult.deltaset nodeps test)
