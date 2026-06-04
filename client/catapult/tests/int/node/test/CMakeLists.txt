add_target(tests.catapult.int.node.test LIBRARY
	LINK_LIBS
		catapult.plugins.coresystem
		tests.catapult.test.local
	SOURCES ./
)
