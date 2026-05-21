add_target(tests.catapult.plugins.signature TEST
	LINK_LIBS tests.catapult.test.cache
	SOURCES
		./
		./plugins
		./validators
)
