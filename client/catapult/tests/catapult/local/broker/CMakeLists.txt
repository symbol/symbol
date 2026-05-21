add_target(tests.catapult.local.broker TEST
	LINK_LIBS
		tests.catapult.test.local
		catapult.plugins.signature
	SOURCES
		./
		./test
)
