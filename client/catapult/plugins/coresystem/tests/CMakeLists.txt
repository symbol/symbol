glob_sources(_sources RECURSE ./)
add_target(tests.${_plugin_full_name} TEST
	LINK_LIBS tests.catapult.test.cache
	SOURCES ${_sources}
)
