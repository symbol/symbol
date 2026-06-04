add_target(tests.catapult.crypto TEST
	WNO_DERIVED_LIB
	LINK_LIBS
		tests.catapult.test.crypto
		OpenSSL::SSL
	SOURCES ./
)
