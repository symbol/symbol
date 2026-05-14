add_target(
	tests.catapult.test.core LIBRARY
	LINK_LIBS 
		tests.catapult.test.nodeps
		${CORE_CATAPULT_LIBS}
		catapult.sdk		
	SOURCES ./ ./mocks
)
