add_target(
	tests.catapult.int.mongo.plugins.restrictionmosaic TEST
	WNO_DERIVED_LIB
	LINK_LIBS
		${MongoDB_IMPORTED_TARGETS}
		tests.catapult.test.cache
		tests.catapult.test.mongo
		tests.catapult.test.mongo.plugins.restrictionmosaic
		catapult.mongo.plugins.restrictionmosaic
	LABELS Database
	SOURCES
		./storages
)
