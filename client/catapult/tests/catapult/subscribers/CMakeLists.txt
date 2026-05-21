add_target(tests.catapult.subscribers TEST
	LINK_LIBS tests.catapult.test.cache
	SOURCES
		./
		./test
)
