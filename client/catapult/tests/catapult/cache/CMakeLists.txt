add_target(tests.catapult.cache TEST
	WNO_DERIVED_LIB
	LINK_LIBS
		tests.catapult.test.cache
		${RocksDB_IMPORTED_TARGETS}
	SOURCES
		./
		./test
)
