add_target(tests.catapult.crypto TEST
	WNO_DERIVED_LIB
	LINK_LIBS tests.catapult.test.crypto
	SOURCES ./
)
catapult_add_openssl_dependencies(tests.catapult.crypto)
