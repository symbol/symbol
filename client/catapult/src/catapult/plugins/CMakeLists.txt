add_target(catapult.plugins LIBRARY
	LINK_LIBS
		catapult.validators
		${CMAKE_DL_LIBS}
	SOURCES ./
)
