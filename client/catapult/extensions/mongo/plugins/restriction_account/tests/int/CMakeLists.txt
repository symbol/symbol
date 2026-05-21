add_target(
	tests.catapult.int.mongo.plugins.restrictionaccount TEST
	WNO_DERIVED_LIB
	LINK_LIBS
		tests.catapult.test.cache
		tests.catapult.test.mongo
		tests.catapult.test.mongo.plugins.restrictionaccount
		catapult.mongo.plugins.restrictionaccount
	LABELS Database
	SOURCES
		./storages
)

set(TARGET_NAME tests.catapult.int.mongo.plugins.restrictionaccount)
catapult_add_mongo_dependencies(${TARGET_NAME})
