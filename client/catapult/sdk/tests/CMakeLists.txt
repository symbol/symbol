add_target(tests.catapult.sdk TEST
	LINK_LIBS tests.catapult.test.core
	SOURCES
		./builders
		./extensions
		./parsers
)
