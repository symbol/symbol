add_target(bench.catapult.bench.nodeps LIBRARY
	LINK_LIBS
		benchmark::benchmark  # this also sets up benchmark include directories
		catapult.utils
		catapult.version
	SOURCES ./
)
