add_target(
	tests.catapult.int.mongo.plugins.multisig TEST
	WNO_DERIVED_LIB
	LINK_LIBS
		${MongoDB_IMPORTED_TARGETS}
		tests.catapult.test.mongo.plugins.multisig
		tests.catapult.test.cache
		tests.catapult.test.mongo
		catapult.mongo.plugins.multisig
		catapult.plugins.multisig.deps
	LABELS Database
	SOURCES
		./storages
)
