add_target(
	tests.catapult.int.mongo.plugins.locksecret TEST
	WNO_DERIVED_LIB
	LINK_LIBS
		tests.catapult.test.mongo.plugins.locksecret	
		tests.catapult.test.plugins.locksecret
		tests.catapult.test.cache
		tests.catapult.test.mongo
		catapult.mongo.plugins.locksecret
		catapult.plugins.locksecret.deps
	LABELS Database
	SOURCES
		./storages
)

set(TARGET_NAME tests.catapult.int.mongo.plugins.locksecret)
catapult_add_mongo_dependencies(${TARGET_NAME})
