add_target(
	tests.catapult.int.mongo.plugins.namespace TEST
	WNO_DERIVED_LIB
	LINK_LIBS
		${MongoDB_IMPORTED_TARGETS}
		tests.catapult.test.mongo.plugins.namespace
		tests.catapult.test.plugins.namespace
		tests.catapult.test.cache
		tests.catapult.test.mongo
		catapult.mongo.plugins.namespace
	LABELS Database
	SOURCES
		./storages
)
