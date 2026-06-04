add_target(tests.catapult.ionet TEST
	WNO_DERIVED_LIB
	LINK_LIBS
		tests.catapult.test.net
		OpenSSL::SSL
	SOURCES ./
)
