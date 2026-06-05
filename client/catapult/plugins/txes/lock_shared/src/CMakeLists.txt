glob_sources(_sources RECURSE ./)
add_target(catapult.plugins.lockshared LIBRARY
	TYPE INTERFACE
	SOURCES ${_sources}
)
