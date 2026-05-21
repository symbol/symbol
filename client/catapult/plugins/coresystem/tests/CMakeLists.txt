add_target(tests.catapult.plugins.coresystem TEST
	LINK_LIBS tests.catapult.test.cache
	SOURCES
		./
		./importance
		./model
		./observers
		./plugins
		./validators
)
