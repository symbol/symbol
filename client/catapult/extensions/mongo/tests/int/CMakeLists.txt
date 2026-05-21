add_target(
	tests.catapult.int.mongo TEST
	WNO_DERIVED_LIB
	LINK_LIBS
		tests.catapult.test.cache
		tests.catapult.test.mongo
		catapult.mongo
	LABELS Database
	SOURCES
		./
		./storages
)

set(TARGET_NAME tests.catapult.int.mongo)
catapult_add_mongo_dependencies(${TARGET_NAME})
