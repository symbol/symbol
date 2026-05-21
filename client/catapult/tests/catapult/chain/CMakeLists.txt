add_target(tests.catapult.chain TEST
	LINK_LIBS tests.catapult.test.cache
	SOURCES
		./
		./test
)
