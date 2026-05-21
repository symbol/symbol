add_target(tests.catapult.thread TEST
	WNO_DERIVED_LIB
	LINK_LIBS tests.catapult.test.core
	SOURCES ./
)
