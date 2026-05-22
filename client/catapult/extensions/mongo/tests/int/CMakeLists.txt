add_target(
	tests.catapult.int.mongo TEST
	WNO_DERIVED_LIB
	LINK_LIBS
		${MongoDB_IMPORTED_TARGETS}
		tests.catapult.test.cache
		tests.catapult.test.mongo
		catapult.mongo
	LABELS Database
	SOURCES
		./
		./storages
)
