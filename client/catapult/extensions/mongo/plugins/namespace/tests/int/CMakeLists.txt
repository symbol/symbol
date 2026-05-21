add_target(
	tests.catapult.int.mongo.plugins.namespace TEST
	WNO_DERIVED_LIB
	LINK_LIBS
		tests.catapult.test.mongo.plugins.namespace
		tests.catapult.test.plugins.namespace
		tests.catapult.test.cache
		tests.catapult.test.mongo
		catapult.mongo.plugins.namespace
	LABELS Database
	SOURCES
		./storages
)

set(TARGET_NAME tests.catapult.int.mongo.plugins.namespace)
catapult_add_mongo_dependencies(${TARGET_NAME})
