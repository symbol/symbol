add_target(catapult.local LIBRARY
	LINK_LIBS catapult.extensions
	SOURCES ./
)

add_subdirectory(broker)
add_subdirectory(importer)
add_subdirectory(recovery)
add_subdirectory(server)
