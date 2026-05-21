add_target(tests.catapult.ionet TEST
	WNO_DERIVED_LIB
	LINK_LIBS tests.catapult.test.net
	SOURCES ./
)
catapult_add_openssl_dependencies(tests.catapult.ionet)
