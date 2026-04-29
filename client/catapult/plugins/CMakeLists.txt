include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/src)

set(PLUGIN_CATAPULT_LIBS
	${CORE_CATAPULT_LIBS}
	catapult.cache
	catapult.cache_core
	catapult.config
	catapult.observers
	catapult.plugins
	catapult.state
	catapult.validators)

subdirectories(ADD BASE . VERBOSE)
