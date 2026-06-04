add_subdirectory(plugins)

add_target(catapult.tools LIBRARY
	LINK_LIBS
		${CORE_CATAPULT_LIBS}
		catapult.api
		catapult.config
		catapult.net
		catapult.sdk
		catapult.version
	SOURCES ./
)
