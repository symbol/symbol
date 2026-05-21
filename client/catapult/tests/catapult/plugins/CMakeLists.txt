add_target(tests.catapult.plugins TEST
	WNO_DERIVED_LIB
	LINK_LIBS
		tests.catapult.test.core
		catapult.plugins.transfer
	SOURCES ./
)
