add_target(
	tests.catapult.int.mongo.plugins.metadata TEST
	WNO_DERIVED_LIB
	LINK_LIBS
		tests.catapult.test.mongo.plugins.metadata
		tests.catapult.test.plugins.metadata
		tests.catapult.test.cache
		tests.catapult.test.mongo
		catapult.mongo.plugins.metadata
	LABELS Database
	SOURCES
		./storages
)

set(TARGET_NAME tests.catapult.int.mongo.plugins.metadata)
catapult_add_mongo_dependencies(${TARGET_NAME})
