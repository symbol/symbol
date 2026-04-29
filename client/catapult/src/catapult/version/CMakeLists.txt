catapult_library_target(catapult.version)
target_compile_definitions(catapult.version PUBLIC DCATAPULT_VERSION_DESCRIPTION="${CATAPULT_VERSION_DESCRIPTION}")

if(NOT (MSVC OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")))
	subdirectories(ADD BASE . DIRS nix VERBOSE)
endif()
