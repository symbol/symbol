add_target(tests.catapult.test.nemesis LIBRARY
	LINK_LIBS
		tests.catapult.test.local
		catapult.plugins.mosaic
		catapult.plugins.namespace
		catapult.plugins.transfer  # shared lib dependencies allow tests to create real transaction plugins
	SOURCES ./
)
