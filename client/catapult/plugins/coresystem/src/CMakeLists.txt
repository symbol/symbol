# create a deps lib
add_target(${_plugin_full_name}.deps LIBRARY
	LINK_LIBS ${PLUGIN_CATAPULT_LIBS}
	SOURCES ./importance ./observers ./validators
)

# create a plugin dll
add_target(${_plugin_full_name}
	LIBRARY TYPE SHARED
	LINK_LIBS ${_plugin_full_name}.deps
	DEPENDENTS plugins
	SOURCES ./plugins
)
