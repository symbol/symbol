add_target(tests.catapult.test.local LIBRARY
	LINK_LIBS
		tests.catapult.test.cache
		tests.catapult.test.net
		catapult.local.server
	SOURCES ./
)
