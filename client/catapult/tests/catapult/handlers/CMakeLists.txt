add_target(tests.catapult.handlers TEST
	LINK_LIBS
		tests.catapult.test.net
		catapult.tree
	SOURCES ./
)
