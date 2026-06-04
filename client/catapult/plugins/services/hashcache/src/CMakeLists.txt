set(PLUGIN_BASE_NAME catapult.plugins.hashcache)

# create a cache lib
add_target(${PLUGIN_BASE_NAME}.cache LIBRARY
	LINK_LIBS
		catapult.cache
		catapult.state
	SOURCES ./ cache
)

# create a deps lib
add_target(${PLUGIN_BASE_NAME}.deps LIBRARY
	LINK_LIBS
		${PLUGIN_CATAPULT_LIBS}
		${PLUGIN_BASE_NAME}.cache
	SOURCES ./ handlers observers validators
)

# create a plugin dll
add_target(${PLUGIN_BASE_NAME} LIBRARY TYPE SHARED
	LINK_LIBS ${PLUGIN_BASE_NAME}.deps
	DEPENDENTS plugins
	SOURCES ./ plugins
)
