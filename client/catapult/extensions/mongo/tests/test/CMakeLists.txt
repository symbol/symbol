add_target(
	tests.catapult.test.mongo LIBRARY
	LINK_LIBS
		${MongoDB_IMPORTED_TARGETS}
		${GTest_IMPORTED_TARGETS}
		tests.catapult.test.core
	SOURCES
		./
		./mocks
)
