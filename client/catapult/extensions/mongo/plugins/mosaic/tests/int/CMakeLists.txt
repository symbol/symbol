add_target(
	tests.catapult.int.mongo.plugins.mosaic TEST
	WNO_DERIVED_LIB
	LINK_LIBS
		tests.catapult.test.mongo.plugins.mosaic
		tests.catapult.test.plugins.mosaic
		tests.catapult.test.cache
		tests.catapult.test.mongo
		catapult.mongo.plugins.mosaic
	LABELS Database
	SOURCES
		./storages
)

set(TARGET_NAME tests.catapult.int.mongo.plugins.mosaic)
catapult_add_mongo_dependencies(${TARGET_NAME})
