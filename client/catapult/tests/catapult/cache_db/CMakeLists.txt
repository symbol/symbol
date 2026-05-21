add_target(tests.catapult.cache_db TEST
	LINK_LIBS
		tests.catapult.test.nodeps
		catapult.io
		${RocksDB_IMPORTED_TARGETS}
	LABELS Database_Rocks
	SOURCES
		./
		./test
)
