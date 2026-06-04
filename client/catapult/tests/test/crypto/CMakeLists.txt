add_target(tests.catapult.test.crypto LIBRARY
	LINK_LIBS
		tests.catapult.test.nodeps
		OpenSSL::SSL
	SOURCES ./
)
