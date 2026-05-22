add_target(tests.catapult.deltaset TEST
	WNO_DERIVED_LIB
	LINK_LIBS tests.catapult.test.nodeps
	SOURCES
		./
		./test
)
