set(PLUGIN_BASE_NAME catapult.plugins.signature)

# create a deps lib
add_target(${PLUGIN_BASE_NAME}.deps LIBRARY
	LINK_LIBS ${PLUGIN_CATAPULT_LIBS}
	SOURCES ./ validators
)

# create a plugin dll
add_target(${PLUGIN_BASE_NAME} LIBRARY TYPE SHARED
	LINK_LIBS ${PLUGIN_BASE_NAME}.deps
	DEPENDENTS plugins
	SOURCES ./ plugins
)
