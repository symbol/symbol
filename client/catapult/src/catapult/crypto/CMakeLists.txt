add_target(catapult.crypto LIBRARY
	LINK_LIBS
		catapult.utils
		external
		OpenSSL::SSL
	SOURCES ./
)
