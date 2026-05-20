set(TARGET_NAME catapult.importer)

add_target(${TARGET_NAME} EXECUTABLE
	LINK_LIBS
		catapult.local.importer
		catapult.process
	DEPENDENTS server
	SOURCES ./
)
