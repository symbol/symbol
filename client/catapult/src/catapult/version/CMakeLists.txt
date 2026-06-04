add_definitions(-DCATAPULT_VERSION_DESCRIPTION="${CATAPULT_VERSION_DESCRIPTION}")

add_target(catapult.version LIBRARY
	SOURCES ./
)

if(NOT MSVC)
	add_subdirectory(nix)
endif()
