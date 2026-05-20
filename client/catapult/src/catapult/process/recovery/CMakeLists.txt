set(TARGET_NAME catapult.recovery)

add_target(${TARGET_NAME} EXECUTABLE
	LINK_LIBS
		catapult.local.recovery
		catapult.process
	DEPENDENTS server
	SOURCES ./
)
