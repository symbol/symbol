add_target(
	tests.catapult.int.mongo.plugins.locksecret TEST
	NO_DERIVED_LIB
	LINK_LIBS
		${MongoDB_IMPORTED_TARGETS}
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
