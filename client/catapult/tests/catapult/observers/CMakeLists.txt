add_target(tests.catapult.observers TEST
	LINK_LIBS tests.catapult.test.cache
	SOURCES
		./
		./test
)
