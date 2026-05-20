set(TARGET_NAME catapult.broker)

add_target(${TARGET_NAME} EXECUTABLE
	LINK_LIBS
		catapult.local.broker
		catapult.process
	DEPENDENTS server
	SOURCES ./
)
