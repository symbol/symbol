add_target(tests.catapult.extensions TEST
	WNO_DERIVED_LIB
	LINK_LIBS
		tests.catapult.test.nemesis
		catapult.plugins.coresystem.deps
	SOURCES ./
)
catapult_add_openssl_dependencies(tests.catapult.extensions)
