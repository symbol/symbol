add_target(tests.catapult.extensions TEST
	WNO_DERIVED_LIB
	LINK_LIBS
		tests.catapult.test.nemesis
		catapult.plugins.coresystem.deps
		OpenSSL::SSL
	SOURCES ./
)
