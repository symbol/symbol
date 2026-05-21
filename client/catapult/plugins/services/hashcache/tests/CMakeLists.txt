add_target(tests.catapult.plugins.hashcache TEST
	LINK_LIBS tests.catapult.test.cache
	SOURCES
		./
		./cache
		./handlers
		./observers
		./plugins
		./validators
		./test
)
