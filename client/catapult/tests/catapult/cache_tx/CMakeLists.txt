add_target(tests.catapult.cache_tx TEST
	LINK_LIBS tests.catapult.test.cache
	SOURCES
		./
		./test
)
