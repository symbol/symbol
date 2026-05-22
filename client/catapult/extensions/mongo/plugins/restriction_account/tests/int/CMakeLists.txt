add_target(
	tests.catapult.int.mongo.plugins.restrictionaccount TEST
	WNO_DERIVED_LIB
	LINK_LIBS
		${MongoDB_IMPORTED_TARGETS}
		tests.catapult.test.cache
		tests.catapult.test.mongo
		tests.catapult.test.mongo.plugins.restrictionaccount
		catapult.mongo.plugins.restrictionaccount
	LABELS Database
	SOURCES
		./storages
)
