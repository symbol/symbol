set(TARGET_NAME catapult.server)

add_target(${TARGET_NAME} EXECUTABLE
	LINK_LIBS
		catapult.local.server
		catapult.process
	DEPENDENTS server
	SOURCES ./
)
