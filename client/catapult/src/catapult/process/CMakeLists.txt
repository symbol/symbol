add_target(catapult.process LIBRARY
	LINK_LIBS catapult.version
	SOURCES ./
)

add_subdirectory(broker)
add_subdirectory(importer)
add_subdirectory(recovery)
add_subdirectory(server)

install(TARGETS catapult.broker catapult.importer catapult.recovery catapult.server)
