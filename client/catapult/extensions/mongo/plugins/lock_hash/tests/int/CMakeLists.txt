add_target(
	tests.catapult.int.mongo.plugins.lockhash TEST
	WNO_DERIVED_LIB
	LINK_LIBS
		${MongoDB_IMPORTED_TARGETS}
		tests.catapult.test.mongo.plugins.lockhash
		tests.catapult.test.plugins.lockhash
		tests.catapult.test.cache
		tests.catapult.test.mongo
		catapult.mongo.plugins.lockhash
		catapult.plugins.lockhash.deps
	LABELS Database
	SOURCES
		./storages
)
