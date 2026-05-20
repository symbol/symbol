set(PLUGIN_CATAPULT_LIBS
	${CORE_CATAPULT_LIBS}
	catapult.cache
	catapult.cache_core
	catapult.config
	catapult.observers
	catapult.plugins
	catapult.state
	catapult.validators)

add_subdirectory(coresystem)
add_subdirectory(services)
add_subdirectory(txes)
