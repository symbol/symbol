cmake_minimum_required(VERSION 3.23)

add_definitions(-DCATAPULT_VERSION_DESCRIPTION="${CATAPULT_VERSION_DESCRIPTION}")
catapult_library_target(catapult.version)

if(NOT MSVC)
	add_subdirectory(nix)
endif()
