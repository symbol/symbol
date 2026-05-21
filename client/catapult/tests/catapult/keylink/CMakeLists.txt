add_target(tests.catapult.keylink TEST
	WNO_DERIVED_LIB
	LINK_LIBS
		tests.catapult.test.cache
		catapult.observers
		catapult.validators
	SOURCES ./
)
