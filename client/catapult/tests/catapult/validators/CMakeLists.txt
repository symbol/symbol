add_target(tests.catapult.validators TEST
	LINK_LIBS tests.catapult.test.cache
	SOURCES
		./
		./test
)
