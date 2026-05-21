add_target(tests.catapult.utils TEST
	WNO_DERIVED_LIB
	LINK_LIBS tests.catapult.test.nodeps
	SOURCES
		./
		./test
)
