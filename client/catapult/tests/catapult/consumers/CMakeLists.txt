add_target(tests.catapult.consumers TEST
	LINK_LIBS tests.catapult.test.cache
	SOURCES
		./
		./test
)
