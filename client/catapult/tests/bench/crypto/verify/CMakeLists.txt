add_target(bench.catapult.crypto.verify EXECUTABLE
	WITH_INSTALL
	LINK_LIBS 
		build.tests
		catapult.crypto 
		bench.catapult.bench.nodeps
	DEPENDENTS tools
	SOURCES ./
)
