add_subdirectory(int)
add_subdirectory(test)

add_target(tests.catapult.mongo TEST
	WNO_DERIVED_LIB
	LINK_LIBS
		tests.catapult.test.mongo
		tests.catapult.test.cache
		catapult.mongo.plugins.transfer
	SOURCES
		./
		./mappers
)
catapult_add_mongo_dependencies(tests.catapult.mongo)
