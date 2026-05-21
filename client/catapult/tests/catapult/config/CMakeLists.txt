add_target(tests.catapult.config TEST
	WNO_DERIVED_LIB
	LINK_LIBS tests.catapult.test.net
	SOURCES ./
)
