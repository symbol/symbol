add_target(tests.catapult.crypto_voting TEST
	LINK_LIBS
		tests.catapult.test.core
		tests.catapult.test.crypto
	SOURCES
		./
		./test
)
