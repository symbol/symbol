add_target(
	tests.catapult.int.mongo.plugins.restrictionmosaic TEST
	WNO_DERIVED_LIB
	LINK_LIBS
		tests.catapult.test.cache
		tests.catapult.test.mongo
		tests.catapult.test.mongo.plugins.restrictionmosaic
		catapult.mongo.plugins.restrictionmosaic
	LABELS Database
	SOURCES
		./storages
)

set(TARGET_NAME tests.catapult.int.mongo.plugins.restrictionmosaic)
catapult_add_mongo_dependencies(${TARGET_NAME})
