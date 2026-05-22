add_subdirectory(int)
add_subdirectory(test)

add_target(tests.catapult.mongo TEST
	WNO_DERIVED_LIB
	LINK_LIBS
		${MongoDB_IMPORTED_TARGETS}
		tests.catapult.test.mongo
		tests.catapult.test.cache
		catapult.mongo.plugins.transfer
	SOURCES
		./
		./mappers
)
