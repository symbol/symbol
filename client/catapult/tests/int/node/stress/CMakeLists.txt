add_target(
	tests.catapult.int.node.stress TEST
	WNO_DERIVED_LIB
	LINK_LIBS
		tests.catapult.int.node.test
		tests.catapult.test.nemesis
	LABELS Integration Stress
	SOURCES 
		./ 
		./test
)
