add_target(
	tests.catapult.int.stress TEST
	WNO_DERIVED_LIB
	LINK_LIBS
		catapult.harvesting
		catapult.plugins.hashcache
		tests.catapult.test.nemesis
	LABELS Integration Stress
	SOURCES 
		./ 
		./test
)

# add dependency on hash cache plugin
