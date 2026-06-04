set(TARGET_NAME catapult.tools.plugins)

add_target(${TARGET_NAME} LIBRARY
	LINK_LIBS catapult.plugins
	SOURCES ./
)

# tool has plugins dependency so it must be able to access src
