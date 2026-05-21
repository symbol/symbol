add_target(tests.catapult.net TEST
	LINK_LIBS
		tests.catapult.test.net
		catapult.api
	SOURCES ./
)
