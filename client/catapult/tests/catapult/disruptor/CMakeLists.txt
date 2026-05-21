add_target(tests.catapult.disruptor TEST
	LINK_LIBS tests.catapult.test.core
	SOURCES
		./
		./test
)
