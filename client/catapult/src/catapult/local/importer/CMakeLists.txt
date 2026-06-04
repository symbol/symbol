add_target(catapult.local.importer LIBRARY
	LINK_LIBS
		catapult.local.recovery
		catapult.local.server
	SOURCES ./
)
