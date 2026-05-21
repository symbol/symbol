add_target(tests.catapult.local.server TEST
	WNO_DERIVED_LIB
	LINK_LIBS tests.catapult.test.nemesis
	SOURCES ./
)
